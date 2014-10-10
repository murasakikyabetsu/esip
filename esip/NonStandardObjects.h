#pragma once

#include "..\esip\ESInterpreter.h"
#include <vector>

class ArrayBuffer : public NativeObject
{
public:

	void *m_pData;
	size_t m_dataSize;

public:

	static Object* createObject(ESInterpreter *pInterpreter);

public:

	ArrayBuffer(ESInterpreter *pInterpreter);
	virtual ~ArrayBuffer();

	void setData(void *pData, size_t dataSize);

};

class Uint8Array : public NativeObject
{
private:

	ArrayBuffer *m_pArrayBuffer;

public:

	static Object* createObject(ESInterpreter *pInterpreter);

public:

	Uint8Array(ESInterpreter *pInterpreter);
	virtual ~Uint8Array();

	Value constructor(Object *pThis, std::vector<Value> arguments);

	bool setVariable(Object *pThis, const wchar_t *pName, const Value &value);
	bool getVariable(Object *pThis, const wchar_t *pName, Value &value);

};
