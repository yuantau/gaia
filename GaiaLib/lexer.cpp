#include "lexer.h"

using namespace Gaia;

Lexer::Lexer(void)
{
}


Lexer::~Lexer(void)
{
	sourceList_.clear();
	tokenList_.clear();
}

/************************************************************************/
/* 判断字符是否空白字符                                                 */
/************************************************************************/
BOOL Lexer::isWhitespace(char c)
{
	if (c == ' ' || c == '\t')
		return TRUE;
	return FALSE;
}

/************************************************************************/
/* 判断字符是否是数字                                                   */
/************************************************************************/
BOOL Lexer::isNumberic(char c)
{
	if (c >= '0' && c <= '9')
		return TRUE;
	return FALSE;
}

/************************************************************************/
/* 判断字符是否是分隔符                                                 */
/************************************************************************/
int Lexer::checkDelim(char c)
{
	switch (c)
	{
	case ',':
		return TOKEN_TYPE_DELIM_COMMA;
	case '(':
		return TOKEN_TYPE_DELIM_OPEN_PAREN;
	case ')':
		return TOKEN_TYPE_DELIM_CLOSE_PAREN;
	case '[':
		return TOKEN_TYPE_DELIM_OPEN_BRACE;
	case ']':
		return TOKEN_TYPE_DELIM_CLOSE_BRACE;
	case '{':
		return TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE;
	case '}':
		return TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE;
	case ';':
		return TOKEN_TYPE_DELIM_SEMICOLON;
	case '.':
		return TOKEN_TYPE_DELIM_DOT;
	case ':':
		return TOKEN_TYPE_DELIM_TYPE;
	}
	return -1;
}

/************************************************************************/
/* 判断字符是否是操作符                                                 */
/************************************************************************/
BOOL Lexer::checkOp(OpState *pState, char c)
{
	//所有的操作符
	static OpState ops[] = {
		{'+', 0, 2, {'+', 13, '=', 14,}},		// +
		{'-', 1, 2, {'-', 15, '=', 16,}},		// -
		{'*', 2, 1, {'=', 17}},					// *
		{'/', 3, 1, {'=', 18}},					// /
		{'%', 4, 1, {'=', 19}},					// %
		{'^', 5, 1, {'=', 20}},					// ^
		{'&', 6, 2, {'=', 21, '&', 22}},		// &
		{'|', 7, 2, {'=', 23, '|', 24}},		// |
		{'~', 8, 0, {0,}},						// ~
		{'<', 9, 2, {'<', 25, '=', 26}},		// <
		{'>', 10, 2, {'>', 27, '=', 28}},		// >
		{'!', 11, 1, {'=', 29,}},				// !
		{'=', 12, 1, {'=', 30,}},				// =

		{'+', 13, 0,},							// ++
		{'=', 14, 0,},							// +=
		{'-', 15, 0,},							// -=
		{'=', 16, 0,},							// ==

		{'=', 17, 0,},							// *=
		{'=', 18, 0,},							// /=
		{'=', 19, 0,},							// %=
		{'=', 20, 0,},							// ^=
		{'=', 21, 0,},							// &=
		{'&', 22, 0,},							// &&
		{'=', 23, 0,},							// |=
		{'|', 24, 0,},							// ||
		{'<', 25, 1, {'=', 31,}},				// <<
		{'=', 26, 0,},							// <=
		{'>', 27, 1, {'=', 32,}},				// >>
		{'=', 28, 0,},							// >=
		{'=', 29, 0,},							// !=
		{'=', 30, 0,},							// ==

		{'=', 31, 0,},							//<<=
		{'=', 32, 0,},							//>>=
	};

	if (pState->type == -1)
	{
		//如果之前不是操作符，在操作符起始状态里面找
		for (int i = 0; i < 13; i++)
		{
			if (ops[i].c == c)
			{
				//如果找到，保存当前OpState
				memcpy(pState, &ops[i], sizeof(OpState));
				return TRUE;
			}
		}
	}
	else
	{
		for (int i = 0; i < pState->subStateCount; i++)
		{
			int index = i * 2;
			if (pState->subStateChar[index] == c)
			{
				memcpy(pState, &ops[pState->subStateChar[index + 1]], sizeof(OpState));
				return TRUE;
			}
		}

	}
	return FALSE;
}

