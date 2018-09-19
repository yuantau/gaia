#include "Parse.h"

using namespace Gaia;

Parse::Parse(void)
{
	if (!pLexer_) return;

}


Parse::~Parse(void)
{

}

void Parse::clear()
{
	symbolTable_.clear();
	functionTable_.clear();	
	stringTable_.clear();
	loopLocation_.clear();
}

/************************************************************************/
/* 设置错误消息                                                         */
/************************************************************************/
void Parse::setError(int code)
{
	error.line = pLexer_->getPrevToken()->line;
	error.index = pLexer_->getPrevToken()->startIndex;
	error.code = code;
	throw code;
}

void Parse::setLexer(Lexer *pLexer)
{
	pLexer_ = pLexer;
}

Stream * Parse::getStream()
{
	return iCode.getStream();
}


/************************************************************************/
/* 语法解析过程                                                         */
/************************************************************************/
void Parse::parse(Runtime *pRuntime)
{
	pRuntime_ = pRuntime;
	clear();
	preParse();
	pLexer_->reset();
	while(TRUE)
	{
		parseStatement();
	
		//检查是否结束
		Token *pToken = pLexer_->getNextToken();

		if(pToken->value ==  TOKEN_TYPE_END_STREAM)
			break;
		else 
			pLexer_->rewindToken();
	}
	ICodeNode *pCode = getFunction(0)->addInstr(INSTR_RET);
	pCode->addOp(GAIA_OP_INT)->intLiteral = 0;
	iCode.generateByteCode(&functionTable_, &symbolTable_, &stringTable_);
}

/************************************************************************/
/* 解析函数声明                                                         */
/************************************************************************/
void Parse::preParse()
{
	currScope = 0;

	//添加一个主函数，存放全局的指令,以0开头，保证不与脚本中的函数名字冲突
	int mainIndex = addFunction("0main");
	FunctionNode *pMain = getFunction(mainIndex);
	pMain->paramCount_ = 0;

	while (TRUE)
	{
		Token * pToken = pLexer_->getNextToken();
		if (pToken->value == TOKEN_TYPE_END_STREAM) break;

		if (pToken->value == TOKEN_TYPE_RSRVD_FUNCTION)
		{
			//检查函数名
			Token *pToken = pLexer_->checkToken(TOKEN_TYPE_IDENT);

			FunctionNode *pFunc;
			int fnIndex = addFunction(pToken->lexeme);
			if (fnIndex == -1)
			{
				//如果函数已存在
				pFunc = findFunction(pToken->lexeme);
				pFunc->clear();
			}
			else 
			{
				pFunc = getFunction(fnIndex);
			}

			//检查左括号
			pLexer_->checkToken(TOKEN_TYPE_DELIM_OPEN_PAREN);

			//如果接下来的不是右括号，说明有参数
			if (pLexer_->lookAhead() != ')')
			{
				int paramCount = 0;
				char paramList[32][MAX_IDENT_SIZE];

				while(TRUE)
				{
					//吃掉换行符
					while(pLexer_->lookAhead() == '\n')
						pLexer_->checkToken(TOKEN_TYPE_DELIM_NEWLINE);

					Token *pParamToken = pLexer_->checkToken(TOKEN_TYPE_IDENT);

					//保存当前参数名
					strcpy(paramList[paramCount], pParamToken->lexeme);
					paramCount++;

					//吃掉换行符
					while(pLexer_->lookAhead() == '\n')
						pLexer_->checkToken(TOKEN_TYPE_DELIM_NEWLINE);

					//如果遇到右括号，表示参数声明结束
					if (pLexer_->lookAhead() == ')')
						break;

					//检查逗号
					pLexer_->checkToken(TOKEN_TYPE_DELIM_COMMA);
				}

				//设置函数的参数个数
				pFunc->paramCount_ = paramCount;


				//把参数添加到符号表
				int stackIndex = 0;
				while(paramCount > 0)
				{
					paramCount--;
					SymbolNode *pSymbol = addSymbol(paramList[paramCount], pFunc->index_, SYMBOL_TYPE_PARAM);
					pSymbol->isParam = TRUE;
					pSymbol->stackIndex = stackIndex;
					stackIndex++;
				}
			}

			//检查右括号
			pLexer_->checkToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);

			//吃掉换行符
			while(pLexer_->lookAhead() == '\n')
				pLexer_->checkToken(TOKEN_TYPE_DELIM_NEWLINE);

			//检查左大括号
			pLexer_->checkToken(TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE);

			int braceCount = 1;
			while(TRUE)
			{
				pToken = pLexer_->getNextToken();
				if (pToken->value == TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE)
					braceCount++;
				else if(pToken->value == TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE)
				{
					braceCount--;
					if (braceCount == 0) break;
				}
			}
		}
	}
}

/************************************************************************/
/* 打印中间代码                                                         */
/************************************************************************/
void Parse::printICode(const char *filename)
{
	FILE *pFile = fopen(filename, "w");
	
	int k=0;
	while(k <symbolTable_.getLength())
	{
		SymbolNode *pSymbol = symbolTable_.getAt(k);
		if (pSymbol->scope == 0)
			fprintf(pFile, "var %s [stack:%d]\n", pSymbol->name, pSymbol->stackIndex);
		k++;
	}

	
	k = 0;
	char spaces[10] = {0,};
	int linec = 1;
	int dc = 0;
	while(k < functionTable_.getLength())
	{
		FunctionNode *pFunc = functionTable_.getAt(k);
		if (pFunc->index_ > 0)
		{
			fprintf(pFile, "function %s \n{\n", pFunc->name_);
			sprintf(spaces, "    ", linec);

			//打印参数
			int paramCount = 0;
			for (int j=0; j<symbolTable_.getLength(); j++)
			{
				if (symbolTable_.getAt(j)->scope == pFunc->index_)
				{

					paramCount++;
					fprintf(pFile, spaces);
					if (paramCount > pFunc->paramCount_)
						fprintf(pFile, "var %s\n", symbolTable_.getAt(j)->name);
					else 
						fprintf(pFile, "param %s\n", symbolTable_.getAt(j)->name);
				}
			}
		}
		else 
		{
			sprintf(spaces, "  ", linec);
		}

		for (int i=0; i<pFunc->icodeStream_.getLength(); i++)
		{
			ICodeNode *pCode = pFunc->icodeStream_.getAt(i);
			if (pCode->type == ICODE_SOURCE_LINE)
			{
				fprintf(pFile, ";%s", pCode->source);
				if (pCode->source[strlen(pCode->source)-1] != '\n')
					fprintf(pFile, "\n");
			}
			else if (pCode->type == ICODE_JUMP_TARGET)
			{
				fprintf(pFile, "L%d:\n", pCode->jumpTarget);
			}
			else 
			{
				linec++;
				fprintf(pFile, "%-02d ", dc++);
				static char temp[200];
				char *opstr = getMnemonics(pCode->opCode);
				
				fprintf(pFile, "%s ", opstr);
				ListNode<ICodeOp *> *pNode = pCode->opList.getHead();

				while(pNode)
				{
					switch (pNode->getData()->type_)
					{
					case GAIA_OP_INT:
						fprintf(pFile, "%d", pNode->getData()->intLiteral);
						break;
					case GAIA_OP_OBJECT:
						fprintf(pFile, "%s", "object");
						break;
					case GAIA_OP_EAX:
						fprintf(pFile, "eax");
						break;
					case GAIA_OP_EBX:
						fprintf(pFile, "ebx");
						break;
					case GAIA_OP_ECX:
						fprintf(pFile, "ecx");
						break;
					case GAIA_OP_VAR:
						{
							SymbolNode *pSymbol = getSymbol(pNode->getData()->symbolIndex);
							fprintf(pFile, "%s [stack:%d]", pSymbol->name, pSymbol->isParam ? -(pFunc->localDataSize_ + pSymbol->stackIndex + 2) : pSymbol->stackIndex);
						}
						
						break;
					case GAIA_OP_JUMP_TARGET_INDEX:
						fprintf(pFile, "%d", pNode->getData()->jumpTarget);
						break;
					case GAIA_OP_FUNC_INDEX:
						{
							int index = pNode->getData()->funcIndex;
							if (index < 0)
							{
								fprintf(pFile, "Local:%d", -(index+1));
							}
							else 
							{
								fprintf(pFile, "%s", getFunction(index)->name_);
							}
						}
						
						break;
					case GAIA_OP_STRING:
						fprintf(pFile, "\"%s\"", stringTable_.getAt(pNode->getData()->stringIndex));
						break;
					}
					if (pNode != pCode->opList.getTail())
						fprintf(pFile, ", ");
					pNode = pNode->getNext();
				}
				fprintf(pFile, "\n");
			}
		}
		if (pFunc->index_ > 0)
			fprintf(pFile, "}\n\n");
		k++;
	}
	fclose(pFile);
}

