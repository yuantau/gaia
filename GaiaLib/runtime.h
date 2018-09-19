#pragma once
#include "def.h"
#include "stream.h"
#include "list.h"
#include "gaiavalue.h"
#include "gaiastring.h"
#include "gaiaobject.h"

namespace Gaia
{
class Instr
{
  public:
	Instr(void);
    int opCode;
    int opCount;
    GaiaValue opList[3];
};

class Runtime
{
  public:
    Runtime(void);
    ~Runtime(void);

  public:
    void begin(Stream *pCpode, Script *pScript);

    BOOL pushLocalFunction(char *name, pLocalFn fn);
    Function *findLocalFunction(char *name);

    BOOL call(char *name);
    BOOL getGlobal(char *name);
    BOOL isInt();
    BOOL IsNumber();
    BOOL isString();
    int toInt();
    float toNumber();
    char *toString();
    void pushInt(int val);
    void pushNumber(float val);
    void pushString(char *str);
    GaiaValue pop();
    void GC();

  private:
    void run();

    GaiaValue getOpValue(int index);
    GaiaValue *getOpValuePtr(int index);

    void getValueAsString(char *buf, GaiaValue *pVal);
    int getValueAsInt(GaiaValue *pVal);
    float getValueAsNumber(GaiaValue *pVal);

    void push(GaiaValue val)
    {
        pStack_[stackTop_] = val;
        stackTop_++;
        if (val.type == GAIA_OP_STRING)
        {
            ((GaiaString *)val.pObj)->ref++;
        }
    }

    GaiaString *Gaia_NewString(char *str)
    {
        GaiaString *strValue = new GaiaString(str);
        gaiaStringTable_.push(strValue);
        return strValue;
    }
    GaiaString *Gaia_NewString(int size)
    {
        GaiaString *strValue = new GaiaString(size);
        gaiaStringTable_.push(strValue);
        return strValue;
    }
    GaiaObject *Gaia_NewObject()
    {
        GaiaObject *pObj = new GaiaObject();
        gaiaObjectTable_.push(pObj);
        return pObj;
    }

    GaiaValue *Gaia_NewBasicValue(int val)
    {
        GaiaValue *p = new GaiaValue;
        p->intValue = val;
        p->type = GAIA_OP_INT;
        p->ref = 0;
        gaiaBasicValueTable_.push(p);
        return p;
    }

    GaiaValue *Gaia_NewBasicValue(float val)
    {
        GaiaValue *p = new GaiaValue;
        p->numberValue = val;
        p->type = GAIA_OP_NUMBER;
        p->ref = 0;
        gaiaBasicValueTable_.push(p);
        return p;
    }

    void addFrame(int size)
    {
        stackTop_ += size;
        stackLocal_ = stackTop_;
    }

    void delFrame(int size)
    {

        for (int i = 0; i < size; i++)
        {
            if (pStack_[stackTop_ - i - 1].type == GAIA_OP_STRING)
                ((GaiaString *)pStack_[stackTop_ - i - 1].pObj)->ref--;
        }
        stackTop_ -= size;
    }

    void setStack(int index, GaiaValue val)
    {
        pStack_[index < 0 ? stackLocal_ + index : index] = val;
    }

  private:
    GaiaValue *pStack_; //ջ�ռ�
    int stackTop_;
    int stackLocal_;

    int stackSize_;

    List<char *> varTable_;
    List<Function *> funcTable_;

    Instr *pInstrs_;
    int instrSize_;

    int ip_;

    List<GaiaString *> gaiaStringTable_;
    List<GaiaObject *> gaiaObjectTable_;
    List<GaiaValue *> gaiaBasicValueTable_;

    GaiaValue eax;
    GaiaValue ebx;

    char strT0_[1024];
    char strT1_[1024];

    List<Function *> localFuncTable_;
    Script *pScript_;
};

} // namespace Gaia
