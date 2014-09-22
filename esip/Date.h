#pragma once

#include "..\esip\ESInterpreter.h"
#include <vector>

class DateAdapter
{
public:
	DateAdapter();
	virtual ~DateAdapter();

	void operator()(ESInterpreter *pInterpreter, Object *pObject);

	static Value constructor(Object *pThis, std::vector<Value> &arguments, void *pUserParam);
	static Value toString(Object *pThis, std::vector<Value> &arguments, void *pUserParam);

	static void destroy(void *pUserParam);

};

