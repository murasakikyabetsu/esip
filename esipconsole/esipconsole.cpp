#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <regex>

#include "..\esip\ESInterpreter.h"
#include "..\esip\NonStandardObjects.h"
#include "..\esipmisc\ESIPImage.h"
#include "Console.h"

#include <Windows.h>

class ESIPHelper
{
private:

	std::list<std::wstring> m_queue;
	std::mutex m_mutex;

public:

	bool m_done;

public:

	static void callback(int line, int linePos)
	{
#ifdef _DEBUG
		//	::wprintf_s(L"%d\n", line);
#endif
	}

private:

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

public:

	ESIPHelper() : m_done(false) {}
	virtual ~ESIPHelper() {}

	void setQueue(std::wstring str)
	{
		std::lock_guard<std::mutex> lg(m_mutex);

		m_queue.push_back(std::move(str));
	}

	bool isQueueEmpty()
	{
		std::lock_guard<std::mutex> lg(m_mutex);
		return m_queue.empty() || m_done;
	}

	void run(bool exitIfError, bool continuation)
	{
		ESInterpreter ip(ESIPHelper::callback);

		ip.getGlobalObject()->setVariable(L"ArrayBuffer", ArrayBuffer::createObject(&ip));
		ip.getGlobalObject()->setVariable(L"Uint8Array", Uint8Array::createObject(&ip));

		ObjectPtr pESIP = ip.createObject();
		pESIP->setVariable(L"Image", ESIPImage::createObject(&ip));
		ip.getGlobalObject()->setVariable(L"ESIP", (Object*)pESIP);

		ip.getGlobalObject()->setVariable(L"console", Console::createObject(&ip));
		ip.getGlobalObject()->setVariable(L"Window", Window::createObject(&ip));

		ip.getGlobalObject()->setVariable(L"Global", ip.getGlobalObject());

		ip.getGlobalObject()->setVariable(L"exit", ip.createFunctionObject([&](Object*, std::vector<Value>, void*)
		{
			Window::exit();
			return Value();
		}, nullptr, nullptr));

		while (1)
		{
			std::lock_guard<std::mutex> lg(m_mutex);

			try
			{
				if (0 < m_queue.size())
				{
					std::wstring source = std::move(*m_queue.begin());
					m_queue.pop_front();

					ip.run(source.c_str());
				}

				if (m_queue.size() == 0 && !continuation)
					break;

				if (!Window::proc())
					break;
			}
			catch (ESException &e)
			{
				std::wcout << L"Error : " << L"(" << e.m_line << L"," << e.m_posInLine << L") : " << getReasonText(e) << std::endl;

				if (exitIfError)
				{
					Window::exit();
#ifdef _DEBUG
					::system("pause");
#endif

					break;
				}
			}
		}

		m_done = true;
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	ESIPHelper helper;

	bool continuation = false;
	bool interaction = false;
	for (int n = 1; n < argc; n++)
	{
		if (argv[n][0] == L'-' || argv[n][0] == L'/')
		{
			if (::wcscmp(argv[n] + 1, L"continuation") == 0)
				continuation = true;
			else if (::wcscmp(argv[n] + 1, L"interaction") == 0)
				interaction = true;
		}
		else
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

			helper.setQueue(source);
		}
	}

	if (helper.isQueueEmpty())
		interaction = true;

	std::thread thread;
	if (interaction)
	{
		thread = std::thread([&]
		{
			while (!helper.m_done)
			{
				std::wcout << L">";

				std::wstring line;
				std::getline(std::wcin, line);

				helper.setQueue(std::move(line));

				while (!helper.isQueueEmpty())
				{
					std::chrono::milliseconds span(1);
					std::this_thread::sleep_for(span);
				}
			}
		});
	}

	helper.run(!interaction, interaction || continuation);

	if (interaction)
	{
		// for interrupting std::getline. I'm looking for a way which doesn't have platform dependency.
		INPUT_RECORD ip[2];
		ip[0].EventType = KEY_EVENT;
		ip[0].Event.KeyEvent.bKeyDown = TRUE;
		ip[0].Event.KeyEvent.dwControlKeyState = 0;
		ip[0].Event.KeyEvent.uChar.UnicodeChar = L'\r';
		ip[0].Event.KeyEvent.wRepeatCount = 1;
		ip[0].Event.KeyEvent.wVirtualKeyCode = '\r';
		ip[0].Event.KeyEvent.wVirtualScanCode = MapVirtualKey('\r', MAPVK_VK_TO_VSC);
		ip[1].EventType = KEY_EVENT;
		ip[1].Event.KeyEvent.bKeyDown = TRUE;
		ip[1].Event.KeyEvent.dwControlKeyState = 0;
		ip[1].Event.KeyEvent.uChar.UnicodeChar = L'\n';
		ip[1].Event.KeyEvent.wRepeatCount = 1;
		ip[1].Event.KeyEvent.wVirtualKeyCode = '\n';
		ip[1].Event.KeyEvent.wVirtualScanCode = MapVirtualKey('\n', MAPVK_VK_TO_VSC);
		DWORD counts = 0;
		::WriteConsoleInput(::GetStdHandle(STD_INPUT_HANDLE), ip, 2, &counts);

		thread.join();
	}

	return 0;
}

