#include "stdafx.h"
#include "Console.h"

#include <sstream>
#include <conio.h>
#include <iostream>

Console::Console(ESInterpreter *pInterpreter, ObjectPtr pThis) : NativeObject(pInterpreter, pThis)
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


	pObject->setCapture([](const wchar_t *pName, const Value &value, void*)
	{
		if (::wcscmp(pName, L"visibility") == 0)
		{
			::ShowWindow(::GetConsoleWindow(), value.toBoolean() ? SW_SHOW : SW_HIDE);
		}
		else if (::wcscmp(pName, L"cursor") == 0)
		{
			HANDLE hStdout = ::GetStdHandle(STD_OUTPUT_HANDLE);

			CONSOLE_CURSOR_INFO cci;
			::GetConsoleCursorInfo(hStdout, &cci);

			cci.bVisible = value.toBoolean();
			::SetConsoleCursorInfo(hStdout, &cci);
		}
		else if (::wcscmp(pName, L"color") == 0)
		{
			::SetConsoleTextAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), (WORD)value.toNumber());
		}

		return false;
	}, nullptr, nullptr, nullptr);

	pObject->setVariable(L"I", 8.0);
	pObject->setVariable(L"R", 4.0);
	pObject->setVariable(L"G", 2.0);
	pObject->setVariable(L"B", 1.0);

	return pObject;
}



const wchar_t Window::CLASSNAME[] = L"esip_window";
ESException Window::m_exception;

Window::Window(ESInterpreter *pInterpreter, ObjectPtr pThis) : NativeObject(pInterpreter, pThis), m_wnd(nullptr)
{

}

Window::~Window()
{
	if (m_wnd)
		::DestroyWindow(m_wnd);	// In this case, the close handler isn't called.
}

Object* Window::createObject(ESInterpreter *pInterpreter)
{
	ObjectPtr pObject = pInterpreter->createNativeObject<Window>(L"Window",
	{
		{ L"getOpenFileName", [](ESInterpreter *pInterpreter, Object *pThis, std::vector<Value> arguments)
			{
				wchar_t path[MAX_PATH] = { L'\0' };

				OPENFILENAME ofn = { 0 };
				ofn.lStructSize = sizeof(ofn);
				ofn.lpstrFile = path;
				ofn.nMaxFile = sizeof(path);
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

				if (::GetOpenFileName(&ofn))
					return Value(path);

				return Value();
			}
		},
		{ L"getSaveFileName", [](ESInterpreter *pInterpreter, Object *pThis, std::vector<Value> arguments)
			{
				wchar_t path[MAX_PATH] = { L'\0' };

				OPENFILENAME ofn = { 0 };
				ofn.lStructSize = sizeof(ofn);
				ofn.lpstrFile = path;
				ofn.nMaxFile = sizeof(path);
				ofn.Flags = OFN_OVERWRITEPROMPT;

				if (::GetSaveFileName(&ofn))
					return Value(path);

				return Value();
			}
		}
	},
	{
		{ L"appendChild", &Window::appendChild }
	});
	pObject->setVariable(L"Image", ImageWindow::createObject(pInterpreter));
	pObject->setVariable(L"Button", Button::createObject(pInterpreter));

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
	wcex.lpszClassName = CLASSNAME;
	wcex.hIconSm = nullptr;
	::RegisterClassEx(&wcex);

	return pObject;
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Window *p = nullptr;

	try
	{
		if (msg == WM_CREATE)
		{
			CREATESTRUCT *pCS = (CREATESTRUCT*)lParam;
			::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCS->lpCreateParams);

			p = (Window*)pCS->lpCreateParams;
		}
		else
		{
			p = (Window*)(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
		}

		if (p)
			return p->WndProc(msg, wParam, lParam);
		else
			return ::DefWindowProc(hWnd, msg, wParam, lParam);
	}
	catch (ESException &e)
	{
		Window::m_exception = e;
	}

	return 0;
}

