#include "ICode.h"

using namespace Gaia;


ICodeOp * ICodeNode::addOp(int type)
{
	ICodeOp *pOp = new ICodeOp;
	pOp->type_ = type;
	opList.push(pOp);
	return pOp;
}

ICodeOp * ICodeNode::getOp(int index)
{
	return opList.getAt(index);
}


void FunctionNode::addSourceLine(char *source)
{
	ICodeNode *pCode = new ICodeNode;
	pCode->type = ICODE_SOURCE_LINE;
	pCode->source = source;
	icodeStream_.push(pCode);
}


void FunctionNode::addJumpTarget(int targetIndex)
{
	ICodeNode *pCode = new ICodeNode;
	pCode->type = ICODE_JUMP_TARGET;
	pCode->jumpTarget = targetIndex;
	icodeStream_.push(pCode);
}


void FunctionNode::clear()
{
	icodeStream_.clear();
}


ICodeNode * FunctionNode::addInstr(int opCode)
{
	ICodeNode *pCode = new ICodeNode;
	pCode->type = ICODE_INSTR;
	pCode->opCode = opCode;
	icodeStream_.push(pCode);
	return pCode;
}



char * FunctionNode::getName()
{
	return name_;
}

int FunctionNode::getIcodeSize()
{
	return icodeStream_.getLength();
}


ICodeNode *FunctionNode::getIcode(int index)
{
	return icodeStream_.getAt(index);
}


ICode::ICode(void)
{
}


ICode::~ICode(void)
{
}

void ICode::generateAsmFile()
{

}

void ICode::generateByteCode(List<FunctionNode *> *pFuncTable, List<SymbolNode *> *pSymbolTable, List<char *> *pStringTable)
{
	pSymbolTable_= pSymbolTable;
	pFuncTable_ = pFuncTable;
	pStringTable_ = pStringTable;
	
	labelTable_.clear();
	funcTable_.clear();

	globalVarSize_ = 0;

	byteCode_.setPosition(0);
	byteCode_.writeString("GAIAASM");
	byteCode_.writeInt(pStringTable->getLength());
	for (int i=0; i<pStringTable->getLength(); i++)
	{
		char *str = pStringTable->getAt(i);
		int strLen = (int)strlen(str);
		byteCode_.writeInt(strLen);
		byteCode_.writeString(str);
	}
	
	int headerSize = byteCode_.getPosition();

	byteCode_.writeInt(0);

	//global variable size
	for (int i=0; i<pSymbolTable->getLength(); i++)
	{
		SymbolNode *pSymbol = pSymbolTable->getAt(i);
		if (pSymbol->scope == 0)
		{
			byteCode_.writeInt(strlen(pSymbol->name));
			byteCode_.writeString(pSymbol->name);
			globalVarSize_++;
		}
	}
	int paramNameSize = byteCode_.getPosition();
	byteCode_.setPosition(headerSize);

	byteCode_.writeInt(globalVarSize_);
	byteCode_.setPosition(paramNameSize);



	byteCode_.writeInt(pFuncTable->getLength()-1);
	int instrIndex = 0;
	for (int i=0; i<pFuncTable->getLength(); i++)
	{
		FunctionNode *pFunc = pFuncTable->getAt(i);

		Function *pRuntimeFunc = new Function;
		funcTable_.push(pRuntimeFunc);
		pRuntimeFunc->entryPoint = instrIndex;
		pRuntimeFunc->localSize = pFunc->getLocalDataSize();
		pRuntimeFunc->returnSize = pFunc->getReturnSize();

		if (i > 0)
		{
			int len = strlen(pFunc->getName());
			byteCode_.writeInt(len + sizeof(int) * 4);
			byteCode_.writeInt(pRuntimeFunc->entryPoint);
			byteCode_.writeInt(pFunc->getParamCount());
			byteCode_.writeInt(pRuntimeFunc->localSize);
			byteCode_.writeInt(pRuntimeFunc->returnSize);
			byteCode_.writeString(pFunc->getName());
		}
		
		for (int j=0; j<pFunc->getIcodeSize(); j++)
		{
			ICodeNode *pCode = pFunc->getIcode(j);

			if (pCode->type == ICODE_INSTR)
			{
				instrIndex++;
			}
			else if (pCode->type == ICODE_JUMP_TARGET)
			{
				Label *pLabel = new Label;
				pLabel->index = pCode->jumpTarget;
				pLabel->target = instrIndex;
				labelTable_.push(pLabel);
			}

		}
	}

	byteCode_.writeInt(instrIndex);

	instrIndex = 0;
	
	for (int i=0; i<pFuncTable->getLength(); i++)
	{
		FunctionNode *pFunc = pFuncTable->getAt(i);

		for (int j=0; j<pFunc->getIcodeSize(); j++)
		{
			ICodeNode *pCode = pFunc->getIcode(j);
			if (pCode->type == ICODE_INSTR)
			{
				
				byteCode_.writeInt(pCode->opCode);
				
				byteCode_.writeInt(pCode->opList.getLength());

				for (int k=0, len=pCode->opList.getLength(); k<len; k++)
				{
					ICodeOp *source = pCode->getOp(k);
					ICodeOp dest;
					copyOp(&dest, source, pFunc);

				
					byteCode_.writeInt(dest.type_);
					switch (dest.type_)
					{
					case GAIA_OP_INT:
						byteCode_.writeInt(dest.intLiteral);
						break;
					case GAIA_OP_NUMBER:
						byteCode_.writeFloat(dest.floatLiteral);
						break;
					case GAIA_OP_STRING:
						{
							
							byteCode_.writeInt(dest.stringIndex);
							break;
						}
					case GAIA_OP_FUNC_INDEX:
						{
							//jmp addr & local size
							byteCode_.writeInt(dest.intLiteral);
							byteCode_.writeInt(dest.offset);
							break;
						}
					case GAIA_OP_INSTR:
						byteCode_.writeInt(dest.jumpTarget);
						break;
					case GAIA_OP_STACK_INDEX:
						byteCode_.writeInt(dest.intLiteral);
						break;
					}
				}
				instrIndex++;
			}
		}
	}
}