/************************************************************************/
/* 解析statement                                                        */
/************************************************************************/
void Parse::parseStatement()
{
	error.code = -1;
	
	if (pLexer_->lookAhead() == ';')
	{
		//读取end of statement
		pLexer_->checkToken(TOKEN_TYPE_DELIM_SEMICOLON);
		return;
	}
	if (pLexer_->lookAhead() == '\n')
	{
		//读取end of statement
		pLexer_->checkToken(TOKEN_TYPE_DELIM_NEWLINE);
		return;
	}

	//读取下一个Token
	Token *pToken = pLexer_->getNextToken();
	switch (pToken->value)
	{
	case TOKEN_TYPE_END_STREAM:
		//意外的文件结束
		setError(ERROR_UNEXPECTED_END_OF_FILE);
		break;
	case TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE:
		//如果是{
		parseBlock();
		break;
	case TOKEN_TYPE_RSRVD_FUNCTION:
		parseFunction();
		break;
	case TOKEN_TYPE_RSRVD_VAR:
		parseVar();
		break;
	case TOKEN_TYPE_IDENT:
		
		//如果是标识符,检查是不是函数
		if (pLexer_->lookAhead() == '(')
		{
			parseFunctionCall();
		}
		else if (pLexer_->lookAhead() == '.' || pLexer_->lookAhead() == '[')
		{
			parsePropAssign();
		}
		else 
		{
			//解析赋值语句
			parseAssign();
		}
		break;
	case TOKEN_TYPE_RSRVD_RETURN:
		parseReturn();
		break;
	case TOKEN_TYPE_RSRVD_WHILE:
		parseWhile();
		break;
	case TOKEN_TYPE_RSRVD_FOR:
		parseFor();
		break;
	case TOKEN_TYPE_RSRVD_BREAK:
	case TOKEN_TYPE_RSRVD_CONTINUE:
		parseControlCommands(pToken->value);
		break;
	case TOKEN_TYPE_RSRVD_IF:
		parseIf();
		break;
	case TOKEN_TYPE_OP:
		{
			//检查是不是前置自增自减运算符
			if (pToken->op == OP_TYPE_DEC || pToken->op == OP_TYPE_INC)
			{
				parseFrontInc_Dec();
				break;
			}
		}
	default:
		setError(ERROR_UNEXPECTED_INPUT);
		break;
	}
}

/************************************************************************/
/* 解析语句块                                                           */
/************************************************************************/
void Parse::parseBlock()
{
	
	while(pLexer_->lookAhead() != '}')
		parseStatement();

	pLexer_->checkToken(TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE);
}
	

/************************************************************************/
/* 解析函数                                                             */
/************************************************************************/
void Parse::parseFunction()
{
	//暂不支持嵌套函数，匿名函数
	if (currScope != 0)
		setError(ERROR_NESTED_FUNCTION_ILLEGAL);

	//检查函数名
	Token *pToken = pLexer_->checkToken(TOKEN_TYPE_IDENT);

	FunctionNode *pFunc;
	int fnIndex = addFunction(pToken->lexeme);
	if (fnIndex == -1)
	{
		//如果函数已存在
		pFunc = findFunction(pToken->lexeme);
		pFunc->clear();
	}
	else 
	{
		pFunc = getFunction(fnIndex);
	}

	//设置当前作用域
	currScope = pFunc->index_;

	//检查左括号
	pLexer_->checkToken(TOKEN_TYPE_DELIM_OPEN_PAREN);

	//如果接下来的不是右括号，说明有参数
	if (pLexer_->lookAhead() != ')')
	{


		while(TRUE)
		{
			//吃掉换行符
			while(pLexer_->lookAhead() == '\n')
				pLexer_->checkToken(TOKEN_TYPE_DELIM_NEWLINE);

			Token *pParamToken = pLexer_->checkToken(TOKEN_TYPE_IDENT);

			//吃掉换行符
			while(pLexer_->lookAhead() == '\n')
				pLexer_->checkToken(TOKEN_TYPE_DELIM_NEWLINE);

			//如果遇到右括号，表示参数声明结束
			if (pLexer_->lookAhead() == ')')
				break;

			//检查逗号
			pLexer_->checkToken(TOKEN_TYPE_DELIM_COMMA);
		}
		
		
	}

	//检查右括号
	pLexer_->checkToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);

	//吃掉换行符
	while(pLexer_->lookAhead() == '\n')
		pLexer_->checkToken(TOKEN_TYPE_DELIM_NEWLINE);

	//检查左大括号
	pLexer_->checkToken(TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE);

	//编译块
	parseBlock();

	//添加RET指令
	pFunc->addInstr(INSTR_RET)->addOp(GAIA_OP_INT)->intLiteral = pFunc->paramCount_;

	//恢复作用域
	currScope = 0;
}

/************************************************************************/
/* 解析变量声明                                                         */
/************************************************************************/
void Parse::parseVar()
{
	while(TRUE)
	{
		//检查接下来的单词是不是标识符
		Token *pToken = pLexer_->checkToken(TOKEN_TYPE_IDENT);

		//保存变量名
		static char ident[MAX_IDENT_SIZE];
		strcpy(ident, pToken->lexeme);
		SymbolNode *pSymbol = addSymbol(ident, currScope, SYMBOL_TYPE_VAR);
		FunctionNode *pFunc = getFunction(currScope);
		int stackIndex = currScope == 0 ? pFunc->localDataSize_ : -(pFunc->localDataSize_+2);
		pSymbol->stackIndex = stackIndex;
		pFunc->localDataSize_++;

		//如果后面跟的是=,那么是对变量赋值
		if (pLexer_->lookAhead() == '=')
			parseAssign();
		else 
			pLexer_->getNextToken();
		
		if (pLexer_->lookAhead() == ',')
		{
			//逗号表示本行后面还有声明语句
			pLexer_->checkToken(TOKEN_TYPE_DELIM_COMMA);
			continue;
		}
			

		if (pLexer_->getPrevToken()->value == TOKEN_TYPE_DELIM_SEMICOLON || 
				 pLexer_->getPrevToken()->value == TOKEN_TYPE_DELIM_NEWLINE)
			//如果是EOS或者换行符，表示语句结束
		{
			while(pLexer_->getNextToken()->value == TOKEN_TYPE_DELIM_NEWLINE);
			pLexer_->rewindToken();
			break;
		}
		else if (pLexer_->getPrevToken()->value == TOKEN_TYPE_END_STREAM)
		{
			//如果源码结束标志，回退一次
			pLexer_->rewindToken();
			break;
		}
		else
		{
			setError(ERROR_UNEXPECTED_END_OF_LINE);
		}
	}
	
}

