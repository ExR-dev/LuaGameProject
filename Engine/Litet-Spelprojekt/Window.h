#pragma once
#include <Windows.h>
#include <iostream>

class Input;

enum class WindowType
{
	MAIN,
	SECONDARY
};

class Window {
protected:
	std::string _name = "Window";

	HWND _hwnd = NULL;

	UINT _height = NULL;
	UINT _width = NULL;
	int _physicalWidth = NULL;
	int _physicalHeight = NULL;
	bool _isFullscreen = false;

	bool _isClosing = false;

	WindowType _windowType = WindowType::MAIN;


public:
	Window();
	Window(std::string name, UINT width, UINT height, WindowType windowType = WindowType::MAIN);
	~Window() = default;

	virtual bool Initialize(std::string name, UINT width, UINT height, WindowType windowType = WindowType::MAIN) = 0;
	virtual bool ToggleFullscreen() = 0;
	virtual bool UpdateWindowSize() = 0;

	virtual bool UpdateWindow(Input* input) = 0;

	HWND GetHWND() const;
	UINT GetHeight() const;
	UINT GetWidth() const;
	int GetPhysicalHeight() const;
	int GetPhysicalWidth() const;
	WindowType GetWindowType() const;
	bool IsFullscreen() const;
	bool IsClosing() const;
};
