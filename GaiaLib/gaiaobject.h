#pragma once
#include "list.h"
#include "def.h"
#include "gaiavalue.h"

namespace Gaia
{
	struct KeyValue
	{
		int key;
		GaiaValue *pValue;
	};
	class GaiaObject:public GaiaValue
	{
	public:
		GaiaObject();
		~GaiaObject();
	public:
		BOOL setValue(char *name, GaiaValue *pObj);
		GaiaValue *find(char *name);
	private:
		List<KeyValue *> map_;
	};
}