bool Window::proc()
{
	Window::m_exception.m_reason = ESException::R_NOERROR;

	MSG msg;
	if (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (::GetMessage(&msg, NULL, 0, 0) <= 0)
			return false;

		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	if (Window::m_exception.m_reason != ESException::R_NOERROR)
		throw Window::m_exception;

	return true;
}

void Window::exit()
{
	::PostQuitMessage(0);
}

Value Window::constructor(std::vector<Value> arguments)
{
	createWindow(CLASSNAME, arguments);

	return Value();
}

Value Window::appendChild(std::vector<Value> arguments)
{
	if (0 < arguments.size())
	{
		ObjectPtr pObject = arguments[0].toObject();
		if (pObject)
		{
			Window *pWnd = static_cast<Window*>(pObject->m_pUserParam);

			if (pWnd)
			{
				RECT windowRect;
				::GetWindowRect(pWnd->m_wnd, &windowRect);

				RECT clientRect;
				::GetClientRect(pWnd->m_wnd, &clientRect);

				::SetWindowLong(pWnd->m_wnd, GWL_STYLE, WS_CHILD | WS_VISIBLE);
				::SetParent(pWnd->m_wnd, m_wnd);

				::MoveWindow(pWnd->m_wnd, windowRect.left, windowRect.top, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, true);
			}
		}
	}

	return Value();
}

LRESULT Window::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
	{
		WORD action = HIWORD(wParam);
		Window *p = (Window*)(::GetWindowLongPtr((HWND)lParam, GWLP_USERDATA));
		if (p)
			return p->commandHandler(action);
	}
		break;
	case WM_CLOSE:
		if (m_pCloseHandler)
			m_pCloseHandler->call(m_pInterpreter->getGlobalObject(), {}, false);

		::DestroyWindow(m_wnd);
		break;
	default:
		return ::DefWindowProc(m_wnd, msg, wParam, lParam);
	}

	return 0;
}

LRESULT Window::commandHandler(int action)
{
	return 0;
}

void Window::createWindow(std::wstring className, std::vector<Value> arguments)
{
	m_wnd = ::CreateWindowEx(0,
		className.c_str(),
		L"",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr,
		nullptr,
		::GetModuleHandle(nullptr),
		this);

	if (className != CLASSNAME)
		::SetWindowLongPtr(m_wnd, GWLP_USERDATA, (LONG_PTR)this);

	if (0 < arguments.size() && arguments[0].toObject() && arguments[0].toObject()->m_class == L"Object")
	{
		ObjectPtr option = arguments[0].toObject();

		Value text = option->getVariable(L"text", true);
		if (text.m_type == Value::VT_STRING)
			m_pThis->setVariable(L"text", text);

		Value children = option->getVariable(L"children", true);
		if (children.m_type == Value::VT_OBJECT)
		{
			ObjectPtr pObj = children.toObject();
			int length = pObj->getVariable(L"length", true).toInt32();
			for (long n = 0; n < length; n++)
			{
				std::wstringstream buf;
				buf << n;
				appendChild({ pObj->getVariable(buf.str().c_str(), true) });
			}
		}

		Value left = option->getVariable(L"left", true);
		if (left.m_type == Value::VT_NUMBER)
			m_pThis->setVariable(L"left", left);

		Value top = option->getVariable(L"top", true);
		if (top.m_type == Value::VT_NUMBER)
			m_pThis->setVariable(L"top", top);

		Value width = option->getVariable(L"width", true);
		if (width.m_type == Value::VT_NUMBER)
			m_pThis->setVariable(L"width", width);

		Value height = option->getVariable(L"height", true);
		if (height.m_type == Value::VT_NUMBER)
			m_pThis->setVariable(L"height", height);

		Value resizable = option->getVariable(L"resizable", true);
		if (resizable.m_type == Value::VT_BOOLEAN)
			m_pThis->setVariable(L"resizable", resizable);
		else
			m_pThis->setVariable(L"resizable", true);

		Value clickHandler = option->getVariable(L"clickHandler", true);
		if (clickHandler.m_type == Value::VT_OBJECT)
			m_pThis->setVariable(L"clickHandler", clickHandler);

		Value closeHandler = option->getVariable(L"closeHandler", true);
		if (closeHandler.m_type == Value::VT_OBJECT)
			m_pThis->setVariable(L"closeHandler", closeHandler);
	}

	::ShowWindow(m_wnd, SW_SHOW);
	::UpdateWindow(m_wnd);

	RECT rect;
	::GetClientRect(m_wnd, &rect);
	m_pThis->setVariable(L"width", (double)rect.right - rect.left);
	m_pThis->setVariable(L"height", (double)rect.bottom - rect.top);
}

