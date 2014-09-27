#include "stdafx.h"
#include "Console.h"

#include <conio.h>
#include <iostream>

#include <Windows.h>

ConsoleAdapter::ConsoleAdapter()
{
}


ConsoleAdapter::~ConsoleAdapter()
{
}

void ConsoleAdapter::operator()(ESInterpreter *pInterpreter, Object *pObject)
{
	Object *pConsole = pInterpreter->createObject();
	pObject->setVariable(L"console", pConsole);

	pConsole->setVariable(L"I", 8.0);
	pConsole->setVariable(L"R", 4.0);
	pConsole->setVariable(L"G", 2.0);
	pConsole->setVariable(L"B", 1.0);

	pConsole->setVariable(L"log", pInterpreter->createFunctionObject([](Object *pThis, std::vector<Value> &arguments, void *pUserParam)
	{
		if (0 < arguments.size())
		{
			HANDLE hStdout = ::GetStdHandle(STD_OUTPUT_HANDLE);

			CONSOLE_SCREEN_BUFFER_INFO csbf;
			::GetConsoleScreenBufferInfo(hStdout, &csbf);

			DWORD dwWritten;
			::FillConsoleOutputCharacter(hStdout, ' ', csbf.dwSize.X - csbf.dwCursorPosition.X, csbf.dwCursorPosition, &dwWritten);
			::SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
			::FillConsoleOutputAttribute(hStdout, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED, csbf.dwSize.X - csbf.dwCursorPosition.X, csbf.dwCursorPosition, &dwWritten);

			std::wcout << arguments[0].toString() << std::endl;
		}
		return Value();
	}, nullptr));

	pConsole->setVariable(L"print", pInterpreter->createFunctionObject([](Object *pThis, std::vector<Value> &arguments, void *pUserParam)
	{
		if (0 < arguments.size())
		{
			std::wcout << arguments[0].toString() << std::endl;
		}
		return Value();
	}, nullptr));

	pConsole->setVariable(L"cursor", pInterpreter->createFunctionObject([](Object *pThis, std::vector<Value> &arguments, void *pUserParam)
	{
		if (0 < arguments.size())
		{
			HANDLE hStdout = ::GetStdHandle(STD_OUTPUT_HANDLE);

			CONSOLE_CURSOR_INFO cci;
			GetConsoleCursorInfo(hStdout, &cci);
			bool visible = cci.bVisible ? true : false;

			cci.bVisible = arguments[0].toBoolean();
			SetConsoleCursorInfo(hStdout, &cci);

			return Value(visible);
		}
		return Value();
	}, nullptr));

	pConsole->setVariable(L"sleep", pInterpreter->createFunctionObject([](Object *pThis, std::vector<Value> &arguments, void *pUserParam)
	{
		if (0 < arguments.size())
			::Sleep((DWORD)arguments[0].toNumber());
		return Value();
	}, nullptr));

	pConsole->setVariable(L"pause", pInterpreter->createFunctionObject([](Object *pThis, std::vector<Value> &arguments, void *pUserParam)
	{
		::_getch();
		return Value();
	}, nullptr));

	pConsole->setVariable(L"cls", pInterpreter->createFunctionObject([](Object *pThis, std::vector<Value> &arguments, void *pUserParam)
	{
		HANDLE hStdout = ::GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_SCREEN_BUFFER_INFO csbf;
		::GetConsoleScreenBufferInfo(hStdout, &csbf);

		COORD coset = { 0, 0 };
		DWORD dwWritten;
		::FillConsoleOutputCharacter(hStdout, ' ', csbf.dwSize.X * csbf.dwSize.Y, coset, &dwWritten);
		::SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
		::FillConsoleOutputAttribute(hStdout, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED, csbf.dwSize.X * csbf.dwSize.Y, coset, &dwWritten);
		::SetConsoleCursorPosition(hStdout, coset);

		return Value();
	}, nullptr));

	pConsole->setVariable(L"locate", pInterpreter->createFunctionObject([](Object *pThis, std::vector<Value> &arguments, void *pUserParam)
	{
		if (2 <= arguments.size())
		{
			HANDLE hStdout = ::GetStdHandle(STD_OUTPUT_HANDLE);
			COORD coset = { (WORD)arguments[0].toNumber(), (WORD)arguments[1].toNumber() };
			::SetConsoleCursorPosition(hStdout, coset);
		}
		return Value();
	}, nullptr));

	pConsole->setVariable(L"color", pInterpreter->createFunctionObject([](Object *pThis, std::vector<Value> &arguments, void *pUserParam)
	{
		if (1 <= arguments.size())
		{
			HANDLE hStdout = ::GetStdHandle(STD_OUTPUT_HANDLE);
			WORD attr = (WORD)arguments[0].toNumber();
			::SetConsoleTextAttribute(hStdout, attr);
		}
		return Value();
	}, nullptr));

	pConsole->setVariable(L"key", pInterpreter->createFunctionObject([](Object *pThis, std::vector<Value> &arguments, void *pUserParam)
	{
		if (1 <= arguments.size())
		{
			return Value(::GetAsyncKeyState((int)arguments[0].toNumber()) ? true : false);
		}
		return Value();
	}, nullptr));
}
