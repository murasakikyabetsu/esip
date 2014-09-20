#pragma once

#include "..\esip\ESInterpreter.h"
#include <vector>

class ESIPImageAdapter
{
public:

	ESIPImageAdapter();
	virtual ~ESIPImageAdapter();

	void operator()(ESInterpreter *pInterpreter, Object *pObject);

	static Value constructor(Object *pThis, std::vector<Value> &arguments, void *pUserParam);
	static bool setVariable(const wchar_t *pName, const Value &value, void *pUserParam);
	static bool getVariable(const wchar_t *pName, Value &value, void *pUserParam);
	static void destroy(void *pUserParam);

	static Value load(Object *pThis, std::vector<Value> &arguments, void *pUserParam);
	static Value save(Object *pThis, std::vector<Value> &arguments, void *pUserParam);
	static Value getPixel(Object *pThis, std::vector<Value> &arguments, void *pUserParam);
	static Value setPixel(Object *pThis, std::vector<Value> &arguments, void *pUserParam);
};

