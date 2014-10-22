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

	Console(ESInterpreter *pInterpreter, ObjectPtr pThis);
	virtual ~Console();

};

class Window : public NativeObject
{
protected:

	static const wchar_t CLASSNAME[];

	std::thread m_thread;

	ObjectPtr m_pClickHandler;
	ObjectPtr m_pCloseHandler;

	static ESException m_exception;

public:

	HWND m_wnd;

public:

	static Object* createObject(ESInterpreter *pInterpreter);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static bool proc();
	static void exit();

protected:

	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

	virtual LRESULT commandHandler(int action);

	virtual void createWindow(std::wstring className, std::vector<Value> arguments);

public:

	Window(ESInterpreter *pInterpreter, ObjectPtr pThis);
	virtual ~Window();

	virtual bool setVariable(const wchar_t *pName, const Value &value);

	virtual Value constructor(std::vector<Value> arguments);
	virtual Value appendChild(std::vector<Value> arguments);

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

	ImageWindow(ESInterpreter *pInterpreter, ObjectPtr pThis);
	virtual ~ImageWindow();

	virtual bool setVariable(const wchar_t *pName, const Value &value);

	virtual Value constructor(std::vector<Value> arguments);

};

class Button : public Window
{
public:

	static Object* createObject(ESInterpreter *pInterpreter);

protected:

	virtual LRESULT commandHandler(int action);

public:

	Button(ESInterpreter *pInterpreter, ObjectPtr pThis);
	virtual ~Button();

	virtual Value constructor(std::vector<Value> arguments);
};
