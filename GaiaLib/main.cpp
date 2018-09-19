#include <stdio.h>
#include <windows.h>
#include "global.h"
#include "script.h"

using namespace Gaia;



int main(int argc, char *argv[])
{

	Gaia::Script script;	


		try 
		{
			script.execFile("test.txt");
			//script.printICode("d:\\2.txt");
			
			//script.getGlobal("name");
			//printf("%s\n", script.toString());
			//script.GC();
			
		}
		catch(int i)
		{
			printf("Error:%d\n", i);
			printf("Line:%d\nIndex:%d\n", error.line+1, error.index);
			printf(error.msg);
		}
		getchar();
	return 0;
}

