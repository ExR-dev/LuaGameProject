#include "stdafx.h"
#include "Window.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

Window::Window()
{
}

Window::Window(std::string name, UINT width, UINT height, WindowType windowType)
{
}

HWND Window::GetHWND() const
{
	return _hwnd;
}

UINT Window::GetHeight() const
{
	return _height;
}

UINT Window::GetWidth() const
{
	return _width;
}

int Window::GetPhysicalHeight() const
{
	return _physicalHeight;
}

int Window::GetPhysicalWidth() const
{
	return _physicalWidth;
}

WindowType Window::GetWindowType() const
{
	return _windowType;
}

bool Window::IsFullscreen() const
{
	return _isFullscreen;
}

bool Window::IsClosing() const
{
	return _isClosing;
}

