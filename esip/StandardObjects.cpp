#include "stdafx.h"
#include "StandardObjects.h"

#include <time.h>
#include <sstream>
#include <iomanip>

Object* Math::createObject(ESInterpreter *pInterpreter)
{
	::srand((unsigned int)::time(NULL));

	return pInterpreter->createNativeObject<Math>(L"Math",
	{
		{ L"random", &Math::random }
	}, {});
}

Value Math::random(ESInterpreter *pInterpreter, Object *pThis, std::vector<Value> arguments)
{
	return (double)::rand() / RAND_MAX;
}


/////////////////////////////////////////////////////////////////////


Object* Array::createObject(ESInterpreter *pInterpreter)
{
	return pInterpreter->createNativeObject<Array>(L"Array",
	{},
	{
		{ L"join", &Array::join },
		{ L"toString", &Array::toString }
	});
}

bool Array::setVariable(Object *pThis, const wchar_t *pName, const Value &value)
{
	if (::iswdigit(*pName))
	{
		int index = ::_wtol(pName);
		if (pThis->getVariable(L"length", true).toInt32() <= index)
			pThis->setVariable(L"length", (double)(index + 1));
	}

	return false;
}

Value Array::constructor(Object *pThis, std::vector<Value> arguments)
{
	if (arguments.size() == 1 && arguments[0].m_type == Value::VT_NUMBER)
	{
		pThis->setVariable(L"length", arguments[0]);
	}
	else if (0 < arguments.size())
	{
		for (size_t n = 0; n < arguments.size(); n++)
		{
			std::wstringstream buffer;
			buffer << n;
			pThis->setVariable(buffer.str().c_str(), std::move(arguments[n]));
		}
	}

	return Value();
}

Value Array::join(Object *pThis, std::vector<Value> arguments)
{
	std::wstring separator = L",";
	if (1 <= arguments.size())
		separator = arguments[0].toString();

	int length = pThis->getVariable(L"length", true).toInt32();

	std::wstringstream buffer;
	for (int n = 0; n < length; n++)
	{
		if (n)
			buffer << separator;

		std::wstringstream id;
		id << n;

		buffer << pThis->getVariable(id.str().c_str(), true).toString();
	}

	return buffer.str().c_str();
}

Value Array::toString(Object *pThis, std::vector<Value> arguments)
{
	return join(pThis, { Value(L",") });
}


/////////////////////////////////////////////////////////////////////



Object* Date::createObject(ESInterpreter *pInterpreter)
{
	return pInterpreter->createNativeObject<Date>(L"Date", {},
	{
		{ L"getTime", &Date::getTime },
		{ L"toString", &Date::toString }
	}, &Date::constructor, [](ESInterpreter *pInterpreter, Object *pThis, std::vector<Value> arguments)
	{
		auto now = std::chrono::system_clock::now();

		__time64_t time = std::chrono::system_clock::to_time_t(now);
		tm local;
		::_gmtime64_s(&local, &time);
		std::wstringstream buffer;
		buffer
			<< std::setfill(L'0') << std::setw(4) << std::right << local.tm_year + 1900 << L"/"
			<< std::setfill(L'0') << std::setw(2) << std::right << local.tm_mon + 1 << L"/"
			<< std::setfill(L'0') << std::setw(2) << std::right << local.tm_mday << L" "
			<< std::setfill(L'0') << std::setw(2) << std::right << local.tm_hour << L":"
			<< std::setfill(L'0') << std::setw(2) << std::right << local.tm_min << L":"
			<< std::setfill(L'0') << std::setw(2) << std::right << local.tm_sec;
		return Value(buffer.str().c_str());
	});
}

Value Date::constructor(Object *pThis, std::vector<Value> arguments)
{
	m_time = std::chrono::system_clock::now();
	return Value();
}

Value Date::getTime(Object *pThis, std::vector<Value> arguments)
{
	auto d = m_time.time_since_epoch();
	return (double)std::chrono::duration_cast<std::chrono::milliseconds>(d).count();
}

Value Date::toString(Object *pThis, std::vector<Value> arguments)
{
	__time64_t time = std::chrono::system_clock::to_time_t(m_time);
	tm local;
	::_localtime64_s(&local, &time);
	std::wstringstream buffer;
	buffer
		<< std::setfill(L'0') << std::setw(4) << std::right << local.tm_year + 1900 << L"/"
		<< std::setfill(L'0') << std::setw(2) << std::right << local.tm_mon + 1 << L"/"
		<< std::setfill(L'0') << std::setw(2) << std::right << local.tm_mday << L" "
		<< std::setfill(L'0') << std::setw(2) << std::right << local.tm_hour << L":"
		<< std::setfill(L'0') << std::setw(2) << std::right << local.tm_min << L":"
		<< std::setfill(L'0') << std::setw(2) << std::right << local.tm_sec;
	return buffer.str().c_str();
}
