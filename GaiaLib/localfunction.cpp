#include "localfunction.h"
#include <ctype.h>

int Gaia::gaia_print(Script *gaia)
{
	
	printf(gaia->toString());
	return 0;
}

int Gaia::gaia_tostring(Script *gaia)
{
	gaia->pushString(gaia->toString());
	return 0;
}

int Gaia::gaia_toInt(Script *gaia)
{
	int r = gaia->toInt();
	gaia->pushInt(r);
	return 0;
}

int Gaia::gaia_toNumber(Script *gaia)
{
	float r = gaia->toNumber();
	gaia->pushNumber(r);
	return 0;
}

int Gaia::gaia_getTimer(Script *gaia)
{
	int r = (int)time(NULL);
	gaia->pushInt(r);
	return 1;
}

int Gaia::gaia_typeof(Script *gaia)
{
	char type[200];
	gaia->getType(type);
	gaia->pushString(type);
	return 1;
}

int Gaia::gaia_toLowerCase(Script *gaia)
{
	char *str = gaia->toString();

	char *start = str;
	char *end = start + strlen(str);
	while(start != end)
	{
		*start = tolower(*start);
		start++;
	}
	gaia->pushString(str);
	return 1;
}

int Gaia::gaia_toUpperCase(Script *gaia)
{
	char *str = gaia->toString();

	char *start = str;
	char *end = start + strlen(str);
	while(start != end)
	{
		*start = toupper(*start);
		start++;
	}
	gaia->pushString(str);
	return 1;
}
