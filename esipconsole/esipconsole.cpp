// esipconsole.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>

#include "..\esip\ESInterpreter.h"
#include "..\esip\ESIPObjects.h"
#include "..\esipmisc\ESIPImage.h"

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

int _tmain(int argc, _TCHAR* argv[])
{
	ESInterpreter ip;

	ip.getGlobalObject()->setNativeFunction(L"print", [](Object *pThis, std::vector<Value> &arguments, void *pUserParam)
	{
		if (0 < arguments.size())
			std::wcout << arguments[0].toString() << std::endl;
		return Value();
	}, nullptr);

	ip.getGlobalObject()->setNativeFunction(L"sleep", [](Object *pThis, std::vector<Value> &arguments, void *pUserParam)
	{
		if (0 < arguments.size())
			::Sleep((DWORD)arguments[0].toNumber());
		return Value();
	}, nullptr);

	ip.getGlobalObject()->setNativeFunction(L"pause", [](Object *pThis, std::vector<Value> &arguments, void *pUserParam)
	{
		::getchar();
		return Value();
	}, nullptr);

	Uint8ArrayAdapter()(ip.getGlobalObject());
	ESIPImageAdapter()(ip.getGlobalObject());

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

