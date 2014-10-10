#pragma once

#include "..\esip\ESInterpreter.h"
#include "..\esipmisc\ESIPImage.h"
#include <vector>
#include <future>

#include <Windows.h>

class Console : public NativeObject
{
public:

	static Object* createObject(ESInterpreter *pInterpreter);

public:

	Console(ESInterpreter *pInterpreter);
	virtual ~Console();

};

class Window : public NativeObject
{
protected:

	static const wchar_t className[];

	std::thread m_thread;

	HWND m_wnd;

public:

	static Object* createObject(ESInterpreter *pInterpreter);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:

	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:

	Window(ESInterpreter *pInterpreter);
	virtual ~Window();

	virtual Value constructor(Object *pThis, std::vector<Value> arguments);

};

class ImageWindow : public Window
{
private:

	ESIPImage *m_pImage;

	HBITMAP m_bmp;
	HDC m_dc;

public:

	static Object* createObject(ESInterpreter *pInterpreter);


protected:

	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:

	ImageWindow(ESInterpreter *pInterpreter);
	virtual ~ImageWindow();

	virtual bool setVariable(Object *pThis, const wchar_t *pName, const Value &value);

	virtual Value constructor(Object *pThis, std::vector<Value> arguments);

};