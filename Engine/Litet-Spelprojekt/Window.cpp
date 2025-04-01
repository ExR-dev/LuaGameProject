#include "stdafx.h"
#include "Window.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

bool Window::SetupWindow()
{
	SDL_InitFlags initFlags = 0;
	initFlags |= SDL_INIT_AUDIO;
	initFlags |= SDL_INIT_VIDEO;
	initFlags |= SDL_INIT_EVENTS;

	if (!SDL_Init(initFlags))
	{
		std::cout << "Error initializing SDL" << ": " << SDL_GetError() << std::endl;
		return false;
	}

	SDL_WindowFlags windowFlags = 0;
	windowFlags |= SDL_WINDOW_RESIZABLE;
	/*windowFlags |= SDL_WINDOW_MOUSE_GRABBED;
	windowFlags |= SDL_WINDOW_INPUT_FOCUS;
	windowFlags |= SDL_WINDOW_MOUSE_FOCUS;
	windowFlags |= SDL_WINDOW_MOUSE_CAPTURE;*/

	_window = SDL_CreateWindow(_name.c_str(), _width, _height, windowFlags);
	if (!_window) {
		std::cerr << "Couldn't create window: " << SDL_GetError() << std::endl;
		return false;
	}

	SDL_PropertiesID props = SDL_GetWindowProperties(_window);
	_hwnd = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
	if (!_hwnd) {
		std::cerr << "Couldn't get window handle: " << SDL_GetError() << std::endl;
		return false;
	}
    return true;
}

Window::Window()
{
}

Window::Window(std::string name, UINT width, UINT height, WindowType windowType)
{
	Initialize(name, width, height, windowType);
}

bool Window::Initialize(std::string name, UINT width, UINT height, WindowType windowType)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(464124224, "Window Initialize");
#endif

	_name = name;
	_width = width;
	_height = height;
	_windowType = windowType;

	if (!SetupWindow())
	{
		std::cerr << "Failed to setup window!" << std::endl;
		return false;
	}

	_viewport.TopLeftX = 0;
	_viewport.TopLeftY = 0;
	_viewport.Width = static_cast<float>(width);
	_viewport.Height = static_cast<float>(height);
	_viewport.MinDepth = 0;
	_viewport.MaxDepth = 1;

	return true;
}

bool Window::ToggleFullscreen()
{
	if (!SDL_SetWindowFullscreen(_window, !_isFullscreen))
		return false;

	_isFullscreen = !_isFullscreen;
	SDL_GetWindowSize(_window, &_physicalWidth, &_physicalHeight);
	return true;
}

bool Window::UpdateWindowSize()
{
	return SDL_GetWindowSize(_window, &_physicalWidth, &_physicalHeight);
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

SDL_Surface* Window::GetSurface() const
{
	return _surface;
}

SDL_Window* Window::GetWindow() const
{
	return _window;
}

const D3D11_VIEWPORT* Window::GetViewport() const
{
	return &_viewport;
}

IDXGISwapChain* Window::GetSwapChain() const
{
	return _swapChain.Get();
}

ID3D11UnorderedAccessView* Window::GetUAV() const
{
	return _uav.Get();
}

WindowType Window::GetWindowType() const
{
	return _windowType;
}

bool Window::IsFullscreen() const
{
	return _isFullscreen;
}

IDXGISwapChain** Window::GetSwapChainAddress()
{
	return _swapChain.ReleaseAndGetAddressOf();
}

ID3D11UnorderedAccessView** Window::GetUAVAddress()
{
	return _uav.ReleaseAndGetAddressOf();
}
