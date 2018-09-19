#pragma once
namespace Gaia
{
	class GaiaValue
	{
	public:
		int ref;
		int type;
		int size;
		int offset;
		union 
		{
			int intValue;
			float numberValue;
			GaiaValue *pObj;
		};
	};
}
