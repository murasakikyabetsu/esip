#include "stdafx.h"
#include "ESIPImage.h"

class ESIPImage
{
public:

	ESInterpreter *m_pInterpreter;
	Object *m_pObject;

	long m_width;
	long m_height;

private:

	std::vector<unsigned char> m_data;

public:

	ESIPImage(ESInterpreter *pInterpreter, Object *pObject) : m_pInterpreter(pInterpreter), m_pObject(pObject), m_width(0), m_height(0)
	{

	}

	virtual ~ESIPImage()
	{

	}

	void load(const wchar_t *pFilename)
	{
		// todo Ç‹Ç∂ÇﬂÇ…èëÇ≠

		FILE *f;
		if (::_wfopen_s(&f, pFilename, L"rb") == 0)
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

	void save(const wchar_t *pFilename)
	{
		// todo Ç‹Ç∂ÇﬂÇ…èëÇ≠

		FILE *f;
		if (::_wfopen_s(&f, pFilename, L"wb") == 0)
		{
			::fwrite(&m_data[0], 1, m_data.size(), f);
			::fclose(f);
		}
	}

	void setPixel(long x, long y, unsigned char r, unsigned char g, unsigned char b)
	{
		unsigned char *pPos = &m_data[(y * m_width + x) * 3];

		pPos[0] = b;
		pPos[1] = g;
		pPos[2] = r;
	}

	void getPixel(long x, long y, unsigned char &r, unsigned char &g, unsigned char &b)
	{
		unsigned char *pPos = &m_data[(y * m_width + x) * 3];

		b = pPos[0];
		g = pPos[1];
		r = pPos[2];
	}

	unsigned char* get_buffer()
	{
		return &m_data[0];
	}
};

///////////////////////////////////////////////////////

ESIPImageAdapter::ESIPImageAdapter()
{
}


ESIPImageAdapter::~ESIPImageAdapter()
{
}

void ESIPImageAdapter::operator()(ESInterpreter *pInterpreter, Object *pObject)
{
	Object *pESIP = nullptr;
	Value value = pObject->getVariable(L"ESIP", true);
	if (value.m_type == Value::VT_OBJECT)
	{
		pESIP = value.toObject();
	}
	else
	{
		pESIP = pInterpreter->createObject();
		pObject->setVariable(L"ESIP", pESIP);
	}

	Object *pImage = pInterpreter->createFunctionObject(ESIPImageAdapter::constructor, pInterpreter);
	pESIP->setVariable(L"Image", pImage);

	Object *pPrototype = pInterpreter->createObject();
	pImage->setVariable(L"prototype", pPrototype);

	pPrototype->setVariable(L"load", pInterpreter->createFunctionObject(ESIPImageAdapter::load, nullptr));
	pPrototype->setVariable(L"save", pInterpreter->createFunctionObject(ESIPImageAdapter::save, nullptr));
	pPrototype->setVariable(L"getPixel", pInterpreter->createFunctionObject(ESIPImageAdapter::getPixel, nullptr));
	pPrototype->setVariable(L"setPixel", pInterpreter->createFunctionObject(ESIPImageAdapter::setPixel, nullptr));
}

Value ESIPImageAdapter::constructor(Object *pThis, std::vector<Value> &arguments, void *pUserParam)
{
	ESInterpreter *pInterpreter = (ESInterpreter*)pUserParam;

	ESIPImage *pImage = new ESIPImage(pInterpreter, pThis);
	pThis->setCapture(ESIPImageAdapter::setVariable, ESIPImageAdapter::getVariable, ESIPImageAdapter::destroy, pImage);

	if (1 <= arguments.size())
		pImage->load(arguments[0].toString().c_str());

	return Value();
}

bool ESIPImageAdapter::setVariable(const wchar_t *pName, const Value &value, void *pUserParam)
{
	ESIPImage *pImage = static_cast<ESIPImage*>(pUserParam);

	if (::wcscmp(pName, L"width") == 0 ||
		::wcscmp(pName, L"height") == 0 ||
		::wcscmp(pName, L"buffer") == 0)
	{
		// read only
		return true;
	}
	
	return false;
}

bool ESIPImageAdapter::getVariable(const wchar_t *pName, Value &value, void *pUserParam)
{
	ESIPImage *pImage = static_cast<ESIPImage*>(pUserParam);

	if (::wcscmp(pName, L"width") == 0)
	{
		value = (double)pImage->m_width;
		return true;
	}
	
	if (::wcscmp(pName, L"height") == 0)
	{
		value = (double)pImage->m_height;
		return true;
	}

	if (::wcscmp(pName, L"buffer") == 0)
	{
		Object *pObject = pImage->m_pInterpreter->createObject();
		pObject->m_pUserParam = pImage->get_buffer();
		pObject->setVariable(L"length", (double)pImage->m_height * pImage->m_width * 3);
		value = pObject;
		return true;
	}

	return false;
}

void ESIPImageAdapter::destroy(void *pUserParam)
{
	ESIPImage *pImage = static_cast<ESIPImage*>(pUserParam);
	delete pImage;
}

Value ESIPImageAdapter::load(Object *pThis, std::vector<Value> &arguments, void *pUserParam)
{
	ESIPImage *pImage = static_cast<ESIPImage*>(pThis->m_pUserParam);

	if (1 <= arguments.size())
		pImage->load(arguments[0].toString().c_str());

	return Value();
}

Value ESIPImageAdapter::save(Object *pThis, std::vector<Value> &arguments, void *pUserParam)
{
	ESIPImage *pImage = static_cast<ESIPImage*>(pThis->m_pUserParam);

	if (1 <= arguments.size())
		pImage->save(arguments[0].toString().c_str());

	return Value();
}

Value ESIPImageAdapter::getPixel(Object *pThis, std::vector<Value> &arguments, void *pUserParam)
{
	ESIPImage *pImage = static_cast<ESIPImage*>(pThis->m_pUserParam);

	if (2 <= arguments.size())
	{
		long x = (long)arguments[0].toNumber();
		long y = (long)arguments[1].toNumber();

		unsigned char r, g, b;
		pImage->getPixel(x, y, r, g, b);

		Object *pObject = pImage->m_pInterpreter->createObject();
		pObject->setVariable(L"r", (double)r);
		pObject->setVariable(L"g", (double)g);
		pObject->setVariable(L"b", (double)b);

		return pObject;
	}

	return Value();
}

Value ESIPImageAdapter::setPixel(Object *pThis, std::vector<Value> &arguments, void *pUserParam)
{
	ESIPImage *pImage = static_cast<ESIPImage*>(pThis->m_pUserParam);

	if (5 <= arguments.size())
	{
		long x = (long)arguments[0].toNumber();
		long y = (long)arguments[1].toNumber();
		unsigned char r = (unsigned char)arguments[2].toNumber();
		unsigned char g = (unsigned char)arguments[3].toNumber();
		unsigned char b = (unsigned char)arguments[4].toNumber();

		pImage->setPixel(x, y, r, g, b);
	}

	return Value();
}
