#include "stdafx.h"
#include "Date.h"

#include <time.h>
#include <iomanip>
#include <sstream>

class Date
{
private:

	::__time64_t m_time;

public:

	Date()
	{
		::_time64(&m_time);
	}

	virtual ~Date()
	{

	}

	std::wstring toString()
	{
		tm local;
		::_localtime64_s(&local, &m_time);
		std::wstringstream buffer;
		buffer
			<< std::setfill(L'0') << std::setw(4) << std::right << local.tm_year + 1900 << L"/"
			<< std::setfill(L'0') << std::setw(2) << std::right << local.tm_mon + 1 << L"/"
			<< std::setfill(L'0') << std::setw(2) << std::right << local.tm_mday << L" "
			<< std::setfill(L'0') << std::setw(2) << std::right << local.tm_hour << L":"
			<< std::setfill(L'0') << std::setw(2) << std::right << local.tm_min << L":"
			<< std::setfill(L'0') << std::setw(2) << std::right << local.tm_sec;
		return buffer.str();
	}

};

DateAdapter::DateAdapter()
{
}


DateAdapter::~DateAdapter()
{
}

void DateAdapter::operator()(ESInterpreter *pInterpreter, Object *pObject)
{
	Object *pDate = pInterpreter->createFunctionObject(DateAdapter::constructor, pInterpreter);
	pDate->m_class = L"Date";
	pObject->setVariable(L"Date", pDate);

	Object *pPrototype = pInterpreter->createObject();
	pDate->setVariable(L"prototype", pPrototype);

	pPrototype->setVariable(L"toString", pInterpreter->createFunctionObject(DateAdapter::toString, nullptr));
}

Value DateAdapter::constructor(Object *pThis, std::vector<Value> &arguments, void *pUserParam)
{
	ESInterpreter *pInterpreter = static_cast<ESInterpreter*>(pUserParam);

	Date *pDate = new Date();
	pThis->setCapture(nullptr, nullptr, DateAdapter::destroy, pDate);
	pThis->m_class = L"Date";

	return Value();
}

Value DateAdapter::toString(Object *pThis, std::vector<Value> &arguments, void *pUserParam)
{
	Date *pDate = static_cast<Date*>(pThis->m_pUserParam);

	return pDate->toString().c_str();
}

void DateAdapter::destroy(void *pUserParam)
{
	Date *pDate = static_cast<Date*>(pUserParam);
	delete pDate;
}
