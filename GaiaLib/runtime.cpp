#include "runtime.h"

using namespace Gaia;

Runtime::Runtime(void) : stackSize_(1024), pStack_(NULL), pInstrs_(NULL)
{
    stackTop_ = stackLocal_ = 0;
}

Runtime::~Runtime(void)
{
}

void Runtime::begin(Stream *pCode, Script *pScript)
{

    pScript_ = pScript;
    varTable_.clear();
    funcTable_.clear();
    if (pInstrs_ != NULL)
        delete[] pInstrs_;

    stackLocal_ = stackTop_ = 0;
    if (pStack_ != NULL)
        delete[] pStack_;
    pStack_ = new GaiaValue[stackSize_];

    gaiaStringTable_.clear();

    eax.type = ebx.type = GAIA_OP_NULL;

    pCode->setPosition(8);

    int stringTableSize = pCode->readInt();

    for (int i = 0; i < stringTableSize; i++)
    {
        int strLen = pCode->readInt();
        GaiaString *pStrObj = Gaia_NewString(strLen);
        pCode->readBytes(pStrObj->strValue, strLen + 1);
        pStrObj->hashVal = hash(pStrObj->strValue);
    }

    int globalSize = pCode->readInt();
    //为全局数据分配空间
    addFrame(globalSize);

    char buffer[1024];
    for (int i = 0; i < globalSize; i++)
    {
        //存储变量名
        int varNameLength = pCode->readInt() + 1;
        pCode->readBytes(buffer, varNameLength);
        varTable_.push(buffer, varNameLength);
    }

    int funcSize = pCode->readInt();
    for (int i = 0; i < funcSize; i++)
    {
        int len = pCode->readInt();
        int entryPoint = pCode->readInt();
        int paramCount = pCode->readInt();
        int localSize = pCode->readInt();
        int returnSize = pCode->readInt();

        Function *p = new Function;
        p->entryPoint = entryPoint;
        p->paramCount = paramCount;
        p->localSize = localSize;
        p->returnSize = returnSize;
        pCode->readBytes(buffer, len - sizeof(int) * 4 + 1);
        p->hashVal = hash(buffer);
        funcTable_.push(p);
    }

    instrSize_ = pCode->readInt();
    pInstrs_ = new Instr[instrSize_]; //指令分配空间

    for (int i = 0; i < instrSize_; i++)
    {
        int opCode = pCode->readInt();
        int opCount = pCode->readInt();

        pInstrs_[i].opCode = opCode;
        pInstrs_[i].opCount = opCount;

        for (int j = 0; j < opCount; j++)
        {
            int opType = pCode->readInt();
            pInstrs_[i].opList[j].type = opType;
            switch (opType)
            {
            case GAIA_OP_INT:
            {
                int val = pCode->readInt();
                pInstrs_[i].opList[j].intValue = val;
                pInstrs_[i].opList[j].type = GAIA_OP_INT;
                break;
            }
            case GAIA_OP_INSTR:
            {
                int val = pCode->readInt();
                pInstrs_[i].opList[j].intValue = val;
                pInstrs_[i].opList[j].type = GAIA_OP_INSTR;
                break;
            }
            case GAIA_OP_STACK_INDEX:
            {
                int val = pCode->readInt();
                pInstrs_[i].opList[j].intValue = val;
                pInstrs_[i].opList[j].type = GAIA_OP_STACK_INDEX;
                break;
            }
            case GAIA_OP_EAX:
                pInstrs_[i].opList[j].type = GAIA_OP_EAX;
                break;
            case GAIA_OP_EBX:
                pInstrs_[i].opList[j].type = GAIA_OP_EBX;
                break;
            case GAIA_OP_NUMBER:
            {
                float val = pCode->readFloat();
                pInstrs_[i].opList[j].numberValue = val;
                pInstrs_[i].opList[j].type = GAIA_OP_NUMBER;
                break;
            }
            case GAIA_OP_STRING:
            {

                //取GaiaString对象 ref设为1
                pInstrs_[i].opList[j].type = GAIA_OP_STRING;
                GaiaString *pStrObj = gaiaStringTable_.getAt(pCode->readInt());
                pStrObj->ref = 1;
                pInstrs_[i].opList[j].pObj = pStrObj;

                break;
            }
            case GAIA_OP_FUNC_INDEX:
            {
                pInstrs_[i].opList[j].type = GAIA_OP_INSTR;
                int instrIndex = pCode->readInt();
                int localSize = pCode->readInt();

                pInstrs_[i].opList[j].intValue = instrIndex;
                pInstrs_[i].opList[j].size = localSize;
                break;
            }
            }
        }
    }

    ip_ = 0;
    GaiaValue val;
    val.type = GAIA_OP_INT;
    val.intValue = 0;
    val.size = 0;
    addFrame(1);
    setStack(-1, val);

    run();
}