/************************************************************************/
/* 解析while循环                                                        */
/************************************************************************/
void Parse::parseWhile()
{
	ICodeNode *pInstr;

	FunctionNode *pFunc = getFunction(currScope);
	pFunc->addSourceLine(pLexer_->getSourceLine(pLexer_->getPrevToken()->line));
	
	//循环顶部和底部的跳转目标
	int startTarget = getNextJumpTargetIndex();
	int endTargetIndex = getNextJumpTargetIndex();

	pFunc->addJumpTarget(startTarget);

	//吃掉换行符
	while(pLexer_->lookAhead() == '\n') pLexer_->getNextToken();

	//检查左括号
	pLexer_->checkToken(TOKEN_TYPE_DELIM_OPEN_PAREN);
	
	//inc dec 置0
	incIndex = 0;
	decIndex = 0;

	//分析表达式
	parseExpression();

	parseInc_Dec();

	//吃掉换行符
	while(pLexer_->lookAhead() == '\n') pLexer_->getNextToken();

	//检查右括号
	pLexer_->checkToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);

	//表达式结果弹出到eax
	pFunc->addInstr(INSTR_POP)->addOp(GAIA_OP_EAX);

	//吃掉换行符
	while(pLexer_->lookAhead() == '\n') pLexer_->getNextToken();

	//如果为false就跳转到结束
	pInstr = pFunc->addInstr(INSTR_JE);
	pInstr->addOp(GAIA_OP_EAX);
	pInstr->addOp(GAIA_OP_INT)->intLiteral = 0;
	pInstr->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = endTargetIndex;

	//循环位置进栈
	loopLocation_.push(new LoopLocation(startTarget, endTargetIndex));

	//分析循环体
	parseStatement();

	loopLocation_.pop();

	//跳转到循环开始
	pFunc->addInstr(INSTR_JMP)->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = startTarget;

	pFunc->addJumpTarget(endTargetIndex);
}

/************************************************************************/
/* 解析控制语句                                                         */
/* int ctrl TOKEN_TYPE_BREAK;TOKEN_TYPE_CONTINUE						*/
/************************************************************************/
void Parse::parseControlCommands(int ctrl)
{
	//检查是否在循环中
	if (loopLocation_.isEmpty())
		setError(GERROR_UNEXPECTED_BREAK);

	FunctionNode *pFunc = getFunction(currScope);
	pFunc->addSourceLine(pLexer_->getSourceLine(pLexer_->getPrevToken()->line));

	if (pLexer_->lookAhead() == '\n')
		pLexer_->checkToken(TOKEN_TYPE_DELIM_NEWLINE);
	else if (pLexer_->lookAhead() == ';')
	{
		pLexer_->checkToken(TOKEN_TYPE_DELIM_SEMICOLON);
	}
	else 
	{
		setError(ERROR_UNEXPECTED_END_OF_LINE);
	}

	//获取循环的结束位置
	int targetIndex = ctrl == TOKEN_TYPE_RSRVD_BREAK ? loopLocation_.peek()->endTarget_:
													   loopLocation_.peek()->startTarget_;

	//jmp end
	pFunc->addInstr(INSTR_JMP)->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = targetIndex;
}

/************************************************************************/
/* 解析if语句块                                                         */
/************************************************************************/
void Parse::parseIf()
{
	FunctionNode *pFunc = getFunction(currScope);
	pFunc->addSourceLine(pLexer_->getSourceLine(pLexer_->getPrevToken()->line));

	int falseJumpTarget = getNextJumpTargetIndex();

	//忽略换行
	while(pLexer_->lookAhead() == '\n') pLexer_->getNextToken();

	//检查左括号
	pLexer_->checkToken(TOKEN_TYPE_DELIM_OPEN_PAREN);

	//忽略换行
	while(pLexer_->lookAhead() == '\n') pLexer_->getNextToken();

	//inc dec 置0
	incIndex = 0;
	decIndex = 0;

	//分析表达式
	parseExpression();

	parseInc_Dec();


	//检查右括号
	pLexer_->checkToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);

	//pop eax, result
	pFunc->addInstr(INSTR_POP)->addOp(GAIA_OP_EAX);

	//je eax, 0, false
	ICodeNode *pInstr = pFunc->addInstr(INSTR_JE);
	pInstr->addOp(GAIA_OP_EAX);
	pInstr->addOp(GAIA_OP_INT)->intLiteral = 0;
	pInstr->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = falseJumpTarget;

	//忽略换行
	while(pLexer_->lookAhead() == '\n') pLexer_->getNextToken();

	//分析true块
	parseStatement();

	//忽略换行
	while(pLexer_->lookAhead() == '\n') pLexer_->getNextToken();

	//如果有else存在
	if (pLexer_->getNextToken()->value == TOKEN_TYPE_RSRVD_ELSE)
	{
		//忽略换行
		while(pLexer_->lookAhead() == '\n') pLexer_->getNextToken();

		//在true语句块后面添加无条件跳转
		int skipFalseTarget = getNextJumpTargetIndex();
		pFunc->addInstr(INSTR_JMP)->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = skipFalseTarget;

		//添加false跳转目标
		pFunc->addJumpTarget(falseJumpTarget);

		//分析语句块
		parseStatement();

		//添加skip跳转目标
		pFunc->addJumpTarget(skipFalseTarget);
	}
	else 
	{
		pLexer_->rewindToken();
		pFunc->addJumpTarget(falseJumpTarget);
	}
}

void Parse::parseFor()
{
	//忽略换行
	while(pLexer_->lookAhead() == '\n') pLexer_->getNextToken();

	//检查左括号
	pLexer_->checkToken(TOKEN_TYPE_DELIM_OPEN_PAREN);

	//解析第一个表达式,如果有var,调用parseVar,否则解析statement
	if (pLexer_->getNextToken()->value == TOKEN_TYPE_RSRVD_VAR)
		parseVar();	
	else
	{
		pLexer_->rewindToken();
		parseStatement();
	}
	
	if (!(pLexer_->getPrevToken()->value == TOKEN_TYPE_OP &&
		(pLexer_->getPrevToken()->op == OP_TYPE_INC || 
		 pLexer_->getPrevToken()->op == OP_TYPE_DEC)))
	pLexer_->rewindToken();

	//忽略换行
	while(pLexer_->lookAhead() == '\n') pLexer_->getNextToken();

	//检查分号
	pLexer_->checkToken(TOKEN_TYPE_DELIM_SEMICOLON);

	//忽略换行
	while(pLexer_->lookAhead() == '\n') pLexer_->getNextToken();
	
	int conditionTarget = getNextJumpTargetIndex();		//循环条件标号
	int loopTarget = getNextJumpTargetIndex();			//循环体标号
	int afterthoughtTarget = getNextJumpTargetIndex();	//迭代
	int endTarget = getNextJumpTargetIndex();

	FunctionNode *pFunc = getFunction(currScope);

	//添加循环条件跳转目标
	pFunc->addJumpTarget(conditionTarget);
	
	//inc dec 置0
	incIndex = 0;
	decIndex = 0;
	//解析循环条件表达式
	parseExpression();
	parseInc_Dec();
	//忽略换行
	while(pLexer_->lookAhead() == '\n') pLexer_->getNextToken();
	pLexer_->checkToken(TOKEN_TYPE_DELIM_SEMICOLON);
	//结果弹出到eax中
	pFunc->addInstr(INSTR_POP)->addOp(GAIA_OP_EAX);
	//如果false则跳转到结束
	ICodeNode *pInstr = pFunc->addInstr(INSTR_JE);
	pInstr->addOp(GAIA_OP_EAX);
	pInstr->addOp(GAIA_OP_INT)->intLiteral = 0;
	pInstr->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = endTarget;
	//否则跳转到loop执行
	pFunc->addInstr(INSTR_JMP)->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = loopTarget;


	//忽略换行
	while(pLexer_->lookAhead() == '\n') pLexer_->getNextToken();
	pFunc->addJumpTarget(afterthoughtTarget);
	//解析第三个表达式
	parseStatement();

	while(pLexer_->lookAhead() == ',')
	{
		pLexer_->getNextToken();
		parseStatement();
	}

	//忽略换行
	while(pLexer_->lookAhead() == '\n') pLexer_->getNextToken();
	//检查右括号
	
	pLexer_->checkToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);
	//忽略换行
	while(pLexer_->lookAhead() == '\n') pLexer_->getNextToken();

	//跳转到条件检查
	pFunc->addInstr(INSTR_JMP)->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = conditionTarget;
	

	pFunc->addJumpTarget(loopTarget);
	//分析for循环体
	parseStatement();
	pFunc->addInstr(INSTR_JMP)->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = afterthoughtTarget;

	pFunc->addJumpTarget(endTarget);
}

