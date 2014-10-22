#include "stdafx.h"
#include "ESIPImage.h"

#include "..\esip\NonStandardObjects.h"

ESIPImage::ESIPImage(ESInterpreter *pInterpreter, ObjectPtr pThis) : NativeObject(pInterpreter, pThis)
{
}


ESIPImage::~ESIPImage()
{
}

Object* ESIPImage::createObject(ESInterpreter *pInterpreter)
{
	return pInterpreter->createNativeObject<ESIPImage>(L"ESIPImage",
	{},
	{
		{ L"load", &ESIPImage::load },
		{ L"save", &ESIPImage::save },
		{ L"getPixel", &ESIPImage::getPixel },
		{ L"setPixel", &ESIPImage::setPixel }
	});
}

Value ESIPImage::constructor(std::vector<Value> arguments)
{
	if (1 <= arguments.size() && arguments[0].m_type == Value::VT_STRING)
		load(arguments);
	else if (2 <= arguments.size() && arguments[0].m_type == Value::VT_NUMBER && arguments[1].m_type == Value::VT_NUMBER)
	{
		m_width = arguments[0].toInt32();
		m_height = arguments[1].toInt32();
		m_data.resize(m_height * m_width * 3);
	}

	return Value();
}

bool ESIPImage::setVariable(const wchar_t *pName, const Value &value)
{
	if (::wcscmp(pName, L"width") == 0 ||
		::wcscmp(pName, L"height") == 0 ||
		::wcscmp(pName, L"buffer") == 0)
	{
		// read only
		return true;
	}
	
	return false;
}

bool ESIPImage::getVariable(const wchar_t *pName, Value &value)
{
	if (::wcscmp(pName, L"width") == 0)
	{
		value = (double)m_width;
		return true;
	}
	
	if (::wcscmp(pName, L"height") == 0)
	{
		value = (double)m_height;
		return true;
	}

	if (::wcscmp(pName, L"buffer") == 0)
	{
		ObjectPtr pArrayBuffer = m_pInterpreter->getGlobalObject()->getVariable(L"ArrayBuffer", true).toObject();
		if (!pArrayBuffer)
			throw ESException(ESException::R_REFERENCEERROR, L"ArrayBuffer");
		value = pArrayBuffer->construct({});
		ArrayBuffer *p = static_cast<ArrayBuffer*>(value.toObject()->m_pUserParam);
		p->setData(&m_data[0], m_data.size());

		return true;
	}

	return false;
}

Value ESIPImage::load(std::vector<Value> arguments)
{
	if (1 <= arguments.size())
	{
		FILE *f;
		if (::_wfopen_s(&f, arguments[0].toString().c_str(), L"rb") == 0)
		{
			::fseek(f, 0, SEEK_END);
			size_t size = ::ftell(f);
			::fseek(f, 0, SEEK_SET);

			std::vector<unsigned char> buffer(size);
			::fread(&buffer[0], 1, size, f);

			::fclose(f);

			BITMAPFILEHEADER *pBFH = (BITMAPFILEHEADER*)&buffer[0];
			BITMAPINFOHEADER *pBIH = (BITMAPINFOHEADER*)&buffer[sizeof(BITMAPFILEHEADER)];
			m_width = pBIH->biWidth;
			m_height = pBIH->biHeight;

			long widthBytes = (m_width * 3 + 3) & (~3);

			if (0 < m_height)
			{
				m_data.resize(m_height * m_width * 3);

				for (long y = 0; y < m_height; y++)
				{
					::memcpy(&m_data[(m_height - y - 1) * m_width * 3], &buffer[pBFH->bfOffBits + y * widthBytes], m_width * 3);
				}
			}
			else
			{
				m_height = -m_height;
				m_data.resize(m_height * m_width * 3);

				for (long y = 0; y < m_height; y++)
				{
					::memcpy(&m_data[y * m_width * 3], &buffer[pBFH->bfOffBits + y * widthBytes], m_width * 3);
				}
			}
		}
	}

	return Value();
}

Value ESIPImage::save(std::vector<Value> arguments)
{
	if (1 <= arguments.size())
	{
		long widthBytes = (m_width * 3 + 3) & (~3);

		BITMAPFILEHEADER bfh = {0};
		bfh.bfType = 'MB';
		bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + m_height * widthBytes;
		bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		BITMAPINFOHEADER bih = {0};
		bih.biSize = sizeof(bih);
		bih.biWidth = m_width;
		bih.biHeight = m_height;
		bih.biPlanes = 1;
		bih.biBitCount = 24;
		bih.biSizeImage = m_height * widthBytes;

		FILE *f;
		if (::_wfopen_s(&f, arguments[0].toString().c_str(), L"wb") == 0)
		{
			::fwrite(&bfh, 1, sizeof(bfh), f);
			::fwrite(&bih, 1, sizeof(bih), f);
			for (long y = m_height - 1; 0 <= y; y--)
			{
				::fwrite(&m_data[y * m_width * 3], 1, m_width * 3, f);
				char dummy[4] = { 0 };
				::fwrite(dummy, 1, widthBytes - m_width * 3, f);
			}
			::fclose(f);
		}
	}

	return Value();
}

Value ESIPImage::getPixel(std::vector<Value> arguments)
{
	if (2 <= arguments.size())
	{
		int x = arguments[0].toInt32();
		int y = arguments[1].toInt32();
		unsigned char *pPos = &m_data[(y * m_width + x) * 3];

		ObjectPtr pObject = m_pInterpreter->createObject();
		pObject->setVariable(L"r", (double)pPos[2]);
		pObject->setVariable(L"g", (double)pPos[1]);
		pObject->setVariable(L"b", (double)pPos[0]);

		return (Object*)pObject;
	}

	return Value();
}

Value ESIPImage::setPixel(std::vector<Value> arguments)
{
	if (5 <= arguments.size())
	{
		int x = arguments[0].toInt32();
		int y = arguments[1].toInt32();
		unsigned char *pPos = &m_data[(y * m_width + x) * 3];

		int r = arguments[2].toInt32();
		int g = arguments[3].toInt32();
		int b = arguments[4].toInt32();

		pPos[0] = b < 0 ? 0 : 255 < b ? 255 : (unsigned char)b;
		pPos[1] = g < 0 ? 0 : 255 < g ? 255 : (unsigned char)g;
		pPos[2] = r < 0 ? 0 : 255 < r ? 255 : (unsigned char)r;
	}

	return Value();
}

long ESIPImage::getWidth()
{
	return m_width;
}

long ESIPImage::getHeight()
{
	return m_height;
}
