#include "stdafx.h"
#include "ESIPObjects.h"


class Uint8Array
{
public:

	Object *m_pObject;

	unsigned char *m_pData;
	long m_size;

public:

	Uint8Array(Object *pObject, unsigned char *pData, long size) : m_pObject(pObject), m_pData(pData), m_size(size) {}
	virtual ~Uint8Array() {}

};


Uint8ArrayAdapter::Uint8ArrayAdapter()
{
}

Uint8ArrayAdapter::~Uint8ArrayAdapter()
{
}

void Uint8ArrayAdapter::operator()(Object *pObject)
{
	pObject->setNativeFunction(L"Uint8Array", Uint8ArrayAdapter::constructor, nullptr);
}

Value Uint8ArrayAdapter::constructor(Object *pThis, std::vector<Value> &arguments, void *pUserParam)
{
	Uint8Array *pArray = new Uint8Array(pThis, static_cast<unsigned char*>(arguments[0].toObject()->m_pUserParam), (long)arguments[0].toObject()->getVariable(L"length", true).toNumber());
	pThis->setCapture(Uint8ArrayAdapter::setVariable, Uint8ArrayAdapter::getVariable, Uint8ArrayAdapter::destroy, pArray);

	return Value();
}

bool Uint8ArrayAdapter::setVariable(const wchar_t *pName, const Value &value, void *pUserParam)
{
	Uint8Array *pArray = static_cast<Uint8Array*>(pUserParam);

	if (::iswdigit(*pName))
	{
		long index = ::_wtol(pName);
		pArray->m_pData[index] = (unsigned char)value.toNumber();
		return true;
	}

	if (::wcscmp(pName, L"length") == 0)
	{
		// read only
		return true;
	}

	return false;
}

bool Uint8ArrayAdapter::getVariable(const wchar_t *pName, Value &value, void *pUserParam)
{
	Uint8Array *pArray = static_cast<Uint8Array*>(pUserParam);

	if (::iswdigit(*pName))
	{
		long index = ::_wtol(pName);
		value = (double)pArray->m_pData[index];
		return true;
	}

	if (::wcscmp(pName, L"length") == 0)
	{
		value = (double)pArray->m_size;
		return true;
	}

	return false;
}

void Uint8ArrayAdapter::destroy(void *pUserParam)
{
	Uint8Array *pArray = static_cast<Uint8Array*>(pUserParam);
	delete pArray;
}
