#include "gaia.h"
#include <stdio.h>

int main()
{

    Gaia::Script script;

    try
    {
        script.execFile("D:/yuantao/Workspace/gaia/test/test.gaia");
		script.pushInt(2);
		script.pushInt(10);
		script.call("add");
		printf("%d", script.toInt());
        // script.printICode("d:\\2.txt");

       /*  script.getGlobal("name");
         printf("%s\n", script.toString());
		 script.getGlobal("tick");
		 printf("%d\n", script.toInt());*/
      
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