void Parse::parseObj()
{
	FunctionNode *pFunc = getFunction(currScope);
	pFunc->addInstr(INSTR_NEW_OBJ);
	if (pLexer_->lookAhead() != '}')
	{
		
		while(TRUE)
		{
			Token *pToken = pLexer_->getNextToken();
			if ((pToken->value != TOKEN_TYPE_IDENT && pToken->value != TOKEN_TYPE_STRING))
			{
				setError(ERROR_UNEXPECTED_INPUT);
			}

			//push key
			int stringIndex = addString(pToken->lexeme);
			pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_STRING)->stringIndex = stringIndex;

			pLexer_->checkToken(TOKEN_TYPE_DELIM_TYPE);

			//push value
			parseExpression();

			//保存key-value
			pFunc->addInstr(INSTR_SAVE_FIELD);


			if (pLexer_->lookAhead() == '}')
				break;
			pLexer_->checkToken(TOKEN_TYPE_DELIM_COMMA);
		}
	}
	
	pLexer_->checkToken(TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE);
}

void Parse::parseArray()
{
	FunctionNode *pFunc = getFunction(currScope);
	int count = 0;
	if (pLexer_->lookAhead() != ']')
	{

		while(TRUE)
		{
			//push value
			parseExpression();
			count++;
			if (pLexer_->lookAhead() == ']')
				break;
			pLexer_->checkToken(TOKEN_TYPE_DELIM_COMMA);
		}
	}
	pFunc->addInstr(INSTR_NEW_ARRAY)->addOp(GAIA_OP_INT)->intLiteral = count;
	pLexer_->checkToken(TOKEN_TYPE_DELIM_CLOSE_BRACE);
}

/************************************************************************/
/* 解析表达式                                                           */
/************************************************************************/
void Parse::parseExpression()
{
	//解析第一项
	parseRelationalExpression();
	
	FunctionNode *pFunc = getFunction(currScope);
	Token *pToken;

	while(TRUE)
	{
		pToken = pLexer_->getNextToken();
		if (pToken->value != TOKEN_TYPE_OP ||
			!pToken->isLogicalOp())
		{
			pLexer_->rewindToken();
			break;
		}

		//解析第二项
		parseRelationalExpression();

		//pop ebx
		pFunc->addInstr(INSTR_POP)->addOp(GAIA_OP_EBX);
		//pop eax
		pFunc->addInstr(INSTR_POP)->addOp(GAIA_OP_EAX);
		
		//是逻辑运算符
		switch (pToken->op)
		{
		case OP_TYPE_LOGICAL_AND:
			{
				int falseJumpTarget = getNextJumpTargetIndex();
				int exitJumpTarget = getNextJumpTargetIndex();

				//JE EAX, 0, True
				ICodeNode *pInstr = pFunc->addInstr(INSTR_JE);
				pInstr->addOp(GAIA_OP_EAX);
				pInstr->addOp(GAIA_OP_INT)->intLiteral = 0;
				pInstr->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = falseJumpTarget;

				//JE EBX, 0, True
				pInstr = pFunc->addInstr(INSTR_JE);
				pInstr->addOp(GAIA_OP_EBX);
				pInstr->addOp(GAIA_OP_INT)->intLiteral = 0;
				pInstr->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = falseJumpTarget;

				//push 1
				pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_INT)->intLiteral = 1;

				//JMP Exit
				pFunc->addInstr(INSTR_JMP)->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = exitJumpTarget;

				//L0 : False
				pFunc->addJumpTarget(falseJumpTarget);

				//push 0
				pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_INT)->intLiteral = 0;

				//L1 : Exit
				pFunc->addJumpTarget(exitJumpTarget);
				break;

			}
		case OP_TYPE_LOGICAL_OR:
			{
				int trueJumpTarget = getNextJumpTargetIndex();
				int exitJumpTarget = getNextJumpTargetIndex();

				//JNE EAX, 0, TRUE
				ICodeNode *pInstr = pFunc->addInstr(INSTR_JNE);
				pInstr->addOp(GAIA_OP_EAX);
				pInstr->addOp(GAIA_OP_INT)->intLiteral = 0;
				pInstr->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = trueJumpTarget;

				//JNE EBX, 0, TRUE
				pInstr = pFunc->addInstr(INSTR_JNE);
				pInstr->addOp(GAIA_OP_EBX);
				pInstr->addOp(GAIA_OP_INT)->intLiteral = 0;
				pInstr->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = trueJumpTarget;

				//push 0
				pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_INT)->intLiteral = 0;

				//jmp exit
				pFunc->addInstr(INSTR_JMP)->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = exitJumpTarget;

				//L0 : True
				pFunc->addJumpTarget(trueJumpTarget);
				//push 1
				pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_INT)->intLiteral = 1;

				//L0 : Exit
				pFunc->addJumpTarget(exitJumpTarget);
				break;
			}

		}
	}
}

void Parse::parseRelationalExpression()
{
	//解析子表达式
	parseSubExpression();

	
	FunctionNode *pFunc = getFunction(currScope);

	Token *pToken;
	while(TRUE)
	{
		pToken= pLexer_->getNextToken();
		if (pToken->value != TOKEN_TYPE_OP ||
			(!pToken->isRelationalOp()))
		{
			//如果不是操作符或者不是关系运算符和逻辑运算符
			//,回退token,并退出循环
			pLexer_->rewindToken();
			break;
		}

		//解析第二个子表达式
		parseSubExpression();

		//pop ebx
		pFunc->addInstr(INSTR_POP)->addOp(GAIA_OP_EBX);

		//pop eax
		pFunc->addInstr(INSTR_POP)->addOp(GAIA_OP_EAX);

		if (pToken->isRelationalOp())
		{
			int trueJumpTarget = getNextJumpTargetIndex();
			int exitJumpTarget = getNextJumpTargetIndex();

			ICodeNode *pInstr;

			
			//如果是关系运算符
			switch (pToken->op)
			{
			case OP_TYPE_EQUAL:
				pInstr = pFunc->addInstr(INSTR_JE);
				break;
			case OP_TYPE_NOT_EQUAL:
				pInstr = pFunc->addInstr(INSTR_JNE);
				break;
			case OP_TYPE_GREATER:
				pInstr = pFunc->addInstr(INSTR_JG);
				break;
			case OP_TYPE_GREATER_EQUAL:
				pInstr = pFunc->addInstr(INSTR_JGE);
				break;
			case OP_TYPE_LESS:
				pInstr = pFunc->addInstr(INSTR_JL);
				break;
			case OP_TYPE_LESS_EQUAL:
				pInstr = pFunc->addInstr(INSTR_JLE);
				break;
			}
			pInstr->addOp(GAIA_OP_EAX);
			pInstr->addOp(GAIA_OP_EBX);
			pInstr->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = trueJumpTarget;

			pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_INT)->intLiteral = 0;

			pFunc->addInstr(INSTR_JMP)->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = exitJumpTarget;

			pFunc->addJumpTarget(trueJumpTarget);

			pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_INT)->intLiteral = 1;

			pFunc->addJumpTarget(exitJumpTarget);

		}
	}
}

/************************************************************************/
/* 解析子表达式                                                         */
/************************************************************************/
void Parse::parseSubExpression()
{

	//解析第一项
	parseTerm();

	Token *pToken;
	FunctionNode *pFunc = getFunction(currScope);	//获取当前作用于

	while(TRUE)
	{
		pToken = pLexer_->getNextToken();
		if (pToken->value != TOKEN_TYPE_OP ||
			(pToken->op != OP_TYPE_ADD &&
			 pToken->op != OP_TYPE_SUB))
		{
			//如果不是操作符或者是+，-以外操作符,退出循环
			pLexer_->rewindToken();
			break;
		}

		//解析第二项
		parseTerm();

		//第2个操作数出栈到ebx
		pFunc->addInstr(INSTR_POP)->addOp(GAIA_OP_EBX);

		//第1个操作数出栈到eax
		pFunc->addInstr(INSTR_POP)->addOp(GAIA_OP_EAX);

		int instrCode = 0;
		switch (pToken->op)
		{
		case OP_TYPE_ADD:
			instrCode = INSTR_ADD;
			break;
		case OP_TYPE_SUB:
			instrCode = INSTR_SUB;
		}

		//eax = eax op ebx
		ICodeNode *pInstr = pFunc->addInstr(instrCode);
		pInstr->addOp(GAIA_OP_EAX);
		pInstr->addOp(GAIA_OP_EBX);

		//push eax
		pInstr = pFunc->addInstr(INSTR_PUSH);
		pInstr->addOp(GAIA_OP_EAX);

	}
}

