#pragma once
#include "def.h"
#include "gaiavalue.h"
namespace Gaia
{
	class GaiaString:public GaiaValue
	{
	public:
		GaiaString(GaiaString &o)
		{
			strValue = new char[o.size+1];
			size = o.size;
			strcpy(strValue, o.strValue);
			type = GAIA_OP_STRING;
			ref = 0;
			hashVal = hash(strValue);
		}
		GaiaString(char *str)
		{
			size = strlen(str);
			strValue = new char[size+1];
			strcpy(strValue, str);
			type = GAIA_OP_STRING;
			ref = 0;
			hashVal = hash(strValue);
		}
		GaiaString(int length)
		{
			size = length;
			strValue = new char[size+1];
			type = GAIA_OP_STRING;
			ref = 0;
			hashVal = 0;
		}
		GaiaString()
		{
			strValue = NULL;
			size = 0;
			type = GAIA_OP_STRING;
			ref = 0;
			hashVal = 0;
		}
		~GaiaString()
		{
			if (strValue != NULL)
				delete [] strValue;
		}
		void setData(char *source)
		{
			if ((int)strlen(source) >= size) return;
			strcpy(strValue, source);
			hashVal = hash(strValue);
		}
	public:
		char *strValue;
		int hashVal;
	};
}
