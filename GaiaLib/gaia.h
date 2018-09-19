/************************************************************************/
/* Gaia Script														    */
/* load source code, gen GASM                                           */
/* start runtime                                                        */
/* 5.28.2013 by Yuan                                                    */
/************************************************************************/
#pragma once
#include "def.h"
#include "lexer.h"
#include "parse.h"
#include "runtime.h"
#include "localfunction.h"

namespace Gaia
{
class Script
{
  public:
    Script(void);
    ~Script(void);

  public:
    void execFile(const char *filename);

    void execString(const char *str);

    void compile(char *pSource);

    void printICode(const char *filename);

    void GC();

    void formatErrorMessage(char *szMsg, int code);

    BOOL call(char *name);
    BOOL getGlobal(char *name);
    BOOL isInt();
    BOOL IsNumber();
    BOOL isString();
    int toInt();
    float toNumber();
    char *toString();
    void getType(char *buf);
    void pushInt(int val);
    void pushNumber(float val);
    void pushString(char *str);
	BOOL pushLocalFunction(char *name, pLocalFn fn);
    GaiaValue popValue();

  private:
    Lexer lexer_;
    Parse parse_;
    Runtime runtime_;
};
} // namespace Gaia
