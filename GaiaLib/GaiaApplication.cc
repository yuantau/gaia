#include "gaia.h"
#include <stdio.h>

int csub(Gaia::Script *script)
{
    int b = script->toInt();
    int a = script->toInt();
    script->pushInt(a - b);
    return 1;
}

int main()
{
    struct A
    {
        int c;
    };
    A *a = new A;

    Gaia::Script script;
    script.pushLocalFunction("csub", csub);
    try
    {
        //script.execFile("test.gaia");
        script.execString("print('hello world.\\n')");
    }
    catch (int i)
    {
        char msg[ERROR_MSG_SIZE];
        script.formatErrorMessage(msg, i);
        printf("Error:%d\n", i);
        printf("Line:%d\nIndex:%d\n", error.line + 1, error.index);
        printf(msg);
    }
    return 0;
}