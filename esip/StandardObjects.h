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

	Math(ESInterpreter *pInterpreter) : NativeObject(pInterpreter) {}
	virtual ~Math() {}

};

class Array : public NativeObject
{
public:

	static Object* createObject(ESInterpreter *pInterpreter);

public:

	Array(ESInterpreter *pInterpreter) : NativeObject(pInterpreter) {}
	virtual ~Array() {}

	virtual bool setVariable(Object *pThis, const wchar_t *pName, const Value &value);

	Value constructor(Object *pThis, std::vector<Value> arguments);
	Value join(Object *pThis, std::vector<Value> arguments);
	Value toString(Object *pThis, std::vector<Value> arguments);

};

class Date : public NativeObject
{
private:

	std::chrono::system_clock::time_point m_time;

public:

	static Object* createObject(ESInterpreter *pInterpreter);

public:

	Date(ESInterpreter *pInterpreter) : NativeObject(pInterpreter) {}
	virtual ~Date() {}

	Value constructor(Object *pThis, std::vector<Value> arguments);
	Value getTime(Object *pThis, std::vector<Value> arguments);
	Value toString(Object *pThis, std::vector<Value> arguments);
};
