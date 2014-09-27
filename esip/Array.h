#pragma once

#include "..\esip\ESInterpreter.h"
#include <vector>

class ArrayAdapter
{
public:

	ArrayAdapter();
	virtual ~ArrayAdapter();

	void operator()(ESInterpreter *pInterpreter, Object *pObject);

	static Value constructor(Object *pThis, std::vector<Value> &arguments, void *pUserParam);
	static Value join(Object *pThis, std::vector<Value> &arguments, void *pUserParam);
	static Value toString(Object *pThis, std::vector<Value> &arguments, void *pUserParam);

	static bool setVariable(const wchar_t *pName, const Value &value, void *pUserParam);
	static bool getVariable(const wchar_t *pName, Value &value, void *pUserParam);
	static void destroy(void *pUserParam);

};

