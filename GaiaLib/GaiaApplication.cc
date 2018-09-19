#include "gaia.h"
#include <stdio.h>

int csub(Gaia::Script * script)
{
	int b = script->toInt();
	int a = script->toInt();
	script->pushInt(a - b);
	return 1;
}

int main()
{

    Gaia::Script script;
	script.pushLocalFunction("csub", csub);

    try
    {
        script.execFile("D:/yuantao/Workspace/gaia/test/test.gaia");
		script.pushInt(2);
		script.pushInt(10);
		script.call("add");
		printf("%d", script.toInt());      
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