/************************************************************************/
/* 判断字符是否是标识符                                                 */
/************************************************************************/
BOOL Lexer::isIdent(char c)
{
	if (c >= '0' && c <= '9' ||
		c >= 'A' && c <= 'Z' ||
		c >= 'a' && c <= 'z' ||
		c == '_')
		return TRUE;
	return FALSE;
}


/************************************************************************/
/* 预处理, 把源代码根据换行符存进链表                                   */
/* 剔除windows换行符 删除注释                                           */
/************************************************************************/
void Lexer::preprocess(char *pSource)
{

	int start = 0;
	int end = 0;
	char *p = (char *)pSource;
	char *pSourceLine;
	int lineSize = 0;

	while (true)
	{
		if (*p == '\n' || (*p == '\0' && end > start))
		{
			//如果遇到换行就添加新行
			lineSize = end - start + 1;
			pSourceLine = new char[lineSize + 1];
			memcpy(pSourceLine, pSource + start, lineSize);
			pSourceLine[lineSize] = '\0';
			sourceList_.push(pSourceLine);
			if (*p == '\0')
				break;
			start = end + 1;
		}
		p++;
		end++;
	}

	char strQuote = '"';
	BOOL isInBlockComment = FALSE;
	BOOL isInString = FALSE;



	for (int i = 0; i < sourceList_.getLength(); i++)
	{
		char *pLine = sourceList_.getAt(i);
		for (int i = 0, len = strlen(pLine); i < len; i++)

		{
			char c = pLine[i];

			//消除windows系统的\r换行符
			if (c == '\r') pLine[i] = ' ';

			//处理字符串状态
			if (!isInString)
			{
				if (c == '"' || c == '\'')
				{
					isInString = TRUE;
					strQuote = c;
				}
			}
			else
			{
				//如果遇到转义符 向前跳一个字符
				if (c == '\\')
				{
					i++;
					continue;
				}
				if (c == strQuote)
					isInString = FALSE;
			}

			//遇到单行注释 直接截断
			if (c == '/' && pLine[i + 1] == '/' && !isInString && !isInBlockComment)
			{
				pLine[i] = '\n';
				pLine[i + 1] = '\0';
				break;
			}

			//检查块注释
			if (c == '/' && pLine[i + 1] == '*' && !isInString && !isInBlockComment)
			{
				isInBlockComment = TRUE;
			}

			if (c == '*' && pLine[i + 1] == '/' && isInBlockComment)
			{
				pLine[i] = ' ';
				pLine[i + 1] = ' ';
				isInBlockComment = FALSE;
			}

			if (isInBlockComment)
			{
				if (pLine[i] != '\n')
					pLine[i] = ' ';
			}
		}
	}

}

/************************************************************************/
/* 读取下一个字符                                                       */
/************************************************************************/
char Lexer::readChar()
{
	char *strLine;
	if (pCurrSourceNode_)
		strLine = pCurrSourceNode_->getData();
	else
		return '\0';

	//如果当前结束位置已经超过源码行长度
	if (startIndex_ >= currSourceLineSize_)
	{
		pCurrSourceNode_ = pCurrSourceNode_->getNext();
		if (pCurrSourceNode_)
		{
			lineIndex_++;
			startIndex_ = endIndex_ = 0;
			strLine = pCurrSourceNode_->getData();
			currSourceLineSize_ = strlen(strLine);
		}
		else
		{
			return '\0';
		}
	}
	return strLine[endIndex_++];
}

