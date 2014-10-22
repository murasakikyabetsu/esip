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

	ArrayBuffer(ESInterpreter *pInterpreter, ObjectPtr pThis);
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

	Uint8Array(ESInterpreter *pInterpreter, ObjectPtr pThis);
	virtual ~Uint8Array();

	Value constructor(std::vector<Value> arguments);

	bool setVariable(const wchar_t *pName, const Value &value);
	bool getVariable(const wchar_t *pName, Value &value);

};
