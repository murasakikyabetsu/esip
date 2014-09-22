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

	pConsole->setVariable(L"log", pInterpreter->createFunctionObject([](Object *pThis, std::vector<Value> &arguments, void *pUserParam)
	{
		if (0 < arguments.size())
			std::wcout << arguments[0].toString() << std::endl;
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
}