/************************************************************************/
/* 检查标识符是否保留字，如果是，返回保留字Token                        */
/************************************************************************/
int Lexer::checkReservedword(char *ident)
{
	static Reservedword rwords[] = {
		{TOKEN_TYPE_RSRVD_VAR,"var"},
		{TOKEN_TYPE_RSRVD_TRUE,"true"},
		{TOKEN_TYPE_RSRVD_FALSE,"false"},
		{TOKEN_TYPE_RSRVD_IF,"if"},
		{TOKEN_TYPE_RSRVD_ELSE,"else"},
		{TOKEN_TYPE_RSRVD_BREAK,"break"},
		{TOKEN_TYPE_RSRVD_CONTINUE,"continue"},
		{TOKEN_TYPE_RSRVD_FOR,"for"},
		{TOKEN_TYPE_RSRVD_WHILE,"while"},
		{TOKEN_TYPE_RSRVD_FUNCTION,"function"},
		{TOKEN_TYPE_RSRVD_RETURN,"return"}
	};
	for (int i = 0; i < 11; i++)
	{
		if (strcmp(ident, rwords[i].ident) == 0)
			return rwords[i].token;
	}
	return TOKEN_TYPE_IDENT;
}

/************************************************************************/
/* 读取下一个Token                                                      */
/************************************************************************/
int Lexer::readToken()
{
	BOOL done = FALSE;
	char c;
	int index = 0;
	BOOL bAdd = TRUE;
	int state = LEX_STATE_START;
	char currStringQuote;
	int currDelim;

	startIndex_ = endIndex_;

	OpState op;
	op.type = -1;

	while (TRUE)
	{
		c = readChar();
		if (c == '\0') break;
		bAdd = TRUE;
		if (state == LEX_STATE_START)
		{
			//如果是空白字符 跳过
			if (isWhitespace(c))
			{
				startIndex_++;
				bAdd = FALSE;
			}
			else if (isNumberic(c)) state = LEX_STATE_INT;
			else if (c == '.')
			{
				//如果之前Token为invalid或者是操作符或: 转换为float状态
				//否则转换为.运算符
				if (currTokenValue_ == TOKEN_TYPE_INVALID ||
					currTokenValue_ == TOKEN_TYPE_OP ||
					currTokenValue_ == TOKEN_TYPE_DELIM_TYPE)
				{
					state = LEX_STATE_FLOAT;
				}
				else
					state = LEX_STATE_DOT;
			}
			else if (isIdent(c)) state = LEX_STATE_IDENT;
			else if ((currDelim = checkDelim(c)) != -1)
			{
				state = LEX_STATE_DELIM;
			}
			else if (c == '\n') state = LEX_STATE_NEWLINE;
			else if (checkOp(&op, c))
			{
				state = LEX_STATE_OP;
			}
			else if (c == '"' || c == '\'')
			{
				bAdd = FALSE;
				currStringQuote = c;
				state = LEX_STATE_STRING;
			}
			else state = LEX_STATE_UNKNOW;
		}
		else if (state == LEX_STATE_INT)
		{
			//如果是int状态，遇到.转为float状态，遇到非数字状态结束
			if (!isNumberic(c))
			{
				if (c == '.') state = LEX_STATE_FLOAT;
				else
				{
					bAdd = FALSE;
					done = TRUE;
				}
			}
		}
		else if (state == LEX_STATE_DOT)
		{
			bAdd = FALSE;
			done = TRUE;
		}
		else  if (state == LEX_STATE_FLOAT)
		{
			//如果是float状态，遇到非数字则完成单词
			if (!isNumberic(c))
			{
				bAdd = FALSE;
				done = TRUE;
			}
		}
		else if (state == LEX_STATE_IDENT)
		{
			//如果是标识符
			if (!isIdent(c))
			{
				bAdd = FALSE;
				done = TRUE;
			}
		}
		else if (state == LEX_STATE_DELIM)
		{
			bAdd = FALSE;
			done = TRUE;
		}
		else if (state == LEX_STATE_NEWLINE)
		{
			bAdd = FALSE;
			done = TRUE;
		}
		else if (state == LEX_STATE_STRING)
		{
			//如果是字符串状态
			if (c == currStringQuote)
			{
				bAdd = FALSE;
				state = LEX_STATE_STRING_CLOSE_QUOTE;
			}
			else if (c == '\\')
			{
				bAdd = FALSE;
				state = LEX_STATE_STRING_ESCAPE;
			}
		}
		else if (state == LEX_STATE_STRING_ESCAPE)
		{
			if (c == 'n')
			{
				c = '\n';
			}
			else if (c == 'r')
			{
				c = '\r';
			}
			else if (c == 't')
			{
				c = '\t';
			}
			state = LEX_STATE_STRING;
		}
		else if (state == LEX_STATE_STRING_CLOSE_QUOTE)
		{
			bAdd = FALSE;
			done = TRUE;
		}
		else if (state == LEX_STATE_OP)
		{
			if (!checkOp(&op, c))
			{
				currOp_ = op.type;
				bAdd = FALSE;
				done = TRUE;
			}
		}
		else if (state == LEX_STATE_UNKNOW)
		{
			done = TRUE;
		}

		if (bAdd) currLexeme_[index++] = c;

		if (done) break;
	}

	currLexeme_[index] = '\0';
	endIndex_--;

	int token;
	switch (state)
	{
	case LEX_STATE_UNKNOW:
		token = TOKEN_TYPE_INVALID;
		break;
	case LEX_STATE_INT:
		token = TOKEN_TYPE_INT;
		break;
	case LEX_STATE_FLOAT:
		token = TOKEN_TYPE_FLOAT;
		break;
	case LEX_STATE_IDENT:
		//如果是标志符，分离出保留字
		token = checkReservedword(currLexeme_);
		break;
	case LEX_STATE_DELIM:
		token = currDelim;
		break;
	case LEX_STATE_STRING_CLOSE_QUOTE:
		token = TOKEN_TYPE_STRING;
		break;
	case LEX_STATE_OP:
		token = TOKEN_TYPE_OP;
		break;
	case LEX_STATE_DOT:
		token = TOKEN_TYPE_DELIM_DOT;
		break;
	case LEX_STATE_NEWLINE:
		token = TOKEN_TYPE_DELIM_NEWLINE;
		break;
	default:
		token = TOKEN_TYPE_END_STREAM;
		break;
	}

	currTokenValue_ = token;
	return token;
}