void Runtime::GC()
{
    ListNode<GaiaString *> *pNode = gaiaStringTable_.getHead();
    while (pNode)
    {
        if (pNode->getData()->ref <= 0)
        {
            ListNode<GaiaString *> *pNext = pNode->getNext();
            gaiaStringTable_.del(pNode);
            pNode = pNext;
            continue;
        }
        pNode = pNode->getNext();
    }
    printf("%d\n", gaiaStringTable_.getLength());
}

GaiaValue Runtime::pop()
{
    stackTop_--;
    GaiaValue val = pStack_[stackTop_];
    if (pStack_[stackTop_].type == GAIA_OP_STRING)
    {
        ((GaiaString *)pStack_[stackTop_].pObj)->ref--;
        pStack_[stackTop_].type = GAIA_OP_NULL;
    }
    return val;
}

using namespace std;
void Runtime::run()
{

    Instr *pCurrent;
    int currIp = 0;
    int retFound = 1;
    while (TRUE)
    {

        //printf("%d\n", retFound);
        if (retFound == 0)
            break;

        currIp = ip_;
        pCurrent = &pInstrs_[ip_];

        /*	
		static char Commands[200];
		cin.getline(Commands, 200);
		if(strcmp(Commands, "t") == 0)
		{
			printf("StackTop: %s\n", getValueType(pStack_[stackTop_-1].type));
		}
		else if(strcmp(Commands, "d") == 0)
		{
			printf("a:%s\n", getValueType(pStack_[0].type));
		}
		printf("IP:%d %s stacktop:%d %d\n", ip_, getMnemonics(pCurrent->opCode), stackTop_, retFound);*/
        switch (pCurrent->opCode)
        {
        case INSTR_NEW_OBJ:
        {
            GaiaObject *pObj = Gaia_NewObject();
            pObj->ref = 0;
            GaiaValue val;
            val.type = GAIA_OP_OBJECT;
            val.pObj = pObj;
            push(val);
            break;
        }
        case INSTR_GET_FIELD:
        {
            GaiaValue key = pop();
            GaiaValue obj = pop();
            printf("%d", obj.pObj->ref);

            break;
        }
        case INSTR_SET_FIELD:
        {
            GaiaValue val = pop();
            GaiaValue key = pop();
            GaiaValue dest = pop();

            GaiaValue *s = Gaia_NewBasicValue(val.intValue);
            ((GaiaObject *)dest.pObj)->setValue(((GaiaString *)key.pObj)->strValue, s);

            printf("%s ", getValueType(val.type));
            printf("%s ", getValueType(key.type));
            printf("%s ", getValueType(dest.type));
            break;
        }
        case INSTR_MOD:
        {
            GaiaValue *pDest = getOpValuePtr(0);
            GaiaValue source = getOpValue(1);
            int a = getValueAsInt(pDest);
            int b = getValueAsInt(&source);
            pDest->type = GAIA_OP_INT;
            pDest->intValue = b % a;
            break;
        }
        case INSTR_DIV:
        {
            GaiaValue *pDest = getOpValuePtr(0);
            GaiaValue source = getOpValue(1);
            if (pDest->type == GAIA_OP_NUMBER || source.type == GAIA_OP_NUMBER)
            {
                pDest->type = GAIA_OP_NUMBER;
                pDest->numberValue = getValueAsNumber(pDest) / getValueAsNumber(&source);
            }
            else
            {
                int a = getValueAsInt(pDest);
                int b = getValueAsInt(&source);

                if (a >= b && b % a == 0)
                {

                    pDest->type = GAIA_OP_INT;
                    pDest->intValue = a / b;
                }
                else
                {
                    pDest->type = GAIA_OP_NUMBER;
                    pDest->numberValue = (float)(a * 1.0 / b * 1.0);
                }
            }
            break;
        }
        case INSTR_MUL:
        {
            GaiaValue *pDest = getOpValuePtr(0);
            GaiaValue source = getOpValue(1);
            if (pDest->type == GAIA_OP_INT)
            {

                if (source.type == GAIA_OP_INT)
                {
                    int num = getValueAsInt(pDest) * getValueAsInt(&source);
                    pDest->intValue = num;
                }
                else if (source.type == GAIA_OP_NUMBER)
                {
                    float num = getValueAsNumber(pDest) * getValueAsNumber(&source);
                    pDest->numberValue = num;
                    pDest->type = GAIA_OP_NUMBER;
                }
            }
            else if (pDest->type == GAIA_OP_NUMBER)
            {
                float num = getValueAsNumber(pDest) * getValueAsNumber(&source);
                pDest->numberValue = num;
            }
            break;
        }
        case INSTR_SUB:
        {

            GaiaValue *pDest = getOpValuePtr(0);
            GaiaValue source = getOpValue(1);
            if (pDest->type == GAIA_OP_INT)
            {

                if (source.type == GAIA_OP_NUMBER)
                {
                    float num = getValueAsNumber(pDest) - getValueAsNumber(&source);
                    pDest->type = GAIA_OP_NUMBER;
                    pDest->numberValue = num;
                }
                else if (source.type == GAIA_OP_INT)
                {
                    int num = getValueAsInt(pDest) - getValueAsInt(&source);
                    pDest->type = GAIA_OP_INT;
                    pDest->intValue = num;
                }
            }
            else if (pDest->type == GAIA_OP_NUMBER)
            {
                float num = getValueAsNumber(pDest) - getValueAsNumber(&source);
                pDest->type = GAIA_OP_NUMBER;
                pDest->numberValue = num;
            }
            break;
        }
        case INSTR_ADD:
        {
            GaiaValue *pDest = getOpValuePtr(0);
            GaiaValue source = getOpValue(1);
            if (pDest->type == GAIA_OP_STRING || source.type == GAIA_OP_STRING)
            {

                if (pDest->type == GAIA_OP_STRING)
                    ((GaiaString *)pDest->pObj)->ref--;

                getValueAsString(strT0_, pDest);
                getValueAsString(strT1_, &source);
                int t1 = strlen(strT0_);
                GaiaString *pStrObj = Gaia_NewString(t1 + strlen(strT1_));
                strcpy(pStrObj->strValue, strT0_);
                strcpy(pStrObj->strValue + t1, strT1_);
                pStrObj->hashVal = hash(pStrObj->strValue);

                pStrObj->ref = 1;
                pDest->type = GAIA_OP_STRING;
                pDest->pObj = pStrObj;
            }
            else
            {
                if (pDest->type == GAIA_OP_NUMBER || source.type == GAIA_OP_NUMBER)
                {
                    float num = getValueAsNumber(pDest) + getValueAsNumber(&source);
                    pDest->type = GAIA_OP_NUMBER;
                    pDest->numberValue = num;
                }
                else
                {
                    pDest->type = GAIA_OP_INT;
                    pDest->intValue = getValueAsInt(pDest) + getValueAsInt(&source);
                }
            }
            break;
        }
        case INSTR_PUSH:
        {
            GaiaValue val = getOpValue(0);
            push(val);
            break;
        }
        case INSTR_POP:
        {
            GaiaValue *pVal = getOpValuePtr(0);
            GaiaValue source = pop();

            if (pVal->type == GAIA_OP_STRING)
                ((GaiaString *)pVal->pObj)->ref--;
            *pVal = source;
            if (pVal->type == GAIA_OP_STRING)
                ((GaiaString *)pVal->pObj)->ref++;

            break;
        }
        case INSTR_MOV:
        {
            GaiaValue *dest = getOpValuePtr(0);
            if (dest->type == GAIA_OP_STRING)
                ((GaiaString *)dest->pObj)->ref--;

            GaiaValue source = getOpValue(1);
            *dest = source;

            if (dest->type == GAIA_OP_STRING)
                ((GaiaString *)dest->pObj)->ref++;

            break;
        }
        case INSTR_CALL:
        {
            GaiaValue *val = getOpValuePtr(0);
            int target = val->intValue;
            int paramSize = val->size;

            //paramSize == -1表示local函数
            if (paramSize > -1)
            {
                retFound++;
                //添加局部变量空间
                addFrame(paramSize + 1);

                GaiaValue bp;
                bp.type = GAIA_OP_INT;
                bp.intValue = paramSize;
                bp.size = ip_ + 1;

                setStack(-1, bp);
                ip_ = target;
            }
            else
            {
                Function *pLocal = localFuncTable_.getAt(target);
                int ret = pLocal->pfn(pScript_);
                if (ret > 0)
                {
                    //如果ret > 0，取栈顶返回值到eax中
                    GaiaValue source = pop();
                    if (eax.type == GAIA_OP_STRING)
                        ((GaiaString *)eax.pObj)->ref--;
                    eax = source;
                    if (eax.type == GAIA_OP_STRING)
                        ((GaiaString *)eax.pObj)->ref++;
                }
            }

            break;
        }
        case INSTR_JG:
        {
            BOOL jump = FALSE;
            GaiaValue val0 = getOpValue(0);
            GaiaValue val1 = getOpValue(1);

            int target = getOpValue(2).intValue;

            if (val0.type == GAIA_OP_INT)
            {
                if (getValueAsInt(&val0) > getValueAsInt(&val1))
                    jump = TRUE;
            }
            else if (val0.type == GAIA_OP_NUMBER)
            {
                if (getValueAsNumber(&val0) > getValueAsNumber(&val1))
                    jump = TRUE;
            }
            if (jump)
                ip_ = target;
            break;
        }
        case INSTR_JGE:
        {
            BOOL jump = FALSE;
            GaiaValue val0 = getOpValue(0);
            GaiaValue val1 = getOpValue(1);

            int target = getOpValue(2).intValue;

            if (val0.type == GAIA_OP_INT)
            {
                if (getValueAsInt(&val0) >= getValueAsInt(&val1))
                    jump = TRUE;
            }
            else if (val0.type == GAIA_OP_NUMBER)
            {
                if (getValueAsNumber(&val0) >= getValueAsNumber(&val1))
                    jump = TRUE;
            }
            if (jump)
                ip_ = target;
            break;
        }
        case INSTR_JL:
        {
            BOOL jump = FALSE;
            GaiaValue val0 = getOpValue(0);
            GaiaValue val1 = getOpValue(1);

            int target = getOpValue(2).intValue;

            if (val0.type == GAIA_OP_INT)
            {
                if (getValueAsInt(&val0) < getValueAsInt(&val1))
                    jump = TRUE;
            }
            else if (val0.type == GAIA_OP_NUMBER)
            {
                if (getValueAsNumber(&val0) < getValueAsNumber(&val1))
                    jump = TRUE;
            }
            if (jump)
                ip_ = target;
            break;
        }
        case INSTR_JE:
        {

            BOOL jump = FALSE;
            GaiaValue val0 = getOpValue(0);
            GaiaValue val1 = getOpValue(1);
            int target = getOpValue(2).intValue;

            if (val0.type == GAIA_OP_INT && val1.type == GAIA_OP_INT)
            {
                if (getValueAsInt(&val0) == getValueAsInt(&val1))
                    jump = TRUE;
            }
            if (val0.type == GAIA_OP_NUMBER && val1.type == GAIA_OP_NUMBER)
            {
                if (getValueAsNumber(&val0) == getValueAsNumber(&val1))
                    jump = TRUE;
            }
            if (val0.type == GAIA_OP_STRING && val1.type == GAIA_OP_STRING)
            {
                if (((GaiaString *)val0.pObj)->hashVal == ((GaiaString *)val1.pObj)->hashVal)
                    jump = TRUE;
            }
            if (jump)
                ip_ = target;
            break;
        }
        case INSTR_NEG:
        {
            GaiaValue *ptr = getOpValuePtr(0);
            if (ptr->type == GAIA_OP_INT)
                ptr->intValue = -ptr->intValue;
            else if (ptr->type == GAIA_OP_NUMBER)
                ptr->numberValue = -ptr->numberValue;
            break;
        }
        case INSTR_INC:
        {
            GaiaValue *ptr = getOpValuePtr(0);
            if (ptr->type == GAIA_OP_INT)
                ptr->intValue += 1;
            else if (ptr->type == GAIA_OP_NUMBER)
                ptr->numberValue += 1;
            break;
        }
        case INSTR_DEC:
        {
            GaiaValue *ptr = getOpValuePtr(0);
            if (ptr->type == GAIA_OP_INT)
                ptr->intValue -= 1;
            else if (ptr->type == GAIA_OP_NUMBER)
                ptr->numberValue -= 1;
            break;
        }
        case INSTR_JLE:
        {
            BOOL jump = FALSE;
            GaiaValue val0 = getOpValue(0);
            GaiaValue val1 = getOpValue(1);

            int target = getOpValue(2).intValue;

            if (val0.type == GAIA_OP_INT)
            {
                if (getValueAsInt(&val0) <= getValueAsInt(&val1))
                    jump = TRUE;
            }
            else if (val0.type == GAIA_OP_NUMBER)
            {
                if (getValueAsNumber(&val0) <= getValueAsNumber(&val1))
                    jump = TRUE;
            }
            if (jump)
                ip_ = target;
            break;
        }
        case INSTR_JMP:
        {
            GaiaValue val0 = getOpValue(0);
            ip_ = val0.intValue;
            break;
        }
        case INSTR_RET:
            retFound--;

            GaiaValue val = pop();
            delFrame(val.intValue + getOpValue(0).intValue);
            ip_ = val.size;
            break;
        }

        if (ip_ == currIp)
            ip_++;
    }

    if (eax.type == GAIA_OP_STRING)
    {
        ((GaiaString *)eax.pObj)->ref--;
    }
    if (ebx.type == GAIA_OP_STRING)
    {
        ((GaiaString *)ebx.pObj)->ref--;
    }
}

