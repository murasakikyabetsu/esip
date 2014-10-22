#pragma once

#include "..\esip\ESInterpreter.h"
#include <vector>
#include <chrono>

class Math : public NativeObject
{
public:

	static Object* createObject(ESInterpreter *pInterpreter);

	static Value random(ESInterpreter *pInterpreter, Object *pThis, std::vector<Value> arguments);

public:

	Math(ESInterpreter *pInterpreter, ObjectPtr pThis) : NativeObject(pInterpreter, pThis) {}
	virtual ~Math() {}

};

class Array : public NativeObject
{
public:

	static Object* createObject(ESInterpreter *pInterpreter);

public:

	Array(ESInterpreter *pInterpreter, ObjectPtr pThis) : NativeObject(pInterpreter, pThis) {}
	virtual ~Array() {}

	virtual bool setVariable(const wchar_t *pName, const Value &value);

	Value constructor(std::vector<Value> arguments);
	Value join(std::vector<Value> arguments);
	Value toString(std::vector<Value> arguments);

};

class Date : public NativeObject
{
private:

	std::chrono::system_clock::time_point m_time;

public:

	static Object* createObject(ESInterpreter *pInterpreter);

public:

	Date(ESInterpreter *pInterpreter, ObjectPtr pThis) : NativeObject(pInterpreter, pThis) {}
	virtual ~Date() {}

	Value constructor(std::vector<Value> arguments);
	Value getTime(std::vector<Value> arguments);
	Value toString(std::vector<Value> arguments);
};

class String : public NativeObject
{
private:

	std::wstring m_str;

public:

	static Object* createObject(ESInterpreter *pInterpreter);

public:

	String(ESInterpreter *pInterpreter, ObjectPtr pThis) : NativeObject(pInterpreter, pThis) {}
	virtual ~String() {}

	Value constructor(std::vector<Value> arguments);
	Value lastIndexOf(std::vector<Value> arguments);
	Value substring(std::vector<Value> arguments);
	Value toString(std::vector<Value> arguments);
};