/************************************************************************/
/* 解析源代码为单词                                                     */
/************************************************************************/
void Lexer::analyze(char *pSource)
{
	//清除源代码链表
	sourceList_.clear();
	tokenList_.clear();

	//预处理
	preprocess(pSource);

	//设置当前源码行为链表头
	pCurrSourceNode_ = sourceList_.getHead();
	if (pCurrSourceNode_) currSourceLineSize_ = (int)strlen(sourceList_.getAt(0));
	lineIndex_ = 0;
	startIndex_ = endIndex_ = 0;
	currTokenValue_ = TOKEN_TYPE_INVALID;
	readIndex_ = 0;

	int tokenValue;
	while (TRUE)
	{
		tokenValue = readToken();
		Token *pToken = new Token;
		pToken->startIndex = startIndex_;
		pToken->line = lineIndex_;
		pToken->value = tokenValue;
		pToken->op = tokenValue == TOKEN_TYPE_OP ? currOp_ : -1;
		strcpy(pToken->lexeme, currLexeme_);
		tokenList_.push(pToken);
		if (tokenValue == TOKEN_TYPE_END_STREAM)
			break;
	}
}

void Lexer::reset()
{
	readIndex_ = 0;
}

/************************************************************************/
/* 获取指定的源代码行                                                   */
/************************************************************************/
char * Lexer::getSourceLine(int index)
{
	return sourceList_.getAt(index);
}

/************************************************************************/
/* 获取当前Token                                                        */
/************************************************************************/
Token * Lexer::getPrevToken()
{
	return tokenList_.getAt(readIndex_ > 0 ? readIndex_ - 1 : 0);
}

/************************************************************************/
/* 获取下一个Token                                                      */
/************************************************************************/
Token *Lexer::getNextToken()
{
	if (readIndex_ >= tokenList_.getLength())
	{
		return tokenList_.getTail()->getData();
	}
	return tokenList_.getAt(readIndex_++);
}

/************************************************************************/
/* 回退一个Token                                                        */
/************************************************************************/
void Lexer::rewindToken()
{
	readIndex_--;
}

