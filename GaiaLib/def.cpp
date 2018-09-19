#include "def.h"

char * Gaia::getMnemonics(int type)
{
	static char ppstrMnemonics [][ 12 ] =
	{
		"Mov",
		"Add", "Sub", "Mul", "Div", "Mod", "Exp", "Neg", "Inc", "Dec",
		"And", "Or", "XOr", "Not", "ShL", "ShR",
		"Concat", "GetChar", "SetChar",
		"Jmp", "JE", "JNE", "JG", "JL", "JGE", "JLE",
		"Push", "Pop",
		"Call", "Ret",
		"Pause", "Exit",
		"New_Obj", "Save_Field", "Set_Field", "Get_Field", "New_Array", "S_CALL",
		"Add_Field", "Sub_Field", "Mul_Field", "Div_Field", "Mod_Field"
	};
	return ppstrMnemonics[type];
}


char *Gaia::getValueType(int p)
{
	static char types[200];
	switch(p)
	{
	case GAIA_OP_INT:
		sprintf(types, "Int");
		break;
	case GAIA_OP_NUMBER:
		sprintf(types, "Number");
		break;
	case GAIA_OP_STRING:
		sprintf(types, "String");
		break;
	case GAIA_OP_OBJECT:
		sprintf(types, "Object");
		break;
	case GAIA_OP_ARRAY:
		sprintf(types, "Array");
		break;
	case GAIA_OP_NULL:
		sprintf(types, "Null");
		break;
	default:
		sprintf(types, "Unknow");
	}
	return types;
}

int Gaia::hash(char *source)
{
	int size = strlen(source);
	int len = size;
	unsigned char *p;
	p = (unsigned char *)source;
	int x;
	x = *p << 7;
	while(--len >= 0)
		x = 1000003 * x ^ *p++;
	x ^= size;
	if (x == -1) x = -2;
	return x;
}