/************************************************************************/
/* 解析项                                                               */
/************************************************************************/
void Parse::parseTerm()
{
	FunctionNode *pFunc = getFunction(currScope);	//获取当前作用域

	//解析第一个因式
	parseFactor();

	

	while (TRUE)
	{
		
		Token *pToken = pLexer_->getNextToken();
		
		//解析 * / % & ^ << >> 操作
		if (pToken->value != TOKEN_TYPE_OP ||
		   (pToken->op != OP_TYPE_MUL &&
			pToken->op != OP_TYPE_DIV &&
			pToken->op != OP_TYPE_MOD &&
			pToken->op != OP_TYPE_BITWISE_AND &&
			pToken->op != OP_TYPE_BITWISE_OR &&
			pToken->op != OP_TYPE_BITWISE_XOR &&
			pToken->op != OP_TYPE_BITWISE_SHIFT_LEFT &&
			pToken->op != OP_TYPE_BITWISE_SHIFT_RIGHT))
		{
			
			pLexer_->rewindToken();
			break;
		}

		

		//解析第二个因子
		parseFactor();

		//pop eax
		pFunc->addInstr(INSTR_POP)->addOp(GAIA_OP_EBX);
		pFunc->addInstr(INSTR_POP)->addOp(GAIA_OP_EAX);

		int instrCode;
		switch (pToken->op)
		{
		case OP_TYPE_MUL:
			instrCode = INSTR_MUL;
			break;
		case OP_TYPE_DIV:
			instrCode = INSTR_DIV;
			break;
		case OP_TYPE_MOD:
			instrCode = INSTR_MOD;
			break;
		case OP_TYPE_BITWISE_AND:
			instrCode = INSTR_AND;
			break;
		case OP_TYPE_BITWISE_OR:
			instrCode = INSTR_OR;
			break;
		case OP_TYPE_BITWISE_XOR:
			instrCode = INSTR_XOR;
			break;
		case OP_TYPE_BITWISE_SHIFT_LEFT:
			instrCode = INSTR_SHL;
			break;
		case OP_TYPE_BITWISE_SHIFT_RIGHT:
			instrCode = INSTR_SHR;
			break;
		}

		ICodeNode *pInstr = pFunc->addInstr(instrCode);
		pInstr->addOp(GAIA_OP_EAX);
		pInstr->addOp(GAIA_OP_EBX);

		//结果入栈
		pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_EAX);
		
	}
}

/************************************************************************/
/* 解析因式                                                             */
/************************************************************************/
void Parse::parseFactor()
{
	int opType;
	BOOL unaryOpPending = FALSE;
	Token *pToken = pLexer_->getNextToken();

	if (pToken->value == TOKEN_TYPE_OP &&
		(pToken->op == OP_TYPE_ADD ||
		pToken->op == OP_TYPE_SUB ||
		pToken->op == OP_TYPE_BITWISE_NOT ||
		pToken->op == OP_TYPE_LOGICAL_NOT ||
		pToken->op == OP_TYPE_INC ||
		pToken->op == OP_TYPE_DEC))
	{
		//检查是否一元运算符
		unaryOpPending = TRUE;
		opType = pToken->op;
	}
	else 
	{
		pLexer_->rewindToken();
	}


	FunctionNode *pFunc = getFunction(currScope);
	
	pToken = pLexer_->getNextToken();
	switch (pToken->value)
	{
	case TOKEN_TYPE_RSRVD_TRUE:
	case TOKEN_TYPE_RSRVD_FALSE:
		pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_INT)->intLiteral
			= pToken->value == TOKEN_TYPE_RSRVD_TRUE ? 1 : 0;
		break;
	case TOKEN_TYPE_INT:
		pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_INT)->intLiteral
			= atoi(pToken->lexeme);
		break;
	case TOKEN_TYPE_FLOAT:
		pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_NUMBER)->floatLiteral
			= (float)atof(pToken->lexeme);
		break;
	case TOKEN_TYPE_STRING:
		{
			int stringIndex = addString(pToken->lexeme);
			pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_STRING)->stringIndex = stringIndex;
		}
		break;
	case TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE:
		{
			parseObj();
			break;
		}
	case TOKEN_TYPE_DELIM_OPEN_BRACE:
		{

			parseArray();

			break;
		}
	case TOKEN_TYPE_IDENT:
		//如果是标识符
		{
			SymbolNode *pSymbol = findSymbol(pToken->lexeme, currScope);

			//寻找全局变量
			if (!pSymbol) pSymbol = findSymbol(pToken->lexeme, 0);
			
			if (pSymbol)
			{
				pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_VAR)->symbolIndex = pSymbol->index;
				Token *pNextToken = pLexer_->getNextToken();
				pLexer_->rewindToken();
				if (pNextToken->value == TOKEN_TYPE_OP)
				{
					pLexer_->getNextToken();
					if (pNextToken->op == OP_TYPE_INC)
					{
						incSymbols[incIndex++] = pSymbol;
					}
					else if (pNextToken->op == OP_TYPE_DEC)
						decSymbols[decIndex++] = pSymbol;
					else 
						pLexer_->rewindToken();
				}
				else if (pNextToken->value == TOKEN_TYPE_DELIM_DOT || pNextToken->value == TOKEN_TYPE_DELIM_OPEN_BRACE)
				{
					char c;
					while(TRUE)
					{

						c = pLexer_->lookAhead();
						if (c == '.' || c == '[')
						{
							if (c == '.')
								parseDotAccess(TRUE);
							else 
								parseArrayAccess(TRUE);

							if (pLexer_->lookAhead() != '(')
							{
								pFunc->addInstr(INSTR_GET_FIELD);
							}
							else 
							{
								parseObjMethodCall();
								c = pLexer_->lookAhead();
								pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_EAX);
							}
							continue;

						}
						break;
					}
				}
			}
			else
			{
				//如果是函数
				if (findFunction(pToken->lexeme) || pRuntime_->findLocalFunction(pToken->lexeme))
				{
					parseFunctionCall();
					pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_EAX);
				}
				else 
				{
					setError(GERROR_UNDEFINED_VAR);
				}
			}
		}
		break;
	case TOKEN_TYPE_DELIM_OPEN_PAREN:
		//如果是左括号，调用表达式解析
		parseExpression();
		pLexer_->checkToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);
		break;
	default:
		setError(GERROR_INVALID_INPUT);
	}


	
	//医院运算符操作
	if (unaryOpPending)
	{
		pFunc->addInstr(INSTR_POP)->addOp(GAIA_OP_EAX);

		if (opType == OP_TYPE_LOGICAL_NOT)
		{
			//如果是取反
			int trueJumpTargetINdex = getNextJumpTargetIndex();
			int exitJumpTargetIndex = getNextJumpTargetIndex();

			//je eax, 0, true
			ICodeNode *pInstr = pFunc->addInstr(INSTR_JE);
			pInstr->addOp(GAIA_OP_EAX);
			pInstr->addOp(GAIA_OP_INT)->intLiteral = 0;
			pInstr->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = exitJumpTargetIndex;

			//push 0
			pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_INT)->intLiteral = 0;

			//jmp true
			pFunc->addInstr(INSTR_JMP)->addOp(GAIA_OP_JUMP_TARGET_INDEX)->jumpTarget = exitJumpTargetIndex;

			//true
			pFunc->addJumpTarget(trueJumpTargetINdex);

			//push 1
			pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_INT)->intLiteral = 1;

			//false
			pFunc->addJumpTarget(exitJumpTargetIndex);
		}
		else if (opType == OP_TYPE_INC || opType == OP_TYPE_DEC)
		{
			SymbolNode *pSymbol = findSymbol(pLexer_->getPrevToken()->lexeme, currScope);
			if (!pSymbol) pSymbol = findSymbol(pLexer_->getPrevToken()->lexeme, 0);

			//add eax,
			ICodeNode *pInstr = pFunc->addInstr(INSTR_ADD);
			pInstr->addOp(GAIA_OP_VAR);
			pInstr->addOp(GAIA_OP_INT)->intLiteral = opType == OP_TYPE_INC ? 1: -1;
			
			//mov eax, var
			pInstr = pFunc->addInstr(INSTR_MOV);
			pInstr->addOp(GAIA_OP_VAR)->symbolIndex = pSymbol->index;
			pInstr->addOp(GAIA_OP_EAX);
			
			//push var
			pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_VAR)->symbolIndex = pSymbol->index;
		}
		else
		{
			int opCode;
			if (opType == OP_TYPE_SUB)
				opCode = INSTR_NEG;
			else if (opType == OP_TYPE_BITWISE_NOT)
				opCode = INSTR_NOT;
			pFunc->addInstr(opCode)->addOp(GAIA_OP_EAX);

			pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_EAX);
		}
		
	}
	
}

