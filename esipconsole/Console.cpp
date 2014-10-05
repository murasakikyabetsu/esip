#include "stdafx.h"
#include "Console.h"

#include <conio.h>
#include <iostream>

#include <Windows.h>

Console::Console(ESInterpreter *pInterpreter) : NativeObject(pInterpreter)
{
}

Console::~Console()
{
}

Object* Console::createObject(ESInterpreter *pInterpreter)
{
	ObjectPtr pObject = pInterpreter->createNativeObject<Console>(L"Console",
	{
		{
			L"log", [](ESInterpreter *pInterpreter, Object *pThis, std::vector<Value> arguments)
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
			}
		},

		{
			L"print", [](ESInterpreter *pInterpreter, Object *pThis, std::vector<Value> arguments)
			{
				if (0 < arguments.size())
				{
					std::wcout << arguments[0].toString();
				}
				return Value();
			}
		},

		{
			L"cursor", [](ESInterpreter *pInterpreter, Object *pThis, std::vector<Value> arguments)
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
			}
		},

		{
			L"sleep", [](ESInterpreter *pInterpreter, Object *pThis, std::vector<Value> arguments)
			{
				if (0 < arguments.size())
					::Sleep((DWORD)arguments[0].toNumber());
				return Value();
			}
		},

		{
			L"pause", [](ESInterpreter *pInterpreter, Object *pThis, std::vector<Value> arguments)
			{
				::_getch();
				return Value();
			}
		},

		{
			L"cls", [](ESInterpreter *pInterpreter, Object *pThis, std::vector<Value> arguments)
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
			}
		},

		{
			L"locate", [](ESInterpreter *pInterpreter, Object *pThis, std::vector<Value> arguments)
			{
				if (2 <= arguments.size())
				{
					HANDLE hStdout = ::GetStdHandle(STD_OUTPUT_HANDLE);
					COORD coset = { (WORD)arguments[0].toNumber(), (WORD)arguments[1].toNumber() };
					::SetConsoleCursorPosition(hStdout, coset);
				}
				return Value();
			}
		},

		{
			L"color", [](ESInterpreter *pInterpreter, Object *pThis, std::vector<Value> arguments)
			{
				if (1 <= arguments.size())
				{
					HANDLE hStdout = ::GetStdHandle(STD_OUTPUT_HANDLE);
					WORD attr = (WORD)arguments[0].toNumber();
					::SetConsoleTextAttribute(hStdout, attr);
				}
				return Value();
			}
		},

		{
			L"key", [](ESInterpreter *pInterpreter, Object *pThis, std::vector<Value> arguments)
			{
				if (1 <= arguments.size())
				{
					return Value(::GetAsyncKeyState((int)arguments[0].toNumber()) ? true : false);
				}
				return Value();
			}
		}
	}, {});

	pObject->setVariable(L"I", 8.0);
	pObject->setVariable(L"R", 4.0);
	pObject->setVariable(L"G", 2.0);
	pObject->setVariable(L"B", 1.0);

	return pObject;
}
