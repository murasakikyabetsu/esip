#include "stdafx.h"
#include "ESIPImage.h"

#include "..\esip\NonStandardObjects.h"

ESIPImage::ESIPImage(ESInterpreter *pInterpreter) : NativeObject(pInterpreter)
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

Value ESIPImage::constructor(Object *pThis, std::vector<Value> arguments)
{
	if (1 <= arguments.size())
		load(pThis, arguments);

	return Value();
}

bool ESIPImage::setVariable(Object *pThis, const wchar_t *pName, const Value &value)
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

bool ESIPImage::getVariable(Object *pThis, const wchar_t *pName, Value &value)
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
		Object *pArrayBuffer = m_pInterpreter->getGlobalObject()->getVariable(L"ArrayBuffer", true).toObject();
		if (!pArrayBuffer)
			throw ESException(ESException::R_REFERENCEERROR, L"ArrayBuffer");
		value = pArrayBuffer->construct({});
		ArrayBuffer *p = static_cast<ArrayBuffer*>(value.toObject()->m_pUserParam);
		p->m_pData = &m_data[0];
		p->m_dataSize = m_data.size();

		return true;
	}

	return false;
}

Value ESIPImage::load(Object *pThis, std::vector<Value> arguments)
{
	if (1 <= arguments.size())
	{
		// todo

		FILE *f;
		if (::_wfopen_s(&f, arguments[0].toString().c_str(), L"rb") == 0)
		{
			::fseek(f, 0, SEEK_END);
			size_t size = ::ftell(f);
			::fseek(f, 0, SEEK_SET);

			std::vector<unsigned char> buffer(size);
			::fread(&buffer[0], 1, size, f);

			::fclose(f);

			m_width = *(long*)&buffer[14 + 4];
			m_height = *(long*)&buffer[14 + 8];

			if (0 < m_height)
			{
				m_data.resize(m_height * m_width * 3);

				for (long y = 0; y < m_height; y++)
				{
					::memcpy(&m_data[(m_height - y - 1) * m_width * 3], &buffer[*(long*)&buffer[10] + y * ((m_width * 3 + 3) & (~3))], m_width * 3);
				}
			}
			else
			{
				m_height = -m_height;
				m_data.resize(m_height * m_width * 3);

				for (long y = 0; y < m_height; y++)
				{
					::memcpy(&m_data[y * m_width * 3], &buffer[*(long*)&buffer[10] + y * ((m_width * 3 + 3) & (~3))], m_width * 3);
				}
			}
		}
	}

	return Value();
}

Value ESIPImage::save(Object *pThis, std::vector<Value> arguments)
{
	ESIPImage *pImage = static_cast<ESIPImage*>(pThis->m_pUserParam);

	if (1 <= arguments.size())
	{
		// todo

		FILE *f;
		if (::_wfopen_s(&f, arguments[0].toString().c_str(), L"wb") == 0)
		{
			::fwrite(&m_data[0], 1, m_data.size(), f);
			::fclose(f);
		}
	}

	return Value();
}

Value ESIPImage::getPixel(Object *pThis, std::vector<Value> arguments)
{
	ESIPImage *pImage = static_cast<ESIPImage*>(pThis->m_pUserParam);

	if (2 <= arguments.size())
	{
		int x = arguments[0].toInt32();
		int y = arguments[1].toInt32();
		unsigned char *pPos = &m_data[(y * m_width + x) * 3];

		Object *pObject = pImage->m_pInterpreter->createObject();
		pObject->setVariable(L"r", (double)pPos[2]);
		pObject->setVariable(L"g", (double)pPos[1]);
		pObject->setVariable(L"b", (double)pPos[0]);

		return pObject;
	}

	return Value();
}

Value ESIPImage::setPixel(Object *pThis, std::vector<Value> arguments)
{
	ESIPImage *pImage = static_cast<ESIPImage*>(pThis->m_pUserParam);

	if (5 <= arguments.size())
	{
		int x = arguments[0].toInt32();
		int y = arguments[1].toInt32();
		unsigned char *pPos = &m_data[(y * m_width + x) * 3];

		pPos[0] = (unsigned char)arguments[4].toNumber();
		pPos[1] = (unsigned char)arguments[3].toNumber();
		pPos[2] = (unsigned char)arguments[2].toNumber();
	}

	return Value();
}