void Runtime::getValueAsString(char *buf, GaiaValue *pVal)
{
    if (pVal->type == GAIA_OP_STRING)
    {
        strcpy(buf, ((GaiaString *)pVal->pObj)->strValue);
    }
    else
    {
        switch (pVal->type)
        {
        case GAIA_OP_INT:
            sprintf(buf, "%d", pVal->intValue);
            break;
        case GAIA_OP_NUMBER:
            sprintf(buf, "%f", pVal->numberValue);
            break;
        default:
            sprintf(buf, "%s", "null");
        }
    }
}

int Runtime::getValueAsInt(GaiaValue *pVal)
{
    if (pVal->type == GAIA_OP_INT)
        return pVal->intValue;
    if (pVal->type == GAIA_OP_NUMBER)
        return (int)pVal->numberValue;
    return 0;
}

float Runtime::getValueAsNumber(GaiaValue *pVal)
{
    if (pVal->type == GAIA_OP_INT)
        return (float)pVal->intValue;
    if (pVal->type == GAIA_OP_NUMBER)
        return pVal->numberValue;
    return 0;
}

GaiaValue Runtime::getOpValue(int index)
{
    GaiaValue val = pInstrs_[ip_].opList[index];
    switch (val.type)
    {
    case GAIA_OP_STACK_INDEX:
    {
        int stackIndex = val.intValue;
        return pStack_[stackIndex < 0 ? (stackIndex + stackLocal_) : stackIndex];
    }
    case GAIA_OP_EAX:
        return eax;
    case GAIA_OP_EBX:
        return ebx;
    }
    return val;
}

