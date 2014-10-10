#include "stdafx.h"
#include "NonStandardObjects.h"


ArrayBuffer::ArrayBuffer(ESInterpreter *pInterpreter) : NativeObject(pInterpreter), m_pData(nullptr), m_dataSize(0)
{

}

ArrayBuffer::~ArrayBuffer()
{

}

Object* ArrayBuffer::createObject(ESInterpreter *pInterpreter)
{
	return pInterpreter->createNativeObject<ArrayBuffer>(L"ArrayBuffer", {}, {});
}

void ArrayBuffer::setData(void *pData, size_t dataSize)
{
	m_pData = pData;
	m_dataSize = dataSize;
}

//////////////////////////////////


Uint8Array::Uint8Array(ESInterpreter *pInterpreter) : NativeObject(pInterpreter), m_pArrayBuffer(nullptr)
{
}

Uint8Array::~Uint8Array()
{
}

Object* Uint8Array::createObject(ESInterpreter *pInterpreter)
{
	return pInterpreter->createNativeObject<Uint8Array>(L"Uint8Array", {}, {});
}

Value Uint8Array::constructor(Object *pThis, std::vector<Value> arguments)
{
	NativeObject::constructor(pThis, arguments);

	ObjectPtr pObject = arguments[0].toObject();
	if (pObject->m_class != L"ArrayBuffer" || !pObject->m_pUserParam)
		throw ESException(ESException::R_TYPEERROR);

	m_pArrayBuffer = static_cast<ArrayBuffer*>(pObject->m_pUserParam);

	return Value();
}

bool Uint8Array::setVariable(Object *pThis, const wchar_t *pName, const Value &value)
{
	if (::iswdigit(*pName))
	{
		long index = ::_wtol(pName);

		if ((long)m_pArrayBuffer->m_dataSize <= index)
			throw ESException(ESException::R_REFERENCEERROR);	// todo

		unsigned char *pData = (unsigned char*)m_pArrayBuffer->m_pData;
		pData[index] = (unsigned char)value.toNumber();
		return true;
	}

	if (::wcscmp(pName, L"length") == 0)
	{
		// read only
		return true;
	}

	return false;
}

bool Uint8Array::getVariable(Object *pThis, const wchar_t *pName, Value &value)
{
	if (::iswdigit(*pName))
	{
		long index = ::_wtol(pName);

		if ((long)m_pArrayBuffer->m_dataSize <= index)
			throw ESException(ESException::R_REFERENCEERROR);	// todo

		unsigned char *pData = (unsigned char*)m_pArrayBuffer->m_pData;
		value = (double)pData[index];
		return true;
	}

	if (::wcscmp(pName, L"length") == 0)
	{
		value = (double)m_pArrayBuffer->m_dataSize;
		return true;
	}

	return false;
}
