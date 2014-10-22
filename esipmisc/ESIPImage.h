#pragma once

#include "..\esip\ESInterpreter.h"
#include <vector>

class ESIPImage : public NativeObject
{
private:

#pragma pack(2)

	typedef struct tagBITMAPFILEHEADER
	{
		unsigned short bfType;
		unsigned long  bfSize;
		unsigned short bfReserved1;
		unsigned short bfReserved2;
		unsigned long  bfOffBits;
	} BITMAPFILEHEADER;

	typedef struct tagBITMAPINFOHEADER
	{
		unsigned long  biSize;
		long           biWidth;
		long           biHeight;
		unsigned short biPlanes;
		unsigned short biBitCount;
		unsigned long  biCompression;
		unsigned long  biSizeImage;
		long           biXPixPerMeter;
		long           biYPixPerMeter;
		unsigned long  biClrUsed;
		unsigned long  biClrImporant;
	} BITMAPINFOHEADER;

#pragma pack()

public:

	long m_width;
	long m_height;

	std::vector<unsigned char> m_data;

public:

	static Object* createObject(ESInterpreter *pInterpreter);

public:

	ESIPImage(ESInterpreter *pInterpreter, ObjectPtr pThis);
	virtual ~ESIPImage();

	Value constructor(std::vector<Value> arguments);
	bool setVariable(const wchar_t *pName, const Value &value);
	bool getVariable(const wchar_t *pName, Value &value);

	Value load(std::vector<Value> arguments);
	Value save(std::vector<Value> arguments);
	Value getPixel(std::vector<Value> arguments);
	Value setPixel(std::vector<Value> arguments);

	long getWidth();
	long getHeight();
};

