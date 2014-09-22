#pragma once

#include "..\esip\ESInterpreter.h"
#include <vector>

class ConsoleAdapter
{
public:

	ConsoleAdapter();
	virtual ~ConsoleAdapter();

	void operator()(ESInterpreter *pInterpreter, Object *pObject);

};