GaiaValue *Runtime::getOpValuePtr(int index)
{
    int type = pInstrs_[ip_].opList[index].type;
    switch (type)
    {
    case GAIA_OP_EAX:
        return &eax;
    case GAIA_OP_EBX:
        return &ebx;
    case GAIA_OP_STACK_INDEX:
    {
        int stackIndex = pInstrs_[ip_].opList[index].intValue;
        if (stackIndex < 0)
            stackIndex += stackLocal_;
        return &pStack_[stackIndex];
    }
    }
    return &pInstrs_[ip_].opList[index];
}

Function *Runtime::findLocalFunction(char *name)
{
    int hashv = hash(name);
    ListNode<Function *> *pNode = localFuncTable_.getHead();
    while (pNode != NULL)
    {
        if (pNode->getData()->hashVal == hashv)
        {
            return pNode->getData();
        }
        pNode = pNode->getNext();
    }
    return NULL;
}

BOOL Runtime::pushLocalFunction(char *name, pLocalFn fn)
{
    if (findLocalFunction(name))
        return FALSE;

    Function *pfn = new Function;
    pfn->entryPoint = -1;
    pfn->hashVal = hash(name);
    pfn->pfn = fn;
    int index = localFuncTable_.push(pfn);
    pfn->index = index;
    return TRUE;
}

