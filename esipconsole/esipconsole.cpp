// esipconsole.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>

#include "..\esip\ESInterpreter.h"
#include "..\esip\NonStandardObjects.h"
#include "..\esipmisc\ESIPImage.h"
#include "Console.h"

#include <Windows.h>

std::wstring getReasonText(ESException &e)
{
	switch (e.m_reason)
	{
	case ESException::R_SYNTAXERROR:
		return L"Syntax error : Unexpected token \'" + e.m_information + L"\'";
	case ESException::R_REFERENCEERROR:
		return L"Reference error : \'" + e.m_information + L"\' is not defined";
	case ESException::R_TYPEERROR:
		return L"Type error : \'" + e.m_information + L"\'";
	}

	return L"";
}

void callback(int line, int linePos)
{
#ifdef _DEBUG
//	::wprintf_s(L"%d\n", line);
#endif
}

int _tmain(int argc, _TCHAR* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	ESInterpreter ip(callback);

	ip.getGlobalObject()->setVariable(L"ArrayBuffer", ArrayBuffer::createObject(&ip));
	ip.getGlobalObject()->setVariable(L"Uint8Array", Uint8Array::createObject(&ip));

	Object *pESIP = ip.createObject();
	pESIP->setVariable(L"Image", ESIPImage::createObject(&ip));
	ip.getGlobalObject()->setVariable(L"ESIP", pESIP);

	ip.getGlobalObject()->setVariable(L"console", Console::createObject(&ip));

	if (1 < argc)
	{
		for (int n = 1; n < argc; n++)
		{
			std::wifstream ifs(argv[n]);
			if (!ifs)
				return -1;

			std::wstring line;
			std::wstring source;
			while (std::getline(ifs, line))
			{
				source += line + L"\r\n";
			}

			try
			{
				ip.run(source.c_str());
			}
			catch (ESException &e)
			{
				std::wcout << L"Error : " << argv[n] << L"(" << e.m_line << L"," << e.m_posInLine << L") : " << getReasonText(e) << std::endl;

#ifdef _DEBUG
				::system("pause");
#endif

				break;
			}
		}
	}
	else
	{
		while (1)
		{
			std::wcout << L">";

			std::wstring line;
			std::getline(std::wcin, line);

			if (line == L"exit")
				break;

			try
			{
				Value value = ip.run(line.c_str());
				std::wcout << value.toString() << std::endl;
			}
			catch (ESException &e)
			{
				std::wcout << L"Error : " << L"(" << e.m_line << L"," << e.m_posInLine << L") : " << getReasonText(e) << std::endl;
			}
		}
	}

	return 0;
}