void Parse::parseDotAccess(BOOL saveKey)
{
	FunctionNode *pFunc = getFunction(currScope);
	pLexer_->checkToken(TOKEN_TYPE_DELIM_DOT);
	Token *pToken = pLexer_->checkToken(TOKEN_TYPE_IDENT);
		
	int strIndex = addString(pToken->lexeme);
	pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_STRING)->stringIndex = strIndex;

	while(pLexer_->lookAhead() == '\n')
	{
		pLexer_->checkToken(TOKEN_TYPE_DELIM_NEWLINE);
	}

		/*
		else if (!saveKey)
			pFunc->addInstr(INSTR_GET_FIELD);
		
		if (pLexer_->lookAhead() == '.')
		{
			if (saveKey) pFunc->addInstr(INSTR_GET_FIELD);
			continue;
		}
		else if (pLexer_->lookAhead() == '[')
		{
			if (saveKey)
				pFunc->addInstr(INSTR_GET_FIELD);
			parseArrayAccess(saveKey);
		}
		break;
		*/

}

void Parse::parseArrayAccess(BOOL saveKey)
{
	FunctionNode *pFunc = getFunction(currScope);
	pLexer_->checkToken(TOKEN_TYPE_DELIM_OPEN_BRACE);
	parseExpression();
	pLexer_->checkToken(TOKEN_TYPE_DELIM_CLOSE_BRACE);
	while(pLexer_->lookAhead() == '\n')
	{
		pLexer_->checkToken(TOKEN_TYPE_DELIM_NEWLINE);
	}
}

void Parse::parseObjMethodCall()
{
	pLexer_->checkToken(TOKEN_TYPE_DELIM_OPEN_PAREN);

	int paramCount = 0;

	while(TRUE)
	{
		if (pLexer_->lookAhead() != ')')
		{
			while(pLexer_->lookAhead() == '\n')
				pLexer_->getNextToken();

			parseExpression();

			paramCount++;

			while(pLexer_->lookAhead() == '\n')
				pLexer_->getNextToken();

			if (pLexer_->lookAhead() != ')')
				pLexer_->checkToken(TOKEN_TYPE_DELIM_COMMA);
		}
		else 
			break;
	}

	pLexer_->checkToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);
	FunctionNode *pFunc = getFunction(currScope);
	pFunc->addInstr(INSTR_PUSH)->addOp(GAIA_OP_INT)->intLiteral = paramCount;
	pFunc->addInstr(INSTR_OCALL);
}

/************************************************************************/
/* 解析函数调用                                                         */
/************************************************************************/
void Parse::parseFunctionCall()
{
	
	Token *pToken = pLexer_->getPrevToken();
	FunctionNode *pFunc = getFunction(currScope);
	pFunc->addSourceLine(pLexer_->getSourceLine(pToken->line));

	FunctionNode *pCall = findFunction(pToken->lexeme);
	Function *pCallLocal = NULL;
	if (!pCall)
		pCallLocal = pRuntime_->findLocalFunction(pToken->lexeme);

	if (pCall == NULL && pCallLocal == NULL)
		setError(GERROR_UNDEFINED_FUNCTION_CALL);

	pLexer_->checkToken(TOKEN_TYPE_DELIM_OPEN_PAREN);

	int paramCount = 0;

	while(TRUE)
	{
		if (pLexer_->lookAhead() != ')')
		{
			//吃掉\n
			while(pLexer_->lookAhead() == '\n')
				pLexer_->getNextToken();

			parseExpression();

			paramCount++;

			if (pCall && (paramCount > pCall->paramCount_))
				setError(GERROR_MORE_PARAM);

			//吃掉\n
			while(pLexer_->lookAhead() == '\n')
				pLexer_->getNextToken();

			if (pLexer_->lookAhead() != ')')
				pLexer_->checkToken(TOKEN_TYPE_DELIM_COMMA);
		}
		else 
			break;
	}

	pLexer_->checkToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);

	if (paramCount < pFunc->paramCount_)
		setError(GERROR_FEW_PARAM);

	ICodeOp *op = pFunc->addInstr(INSTR_CALL)->addOp(GAIA_OP_FUNC_INDEX);
	if (pCallLocal)
	{
		op->funcIndex = -(1 + pCallLocal->index);
	}
	else 
	{
		op->funcIndex = pCall->index_;
	}
	


}


void Parse::parsePropAssign()
{
	BOOL isOCall = FALSE;
	Token *pToken = pLexer_->getPrevToken();

	FunctionNode *pFunction = getFunction(currScope);

	pFunction->addSourceLine(pLexer_->getSourceLine(pToken->line));

	SymbolNode *pSymbol = findSymbol(pToken->lexeme, currScope);

	//如果局部变量不存在，寻找全局变量
	if (!pSymbol)
		pSymbol = findSymbol(pToken->lexeme, 0);

	//如果不存在,添加一个全局变量
	if (!pSymbol)
	{
		pSymbol = addSymbol(pToken->lexeme, 0, SYMBOL_TYPE_VAR);
		pSymbol->stackIndex = getFunction(0)->localDataSize_ ;
		getFunction(0)->localDataSize_ ++;
	}

	//检查属性访问运算符
	char c = pLexer_->lookAhead();
	if (c == '.' || c == '[')
	{
		pFunction->addInstr(INSTR_PUSH)->addOp(GAIA_OP_VAR)->symbolIndex = pSymbol->index;
	}
		
	while(TRUE)
	{
		c = pLexer_->lookAhead();
		if (c == '.' || c == '[')
		{
			if (c == '.')
				parseDotAccess(TRUE);
			else 
				parseArrayAccess(TRUE);

			c = pLexer_->lookAhead();
			
			if (c == '(')
			{
				isOCall = TRUE;
				parseObjMethodCall();
				c = pLexer_->lookAhead();
				if (c == '.' || c == '[')
					pFunction->addInstr(INSTR_PUSH)->addOp(GAIA_OP_EAX);
			}
			else 
			{
				isOCall = FALSE;
				if (c == '.' || c == '[')
					pFunction->addInstr(INSTR_GET_FIELD);
			}
			continue;
		}
		break;
	}

	if (pLexer_->lookAhead() == ';' || pLexer_->lookAhead() == '\n')
	{
		if (!isOCall) setError(ERROR_UNEXPECTED_END_OF_LINE);
		return;
	}
	
	pToken = pLexer_->getNextToken();
	if (pToken->value != TOKEN_TYPE_OP && 
		pToken->op != OP_TYPE_ASSIGN &&			// =
		pToken->op != OP_TYPE_ASSIGN_ADD &&		// +=
		pToken->op != OP_TYPE_ASSIGN_SUB &&     // -=
		pToken->op != OP_TYPE_ASSIGN_MUL &&     // *=
		pToken->op != OP_TYPE_ASSIGN_DIV &&
		pToken->op != OP_TYPE_ASSIGN_MOD &&
		pToken->op != OP_TYPE_ASSIGN_AND &&
		pToken->op != OP_TYPE_ASSIGN_OR &&
		pToken->op != OP_TYPE_ASSIGN_XOR &&
		pToken->op != OP_TYPE_ASSIGN_SHIFT_LEFT &&
		pToken->op != OP_TYPE_ASSIGN_SHIFT_RIGHT)
	{
		setError(ERROR_ILLEGAL_ASSIGN_OP);
	}
	if (isOCall)
		setError(GERROR_INVALID_LEFT_HAND_SIDE_ASSGIN);

	int assignToken = pToken->op;

	

	if (assignToken == OP_TYPE_INC || assignToken == OP_TYPE_DEC)
	{
		ICodeNode *pInstr = pFunction->addInstr(assignToken == OP_TYPE_INC ? INSTR_INC : INSTR_DEC);
	}
	else 
	{
		
		incIndex = decIndex = 0;

		//调用表达式解析
		parseExpression();

	
		if(pLexer_->lookAhead() == ')' || pLexer_->lookAhead() == ',')
			pLexer_->rewindToken();
		pLexer_->getNextToken();

		ICodeNode *pCurrInstr = NULL;
		switch(assignToken)
		{
		case OP_TYPE_ASSIGN:			//=
			pCurrInstr = pFunction->addInstr(INSTR_SET_FIELD);
			break;
		case OP_TYPE_ASSIGN_ADD:		//+=
			pCurrInstr = pFunction->addInstr(INSTR_ADD_FIELD);
			break;
		case OP_TYPE_ASSIGN_SUB:		//-=
			pCurrInstr = pFunction->addInstr(INSTR_SUB_FIELD);
			break;	
		case OP_TYPE_ASSIGN_MUL:		//*=
			pCurrInstr = pFunction->addInstr(INSTR_MUL_FIELD);
			break;
		case OP_TYPE_ASSIGN_DIV:		///=
			pCurrInstr = pFunction->addInstr(INSTR_DIV_FIELD);
			break;
		case OP_TYPE_ASSIGN_MOD:		//%=
			pCurrInstr = pFunction->addInstr(INSTR_MOD_FIELD);
			break;
		}

		parseInc_Dec();
	}
}