BOOL Runtime::call(char *name)
{
    int hashVal = hash(name);
    for (int i = 0; i < funcTable_.getLength(); i++)
    {
        Function *pFunc = funcTable_.getAt(i);
        if (pFunc->hashVal == hashVal)
        {
            ip_ = pFunc->entryPoint;
            addFrame(pFunc->localSize + 1);
            GaiaValue bp;
            bp.type = GAIA_OP_INT;
            bp.intValue = pFunc->localSize;
            bp.size = ip_ + 1;

            setStack(-1, bp);
            run();
            if (pFunc->returnSize > 0)
            {
                push(eax);
            }
            return TRUE;
        }
    }
    return FALSE;
}

BOOL Runtime::getGlobal(char *name)
{
    for (int i = 0; i < varTable_.getLength(); i++)
    {
        if (strcmp(varTable_.getAt(i), name) == 0)
        {
            GaiaValue v = pStack_[i];
            push(v);
            return TRUE;
        }
    }
    return FALSE;
}

BOOL Runtime::isInt()
{
    return pStack_[stackTop_ - 1].type == GAIA_OP_INT;
}

BOOL Runtime::IsNumber()
{
    return (pStack_[stackTop_ - 1].type == GAIA_OP_NUMBER) || (pStack_[stackTop_ - 1].type == GAIA_OP_INT);
}

BOOL Runtime::isString()
{
    return (pStack_[stackTop_ - 1].type == GAIA_OP_STRING);
}

int Runtime::toInt()
{
    GaiaValue val = pop();
    return getValueAsInt(&val);
}

float Runtime::toNumber()
{
    GaiaValue val = pop();
    return getValueAsNumber(&val);
}

char *Runtime::toString()
{
    GaiaValue val = pop();
    getValueAsString(this->strT0_, &val);
    return strT0_;
}

void Runtime::pushInt(int val)
{
    GaiaValue gv;
    gv.type = GAIA_OP_INT;
    gv.intValue = val;
    push(gv);
}

void Runtime::pushNumber(float val)
{
    GaiaValue gv;
    gv.type = GAIA_OP_NUMBER;
    gv.numberValue = val;
    push(gv);
}

void Runtime::pushString(char *str)
{
    GaiaString *pStrObj = Gaia_NewString(str);
    pStrObj->ref = 0;
    GaiaValue gv;
    gv.type = GAIA_OP_STRING;
    gv.pObj = pStrObj;
    push(gv);
}