/************************************************************************/
/* 检查下一个Token是不是指定的类型，如果不是，抛出错误                  */
/************************************************************************/
Token *Lexer::checkToken(int val)
{
	Token *pToken = getNextToken();
	if (pToken->value != val)
	{
		char pstrErrorMssg[200];
		switch (val)
		{
		case TOKEN_TYPE_INT:
			strcpy(pstrErrorMssg, "need int");
			break;
		case TOKEN_TYPE_FLOAT:
			strcpy(pstrErrorMssg, "need float");
			break;
		case TOKEN_TYPE_IDENT:
			strcpy(pstrErrorMssg, "need ident");
			break;
		case TOKEN_TYPE_RSRVD_VAR:
			strcpy(pstrErrorMssg, "need [var]");
			break;
		case TOKEN_TYPE_RSRVD_TRUE:
			strcpy(pstrErrorMssg, "need [true]");
			break;
		case TOKEN_TYPE_RSRVD_FALSE:
			strcpy(pstrErrorMssg, "need [false]");
			break;
		case TOKEN_TYPE_RSRVD_IF:
			strcpy(pstrErrorMssg, "need [if]");
			break;
		case TOKEN_TYPE_RSRVD_ELSE:
			strcpy(pstrErrorMssg, "need [else]");
			break;
		case TOKEN_TYPE_RSRVD_BREAK:
			strcpy(pstrErrorMssg, "need [break]");
			break;
		case TOKEN_TYPE_RSRVD_CONTINUE:
			strcpy(pstrErrorMssg, "need [continue]");
			break;
		case TOKEN_TYPE_RSRVD_FOR:
			strcpy(pstrErrorMssg, "need [for]");
			break;
		case TOKEN_TYPE_RSRVD_WHILE:
			strcpy(pstrErrorMssg, "need [while]");
			break;
		case TOKEN_TYPE_RSRVD_FUNCTION:
			strcpy(pstrErrorMssg, "need [function]");
			break;
		case TOKEN_TYPE_RSRVD_RETURN:
			strcpy(pstrErrorMssg, "need [return]");
			break;
		case TOKEN_TYPE_OP:
			strcpy(pstrErrorMssg, "need op");
			break;
		case TOKEN_TYPE_DELIM_COMMA:
			strcpy(pstrErrorMssg, "need [,]");
			break;
		case TOKEN_TYPE_DELIM_OPEN_PAREN:
			strcpy(pstrErrorMssg, "need [(]");
			break;
		case TOKEN_TYPE_DELIM_CLOSE_PAREN:
			strcpy(pstrErrorMssg, "need [)]");
			break;
		case TOKEN_TYPE_DELIM_OPEN_BRACE:
			strcpy(pstrErrorMssg, "need [[]");
			break;
		case TOKEN_TYPE_DELIM_CLOSE_BRACE:
			strcpy(pstrErrorMssg, "need []]");
			break;
		case TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE:
			strcpy(pstrErrorMssg, "need [{]");
			break;
		case TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE:
			strcpy(pstrErrorMssg, "need [}]");
			break;
		case TOKEN_TYPE_DELIM_SEMICOLON:
			strcpy(pstrErrorMssg, "need [;]");
			break;
		case TOKEN_TYPE_STRING:
			strcpy(pstrErrorMssg, "need string");
			break;
		}
		error.code = GERROR_INVALID_TOKEN;
		setErrorMsg(pToken->line, pToken->startIndex, pstrErrorMssg);
		throw error.code;
		return NULL;
	}
	return pToken;
}

/************************************************************************/
/* 设置错误消息                                                         */
/************************************************************************/
void Lexer::setErrorMsg(int line, int index, char *msg)
{
	error.line = line + 1;
	error.index = index + 1;
	strcpy(error.source, sourceList_.getAt(line));
	strcpy(error.msg, msg);
}

/************************************************************************/
/* 向前查看一个字符                                                     */
/************************************************************************/
char Lexer::lookAhead()
{
	if (readIndex_ < tokenList_.getLength())
	{
		Token *p = tokenList_.getAt(readIndex_);
		//如果是字符串返回任意可见字符
		if (p->value == TOKEN_TYPE_STRING)
			return 's';
		return tokenList_.getAt(readIndex_)->lexeme[0];
	}
	return 0;
}