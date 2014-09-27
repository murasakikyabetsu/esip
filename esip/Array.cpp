#include "stdafx.h"
#include "Array.h"

#include <sstream>

class Array
{
public:

	Object *m_pObject;
	std::vector<Value> m_values;

public:

	Array(Object *pObject) : m_pObject(pObject)
	{

	}

	virtual ~Array()
	{

	}

	Value join(const wchar_t *pSeparator)
	{
		std::wstringstream buffer;
		for (size_t n = 0; n < m_values.size(); n++)
		{
			if (n)
				buffer << pSeparator;

			buffer << m_values[n].toString();
		}
		return buffer.str().c_str();
	}
};



ArrayAdapter::ArrayAdapter()
{
}


ArrayAdapter::~ArrayAdapter()
{
}

void ArrayAdapter::operator()(ESInterpreter *pInterpreter, Object *pObject)
{
	Object *pArray = pInterpreter->createFunctionObject(ArrayAdapter::constructor, pInterpreter);
	pArray->m_class = L"Array";
	pObject->setVariable(L"Array", pArray);

	Object *pPrototype = pInterpreter->createObject();
	pArray->setVariable(L"prototype", pPrototype);

	pPrototype->setVariable(L"join", pInterpreter->createFunctionObject(ArrayAdapter::join, nullptr));
	pPrototype->setVariable(L"toString", pInterpreter->createFunctionObject(ArrayAdapter::toString, nullptr));
}

Value ArrayAdapter::constructor(Object *pThis, std::vector<Value> &arguments, void *pUserParam)
{
	ESInterpreter *pInterpreter = static_cast<ESInterpreter*>(pUserParam);

	Array *pArray = new Array(pThis);
	pThis->setCapture(ArrayAdapter::setVariable, ArrayAdapter::getVariable, ArrayAdapter::destroy, pArray);
	pThis->m_class = L"Array";

	if (arguments.size() == 1 && arguments[0].m_type == Value::VT_NUMBER)
	{
		pArray->m_values.resize((int)arguments[0].toNumber());
	}
	else if (0 < arguments.size())
	{
		for (size_t n = 0; n < arguments.size(); n++)
		{
			pArray->m_values.push_back(std::move(arguments[n]));
		}
	}
	pThis->setVariable(L"length", (double)pArray->m_values.size());

	return Value();
}

Value ArrayAdapter::join(Object *pThis, std::vector<Value> &arguments, void *pUserParam)
{
	Array *pArray = static_cast<Array*>(pThis->m_pUserParam);

	if (1 <= arguments.size())
		return pArray->join(arguments[0].toString().c_str());
	else
		return pArray->join(L",");
}

Value ArrayAdapter::toString(Object *pThis, std::vector<Value> &arguments, void *pUserParam)
{
	Array *pArray = static_cast<Array*>(pThis->m_pUserParam);

	return pArray->join(L",");
}

bool ArrayAdapter::setVariable(const wchar_t *pName, const Value &value, void *pUserParam)
{
	Array *pArray = static_cast<Array*>(pUserParam);

	if (::iswdigit(*pName))
	{
		int index = ::_wtol(pName);
		if ((int)pArray->m_values.size() <= index)
			pArray->m_values.resize(index + 1);
		pArray->m_values[index] = value;
		return true;
	}

	if (::wcscmp(pName, L"length") == 0)
	{
		pArray->m_values.resize((int)value.toNumber());
		return true;
	}

	return false;
}

bool ArrayAdapter::getVariable(const wchar_t *pName, Value &value, void *pUserParam)
{
	Array *pArray = static_cast<Array*>(pUserParam);

	if (::iswdigit(*pName))
	{
		int index = ::_wtol(pName);
		if (index < 0 || (int)pArray->m_values.size() <= index)
			return false;
		value = pArray->m_values[index];
		value.m_pBase = pArray->m_pObject;
		value.m_referenceName = pName;
		return true;
	}

	if (::wcscmp(pName, L"length") == 0)
	{
		value = (double)pArray->m_values.size();
		return true;
	}

	return false;
}

void ArrayAdapter::destroy(void *pUserParam)
{
	Array *pArray = static_cast<Array*>(pUserParam);
	delete pArray;
}
