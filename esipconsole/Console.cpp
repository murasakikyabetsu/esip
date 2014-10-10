#include "stdafx.h"
#include "Console.h"

#include <conio.h>
#include <iostream>

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
					::GetConsoleCursorInfo(hStdout, &cci);
					bool visible = cci.bVisible ? true : false;

					cci.bVisible = arguments[0].toBoolean();
					::SetConsoleCursorInfo(hStdout, &cci);

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



const wchar_t Window::className[] = L"esip_window";


Window::Window(ESInterpreter *pInterpreter) : NativeObject(pInterpreter), m_wnd(nullptr)
{

}

Window::~Window()
{
#if 1
	if (m_wnd)
	{
		::PostMessage(m_wnd, WM_CLOSE, 0, 0);
		::SetWindowLongPtr(m_wnd, GWL_USERDATA, 0);
	}

	m_thread.detach();
#else
	// I don't why but it doesn't return from join
	if (m_wnd)
		::PostMessage(m_wnd, WM_CLOSE, 0, 0);

	m_thread.join();
#endif
}

Object* Window::createObject(ESInterpreter *pInterpreter)
{
	ObjectPtr pObject = pInterpreter->createNativeObject<Window>(L"Window",
	{},
	{});

	pObject->setVariable(L"Image", ImageWindow::createObject(pInterpreter));

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = Window::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = ::GetModuleHandle(nullptr);
	wcex.hIcon = nullptr;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = className;
	wcex.hIconSm = nullptr;

	// ウインドウクラス生成
	::RegisterClassEx(&wcex);

	return pObject;
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Window *p = nullptr;

	if (msg == WM_CREATE)
	{
		CREATESTRUCT *pCS = (CREATESTRUCT*)lParam;
		::SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)pCS->lpCreateParams);

		p = (Window*)pCS->lpCreateParams;
	}
	else
	{
		p = (Window*)(::GetWindowLongPtr(hWnd, GWL_USERDATA));
	}

	if (p)
		return p->WndProc(msg, wParam, lParam);
	else
		return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

Value Window::constructor(Object *pThis, std::vector<Value> arguments)
{
	std::mutex m;
	std::condition_variable cv;
	bool ready = false;

	m_thread = std::thread([&]
	{
		{
			std::lock_guard<std::mutex> lg(m);

			if (0 < arguments.size() && arguments[0].toObject()->m_class == L"Window")
			{
				m_wnd = ::CreateWindowEx(0,
					className,
					L"",
					WS_CHILD | WS_VISIBLE,
					0, 0, 500, 500,
					static_cast<Window*>(arguments[0].toObject()->m_pUserParam)->m_wnd,
					nullptr,
					::GetModuleHandle(nullptr),
					this);
			}
			else
			{
				m_wnd = ::CreateWindowEx(0,
					className,
					L"",
					WS_OVERLAPPEDWINDOW,
					CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
					nullptr,
					nullptr,
					::GetModuleHandle(nullptr),
					this);
			}

			::ShowWindow(m_wnd, SW_SHOW);
			::UpdateWindow(m_wnd);

			ready = true;
		}
		cv.notify_one();

		MSG msg;
		while (0 < ::GetMessage(&msg, NULL, 0, 0))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	});

	std::unique_lock<std::mutex> ul(m);
	cv.wait(ul, [&] { return ready; });

	return Value();
}

LRESULT Window::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		::DestroyWindow(m_wnd);
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	default:
		return ::DefWindowProc(m_wnd, msg, wParam, lParam);
	}

	return 0;
}


//////////////////////////////////////////////////////////

ImageWindow::ImageWindow(ESInterpreter *pInterpreter) : Window(pInterpreter), m_pImage(nullptr), m_bmp(nullptr), m_dc(nullptr)
{

}

ImageWindow::~ImageWindow()
{
	::DeleteDC(m_dc);
}


Object* ImageWindow::createObject(ESInterpreter *pInterpreter)
{
	ObjectPtr pObject = pInterpreter->createNativeObject<ImageWindow>(L"ImageWindow",
	{},
	{});

	return pObject;
}

bool ImageWindow::setVariable(Object *pThis, const wchar_t *pName, const Value &value)
{
	if (::wcscmp(pName, L"src") == 0)
	{
		m_pImage = static_cast<ESIPImage*>(value.toObject()->m_pUserParam);

		::DeleteObject(m_bmp);
		BITMAPINFO bi = {0};
		bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth = m_pImage->getWidth();
		bi.bmiHeader.biHeight = m_pImage->getHeight();
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biBitCount = 24;
		bi.bmiHeader.biCompression = BI_RGB;
		void *pData;
		m_bmp = ::CreateDIBSection(m_dc, &bi, DIB_RGB_COLORS, &pData, nullptr, 0);

		for (long y = 0; y < m_pImage->getHeight(); y++)
		{
			::memcpy((unsigned char*)pData + (m_pImage->getHeight() - y - 1) * ((m_pImage->getWidth() * 3 + 3) & (~3)), &m_pImage->m_data[y * m_pImage->getWidth() * 3], m_pImage->getWidth() * 3);
		}

		::InvalidateRect(m_wnd, nullptr, true);

		return true;
	}

	return false;
}

Value ImageWindow::constructor(Object *pThis, std::vector<Value> arguments)
{
	Value v = Window::constructor(pThis, arguments);

	m_dc = ::CreateCompatibleDC(nullptr);

	return v;
}

LRESULT ImageWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC dc = ::BeginPaint(m_wnd, &ps);

		if (m_pImage)
		{
			HBITMAP bmp = (HBITMAP)::SelectObject(m_dc, m_bmp);
			::BitBlt(dc, 0, 0, m_pImage->getWidth(), m_pImage->getHeight(), m_dc, 0, 0, SRCCOPY);
			::SelectObject(m_dc, bmp);
		}

		::EndPaint(m_wnd, &ps);

		break;
	}

	default:
		return Window::WndProc(msg, wParam, lParam);
	}

	return 0;
}