void ICode::printStream()
{
	byteCode_.setPosition(0);
	char buffer[1024];
	byteCode_.readBytes(buffer, 8);
	printf("%s\n", buffer);
	printf("==================\nGlobal Variable:\n");
	int globalSize = byteCode_.readInt();
	for (int i=0; i<globalSize; i++)
	{
		byteCode_.readBytes(buffer, byteCode_.readInt()+1);
		printf("  %s\n", buffer);
	}
	printf("==================\n\n");

	printf("==================\nFunctions:\n");
	int funcSize = byteCode_.readInt();
	for (int i=0; i<funcSize; i++)
	{
		int len = byteCode_.readInt();
		int entryPoint = byteCode_.readInt();
		int paramCount = byteCode_.readInt();
		int localSize = byteCode_.readInt();
		byteCode_.readBytes(buffer, len - sizeof(int) * 3 + 1);
		printf("  %-05s Entry:%-03d, Param:%-03d,  Local:%-03d\n", buffer, entryPoint, paramCount, localSize);
	}
	printf("==================\n\n");

	int instrSize = byteCode_.readInt();
	printf("==================\nInstr %d\n", instrSize);

	for (int i=0; i<instrSize; i++)
	{
		int opCode = byteCode_.readInt();
		int opCount = byteCode_.readInt();

		printf("%-02d %-05s ", i, getMnemonics(opCode), opCount);

		for (int j=0; j<opCount; j++)
		{
			int opType = byteCode_.readInt();
			switch (opType)
			{
			case GAIA_OP_INT:
			case GAIA_OP_INSTR:
			case GAIA_OP_STACK_INDEX:
				printf("{%d} ", byteCode_.readInt());
				break;
			case GAIA_OP_EAX:
				printf("eax ");
				break;
			case GAIA_OP_EBX:
				printf("ebx ");
				break;
			case GAIA_OP_NUMBER:
				printf("{%f} ", byteCode_.readFloat());
				break;
			case GAIA_OP_STRING:
				{
					int len = byteCode_.readInt();
					byteCode_.readBytes(buffer, len+1);
					printf("\"%s\" ", buffer);
					break;
				}
			case GAIA_OP_FUNC_INDEX:
				{
					printf("%d ", byteCode_.readInt());
					printf("%d ", byteCode_.readInt());
					break;
				}
			}
		}

		printf("\n");
	}
}

void ICode::copyOp(ICodeOp *pDest, ICodeOp *pSource, FunctionNode *pFunc)
{
	switch (pSource->type_)
	{
	case GAIA_OP_INT:
		pDest->type_ = GAIA_OP_INT;
		pDest->intLiteral = pSource->intLiteral;
		break;
	case GAIA_OP_NUMBER:
		pDest->type_ = GAIA_OP_NUMBER;
		pDest->floatLiteral = pSource->floatLiteral;
		break;
	case GAIA_OP_STRING:
		pDest->type_ = GAIA_OP_STRING;
		pDest->stringIndex = pSource->stringIndex;
		break;
	case GAIA_OP_EAX:
	case GAIA_OP_EBX:
		pDest->type_ = pSource->type_;
		break;
	case GAIA_OP_VAR:
		{
			SymbolNode *pSymbol =  pSymbolTable_->getAt(pSource->symbolIndex);
			int stackIndex = pSymbol->stackIndex;
			if (pSymbol->isParam)
			{
				stackIndex = -(pFunc->getLocalDataSize() + pSymbol->stackIndex + 2);
			}
			pDest->type_ = GAIA_OP_STACK_INDEX;
			pDest->intLiteral = stackIndex;
			break;
		}
	case GAIA_OP_JUMP_TARGET_INDEX:
		{
			int target = 0;
			for (int i=0; i<labelTable_.getLength(); i++)
			{
				if (labelTable_.getAt(i)->index == pSource->jumpTarget)
				{
					target = labelTable_.getAt(i)->target;
					break;
				}
			}
			pDest->type_ = GAIA_OP_INSTR;
			pDest->jumpTarget = target;
			break;
		}
	case GAIA_OP_FUNC_INDEX:
		{
			pDest->type_ = GAIA_OP_FUNC_INDEX;
			if (pSource->funcIndex > -1)
			{
				Function *p = funcTable_.getAt(pSource->funcIndex);
				pDest->intLiteral = p->entryPoint;
				pDest->offset = p->localSize;
				
			}
			else 
			{
				pDest->intLiteral = -(pSource->funcIndex + 1);
				pDest->offset = -1;
			
			}
			
			
			break;
		}
	}
}

Stream * ICode::getStream()
{
	return &byteCode_;
}