/************************************************************************/
/* 解析赋值语句                                                         */
/************************************************************************/
void Parse::parseAssign()
{
	BOOL isOCall = FALSE;
	BOOL isPropAccess = FALSE;
	int paramStrIndex = -1;
	int assignToken = 0;

	Token *pToken = pLexer_->getPrevToken();

	FunctionNode *pFunction = getFunction(currScope);

	pFunction->addSourceLine(pLexer_->getSourceLine(pToken->line));

	SymbolNode *pSymbol = findSymbol(pToken->lexeme, currScope);

	//如果局部变量不存在，寻找全局变量
	if (!pSymbol)
		pSymbol = findSymbol(pToken->lexeme, 0);

	//如果不存在,添加一个全局变量
	if (!pSymbol)
	{
		pSymbol = addSymbol(pToken->lexeme, 0, SYMBOL_TYPE_VAR);
		pSymbol->stackIndex = getFunction(0)->localDataSize_ ;
		getFunction(0)->localDataSize_ ++;
	}

	//检查属性访问运算符
	if (pLexer_->lookAhead() == '.')
	{
		isPropAccess = TRUE;
		pFunction->addInstr(INSTR_PUSH)->addOp(GAIA_OP_VAR)->symbolIndex = pSymbol->index;
		parseDotAccess(TRUE);
		
		if (pLexer_->getPrevToken()->value == TOKEN_TYPE_DELIM_CLOSE_PAREN)
			isOCall = TRUE;
		if (pLexer_->lookAhead() == ';' || pLexer_->lookAhead() == '\n')
			return;
	}
	else if (pLexer_->lookAhead() == '[')
	{
		pLexer_->checkToken(TOKEN_TYPE_DELIM_OPEN_BRACE);
		isPropAccess = TRUE;
		pFunction->addInstr(INSTR_PUSH)->addOp(GAIA_OP_VAR)->symbolIndex = pSymbol->index;
		parseExpression();
		pFunction->addInstr(INSTR_POP)->addOp(GAIA_OP_ECX);
		pFunction->addInstr(INSTR_PUSH)->addOp(GAIA_OP_ECX);
		pLexer_->checkToken(TOKEN_TYPE_DELIM_CLOSE_BRACE);
		if (pLexer_->lookAhead() == '(')
		{
			parseObjMethodCall();
			return;
		}
		else 
			pFunction->addInstr(INSTR_GET_FIELD);

	}
	
	pToken = pLexer_->getNextToken();
	if (pToken->value != TOKEN_TYPE_OP && 
		pToken->op != OP_TYPE_ASSIGN &&			// =
		pToken->op != OP_TYPE_ASSIGN_ADD &&		// +=
		pToken->op != OP_TYPE_ASSIGN_SUB &&     // -=
		pToken->op != OP_TYPE_ASSIGN_MUL &&     // *=
		pToken->op != OP_TYPE_ASSIGN_DIV &&
		pToken->op != OP_TYPE_ASSIGN_MOD &&
		pToken->op != OP_TYPE_ASSIGN_AND &&
		pToken->op != OP_TYPE_ASSIGN_OR &&
		pToken->op != OP_TYPE_ASSIGN_XOR &&
		pToken->op != OP_TYPE_ASSIGN_SHIFT_LEFT &&
		pToken->op != OP_TYPE_ASSIGN_SHIFT_RIGHT)
	{
		setError(ERROR_ILLEGAL_ASSIGN_OP);
	}
	if (isOCall)
	{
		setError(GERROR_INVALID_LEFT_HAND_SIDE_ASSGIN);
	}

	assignToken = pToken->op;

	

	if (assignToken == OP_TYPE_INC || assignToken == OP_TYPE_DEC)
	{
		if (isPropAccess)
		{
			pFunction->addInstr(INSTR_POP)->addOp(GAIA_OP_EAX);
			ICodeNode *pInstr = pFunction->addInstr(assignToken == OP_TYPE_INC ? INSTR_INC : INSTR_DEC);
			pInstr->addOp(GAIA_OP_EAX);
			pFunction->addInstr(INSTR_PUSH)->addOp(GAIA_OP_EAX);
			pFunction->addInstr(INSTR_PUSH)->addOp(GAIA_OP_VAR)->symbolIndex = pSymbol->index;
			pFunction->addInstr(INSTR_PUSH)->addOp(GAIA_OP_STRING)->stringIndex = paramStrIndex;
			pFunction->addInstr(INSTR_SET_FIELD);
		}
		else 
		{
			ICodeNode *pInstr = pFunction->addInstr(assignToken == OP_TYPE_INC ? INSTR_INC : INSTR_DEC);
			pInstr->addOp(GAIA_OP_VAR)->symbolIndex = pSymbol->index;
		}
	}
	else 
	{
		
		incIndex = decIndex = 0;

		//调用表达式解析
		parseExpression();

		if (pLexer_->lookAhead() != '\n' && pLexer_->lookAhead() != ';' && pLexer_->lookAhead() != ',' && pLexer_->lookAhead() != ')')
		{
			if (pLexer_->lookAhead() != '\0')
			{
				setError(ERROR_UNEXPECTED_END_OF_LINE);
			}
		}
		if(pLexer_->lookAhead() == ')' || pLexer_->lookAhead() == ',')
			pLexer_->rewindToken();
		pLexer_->getNextToken();

		//栈顶元素pop到eax
		pFunction->addInstr(INSTR_POP)->addOp(GAIA_OP_EAX);


		if (isPropAccess)
		{
			//field值pop到ebx
			pFunction->addInstr(INSTR_POP)->addOp(GAIA_OP_EBX);
		}

		ICodeNode *pCurrInstr = NULL;
		switch(assignToken)
		{
		case OP_TYPE_ASSIGN:			//=
			pCurrInstr = pFunction->addInstr(INSTR_MOV);
			break;
		case OP_TYPE_ASSIGN_ADD:		//+=
			pCurrInstr = pFunction->addInstr(INSTR_ADD);
			break;
		case OP_TYPE_ASSIGN_SUB:		//-=
			pCurrInstr = pFunction->addInstr(INSTR_SUB);
			break;	
		case OP_TYPE_ASSIGN_MUL:		//*=
			pCurrInstr = pFunction->addInstr(INSTR_MUL);
			break;
		case OP_TYPE_ASSIGN_DIV:		///=
			pCurrInstr = pFunction->addInstr(INSTR_DIV);
			break;
		case OP_TYPE_ASSIGN_MOD:		//%=
			pCurrInstr = pFunction->addInstr(INSTR_MOD);
			break;
		}

		
		//把变量添加到第一个操作数
		if (isPropAccess)
			pCurrInstr->addOp(GAIA_OP_EBX);
		else 
			pCurrInstr->addOp(GAIA_OP_VAR)->symbolIndex = pSymbol->index;

		//eax添加到第2个操作数
		pCurrInstr->addOp(GAIA_OP_EAX);
	
		if (isPropAccess)
		{
			//setfield
			pFunction->addInstr(INSTR_PUSH)->addOp(GAIA_OP_EBX);
			pFunction->addInstr(INSTR_PUSH)->addOp(GAIA_OP_VAR)->symbolIndex = pSymbol->index;
			if (paramStrIndex == -1)
				pFunction->addInstr(INSTR_PUSH)->addOp(GAIA_OP_ECX);
			else 
				pFunction->addInstr(INSTR_PUSH)->addOp(GAIA_OP_STRING)->stringIndex = paramStrIndex;
			pFunction->addInstr(INSTR_SET_FIELD);
		}

		

		parseInc_Dec();
	}

	
}


