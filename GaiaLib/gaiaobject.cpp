#include "gaiaobject.h"

using namespace Gaia;

GaiaObject::GaiaObject()
{

}

GaiaObject::~GaiaObject()
{

}

BOOL GaiaObject::setValue(char *name, GaiaValue *pObj)
{
	KeyValue *pKey = new KeyValue;
	pKey->key = hash(name);
	pKey->pValue = pObj;
	map_.push(pKey);
	return TRUE;
}

GaiaValue *GaiaObject::find(char *name)
{
	ListNode<KeyValue *> *pCurrent = map_.getHead();
	int v = hash(name);
	while(pCurrent != NULL)
	{
		if (pCurrent->getData()->key == v)
			return pCurrent->getData()->pValue;
		pCurrent = pCurrent->getNext();
	}
	return NULL;
}