bool Window::setVariable(const wchar_t *pName, const Value &value)
{
	if (::wcscmp(pName, L"text") == 0)
	{
		::SetWindowText(m_wnd, value.toString().c_str());
	}
	else if (::wcscmp(pName, L"clickHandler") == 0)
	{
		ObjectPtr clickHandler = value.toObject();
		if (clickHandler->m_class != L"Function")
			throw ESException(ESException::R_TYPEERROR);
		m_pClickHandler = clickHandler;
	}
	else if (::wcscmp(pName, L"closeHandler") == 0)
	{
		ObjectPtr closeHandler = value.toObject();
		if (closeHandler->m_class != L"Function")
			throw ESException(ESException::R_TYPEERROR);
		m_pCloseHandler = closeHandler;
	}
	else if (::wcscmp(pName, L"left") == 0)
	{
		RECT windowRect;
		::GetWindowRect(m_wnd, &windowRect);
		::SetWindowPos(m_wnd, nullptr, value.toInt32(), windowRect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
	else if (::wcscmp(pName, L"top") == 0)
	{
		RECT windowRect;
		::GetWindowRect(m_wnd, &windowRect);
		::SetWindowPos(m_wnd, nullptr, windowRect.left, value.toInt32(), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
	else if (::wcscmp(pName, L"width") == 0)
	{
		RECT windowRect;
		::GetClientRect(m_wnd, &windowRect);
		windowRect.right = windowRect.left + value.toInt32();
		::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);
		::SetWindowPos(m_wnd, nullptr, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_NOMOVE | SWP_NOZORDER);
	}
	else if (::wcscmp(pName, L"height") == 0)
	{
		RECT windowRect;
		::GetClientRect(m_wnd, &windowRect);
		windowRect.bottom = windowRect.top + value.toInt32();
		::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);
		::SetWindowPos(m_wnd, nullptr, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_NOMOVE | SWP_NOZORDER);
	}
	else if (::wcscmp(pName, L"resizable") == 0)
	{
		long style = ::GetWindowLong(m_wnd, GWL_STYLE);
		if (value.toBoolean())
			::SetWindowLong(m_wnd, GWL_STYLE, style | WS_THICKFRAME);
		else
			::SetWindowLong(m_wnd, GWL_STYLE, style & (~WS_THICKFRAME));
	}

	return false;
}

//////////////////////////////////////////////////////////

ImageWindow::ImageWindow(ESInterpreter *pInterpreter, ObjectPtr pThis) : Window(pInterpreter, pThis), m_pImage(nullptr), m_bmp(nullptr), m_dc(nullptr)
{

}

ImageWindow::~ImageWindow()
{
	::DeleteDC(m_dc);
}


Object* ImageWindow::createObject(ESInterpreter *pInterpreter)
{
	return pInterpreter->createNativeObject<ImageWindow>(L"ImageWindow", {}, {});
}

bool ImageWindow::setVariable(const wchar_t *pName, const Value &value)
{
	if (Window::setVariable(pName, value))
		return true;

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
	}

	return false;
}

Value ImageWindow::constructor(std::vector<Value> arguments)
{
	Value v = Window::constructor(arguments);

	if (0 < arguments.size() && arguments[0].toObject() && arguments[0].toObject()->m_class == L"Object")
	{
		ObjectPtr option = arguments[0].toObject();

		Value src = option->getVariable(L"src", true);
		if (src.m_type == Value::VT_OBJECT)
			m_pThis->setVariable(L"src", src);
	}

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

			RECT rect;
			if (::GetClientRect(m_wnd, &rect))
			{
				::BitBlt(dc, (rect.left + rect.right - m_pImage->getWidth()) / 2, (rect.top + rect.bottom - m_pImage->getHeight()) / 2, m_pImage->getWidth(), m_pImage->getHeight(), m_dc, 0, 0, SRCCOPY);
			}

			::SelectObject(m_dc, bmp);
		}

		::EndPaint(m_wnd, &ps);
	}
		break;

	default:
		return Window::WndProc(msg, wParam, lParam);
	}

	return 0;
}

/////////////////////////////////////////////////


Button::Button(ESInterpreter *pInterpreter, ObjectPtr pThis) : Window(pInterpreter, pThis)
{

}

Button::~Button()
{

}

Object* Button::createObject(ESInterpreter *pInterpreter)
{
	return pInterpreter->createNativeObject<Button>(L"Button", {}, {});
}

LRESULT Button::commandHandler(int action)
{
	if (action == BN_CLICKED && m_pClickHandler)
		m_pClickHandler->call(m_pInterpreter->getGlobalObject(), {}, false);

	return 0;
}

Value Button::constructor(std::vector<Value> arguments)
{
	createWindow(L"BUTTON", arguments);

	return Value();
}

