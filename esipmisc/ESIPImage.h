#pragma once

#include "..\esip\ESInterpreter.h"
#include <vector>

class ESIPImage : public NativeObject
{
public:

	long m_width;
	long m_height;

	std::vector<unsigned char> m_data;

public:

	static Object* createObject(ESInterpreter *pInterpreter);

public:

	ESIPImage(ESInterpreter *pInterpreter);
	virtual ~ESIPImage();

	Value constructor(Object *pThis, std::vector<Value> arguments);
	bool setVariable(Object *pThis, const wchar_t *pName, const Value &value);
	bool getVariable(Object *pThis, const wchar_t *pName, Value &value);

	Value load(Object *pThis, std::vector<Value> arguments);
	Value save(Object *pThis, std::vector<Value> arguments);
	Value getPixel(Object *pThis, std::vector<Value> arguments);
	Value setPixel(Object *pThis, std::vector<Value> arguments);
};