/************************************************************************/
/* 解析函数返回值                                                       */
/************************************************************************/
void Parse::parseReturn()
{
	if (currScope == 0)
		setError(GERROR_UNEXPECTED_RETURN);

	Token *pToken = pLexer_->getPrevToken();
	FunctionNode *pFunc = getFunction(currScope);
	pFunc->addSourceLine(pLexer_->getSourceLine(pToken->line));

	if (pLexer_->lookAhead() != ';' && pLexer_->lookAhead() != '\n')
	{
		//如果不是EOS或者\n 解析返回值
		pFunc->setReturnSize(1);
		parseExpression();
	}
	ICodeNode *pInstr = pFunc->addInstr(INSTR_POP);
	pInstr->addOp(GAIA_OP_EAX);

	pFunc->addInstr(INSTR_RET)->addOp(GAIA_OP_INT)->intLiteral = pFunc->paramCount_;

	

}

/************************************************************************/
/* 解析后置自增 自减运算符                                                */
/************************************************************************/
void Parse::parseInc_Dec()
{
	FunctionNode *pFunction = getFunction(currScope);
	for (int i=0; i<incIndex; i++)
	{
		ICodeNode *pInstr = pFunction->addInstr(INSTR_ADD);
		pInstr->addOp(GAIA_OP_VAR)->symbolIndex = incSymbols[i]->index;
		pInstr->addOp(GAIA_OP_INT)->intLiteral = 1;
	}

	for (int i=0; i<decIndex; i++)
	{
		ICodeNode *pInstr = pFunction->addInstr(INSTR_ADD);
		pInstr->addOp(GAIA_OP_VAR)->symbolIndex = decSymbols[i]->index;
		pInstr->addOp(GAIA_OP_INT)->intLiteral = -1;
	}
}

/************************************************************************/
/* 解析前置自增自减运算符                                               */
/************************************************************************/
void Parse::parseFrontInc_Dec()
{
	pLexer_->rewindToken();
	int op = pLexer_->getNextToken()->op;
	
	
	Token *pToken = pLexer_->getNextToken();
	SymbolNode *pSymbol = findSymbol(pToken->lexeme, currScope);
	if (!pSymbol) pSymbol = findSymbol(pToken->lexeme, 0);
	if (!pSymbol)
	{
		setError(GERROR_UNDEFINED_VAR);
	}
	FunctionNode *pFunc = getFunction(currScope);
	ICodeNode *pInstr = pFunc->addInstr(op == OP_TYPE_INC ? INSTR_INC : INSTR_DEC);
	pInstr->addOp(GAIA_OP_VAR)->symbolIndex = pSymbol->index;
}

/************************************************************************/
/* 添加字符串到字符串表                                                 */
/************************************************************************/
int Parse::addString(char *str)
{
	for (int i=0; i<stringTable_.getLength(); i++)
	{
		if (strcmp(stringTable_.getAt(i), str) == 0)
			return i;
	}

	return stringTable_.push(str, strlen(str)+1);
}

/************************************************************************/
/* 添加变量到符号表                                                     */
/************************************************************************/
SymbolNode * Parse::addSymbol(char *name, int scope, int type)
{
	//如果符号已存在
	SymbolNode *pSymbol;
	if (pSymbol = findSymbol(name, scope))
		return pSymbol;

	pSymbol = new SymbolNode;
	strcpy(pSymbol->name, name);
	pSymbol->scope = scope;
	pSymbol->type = type;

	int index = symbolTable_.push(pSymbol);
	pSymbol->index = index;
	return pSymbol;
}

/************************************************************************/
/* 根据名称以及作用域搜索变量                                           */
/************************************************************************/
SymbolNode * Parse::findSymbol(char *name, int scope)
{
	for (int i=0, len=symbolTable_.getLength(); i<len; i++)
	{
		SymbolNode *pSymbol = symbolTable_.getAt(i);
		if (pSymbol->scope == scope && (strcmp(name, pSymbol->name) == 0))
			return pSymbol;
	}
	return NULL;
}

/************************************************************************/
/* 根据索引获取变量                                                     */
/************************************************************************/
SymbolNode * Parse::getSymbol(int index)
{
	if (index >-1 && index <symbolTable_.getLength())
		return symbolTable_.getAt(index);
	return NULL;
}

/************************************************************************/
/* 添加函数到函数表                                                     */
/************************************************************************/
int Parse::addFunction(char *name)
{
	//如果函数已存在
	if (findFunction(name))
		return -1;

	FunctionNode *pFunc = new FunctionNode;
	strcpy(pFunc->name_, name);

	int index = functionTable_.push(pFunc);
	pFunc->index_ = index;
	pFunc->paramCount_ = 0;
	return index;
}

/************************************************************************/
/* 根据函数名字搜索函数                                                 */
/************************************************************************/
FunctionNode * Parse::findFunction(char *name)
{
	for (int i=0, len=functionTable_.getLength(); i<len; i++)
	{
		FunctionNode *pFunc = functionTable_.getAt(i);
		if (strcmp(pFunc->name_, name) == 0)
			return pFunc;
	}
	return NULL;
}

/************************************************************************/
/* 根据索引获取函数                                                     */
/************************************************************************/
FunctionNode * Parse::getFunction(int index)
{
	if (index >= 0 && index <= functionTable_.getLength()-1)
		return functionTable_.getAt(index);
	return NULL;
}

/************************************************************************/
/* 获取跳转目标                                                         */
/************************************************************************/
int Parse::getNextJumpTargetIndex()
{
	static int jumpTargetIndex = 0;
	return jumpTargetIndex++;
}

int Parse::getGlobalVarSize()
{
	int result = 0;
	for (int i=0; i<symbolTable_.getLength(); i++)
	{
		if (symbolTable_.getAt(i)->scope == 0)
			result++;
	}
	return result;
}

/************************************************************************/
/* 获取总的指令条数                                                     */
/************************************************************************/
int Parse::getInstrSize()
{
	int result = 0;
	for (int i=0; i<functionTable_.getLength(); i++)
	{
		FunctionNode *pFunc = getFunction(i);
		for (int j=0; j<pFunc->icodeStream_.getLength(); j++)
		{
			ICodeNode *pCode = pFunc->icodeStream_.getAt(j);
			if (pCode->type == ICODE_INSTR) result++;
		}
	}
	return result;
}

/************************************************************************/
/* 获取函数总数                                                         */
/************************************************************************/
int Parse::getFunctionCount()
{
	return functionTable_.getLength();
}

Stream * Parse::getByteCode()
{
	//iCode.printStream();
	return iCode.getStream();
}