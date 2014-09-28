#pragma once

#include "..\esip\ESInterpreter.h"
#include <vector>

class Console : public NativeObject
{
public:

	static Object* createObject(ESInterpreter *pInterpreter);

public:

	Console(ESInterpreter *pInterpreter);
	virtual ~Console();

};

