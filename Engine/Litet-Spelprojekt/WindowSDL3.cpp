#include "stdafx.h"
#include "WindowSDL3.h"

#ifdef USE_SDL3

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

bool WindowSDL3::SetupWindow()
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

WindowSDL3::WindowSDL3():
	Window()
{
}

WindowSDL3::WindowSDL3(std::string name, UINT width, UINT height, WindowType windowType):
	Window(name, width, height, windowType)
{
	Initialize(name, width, height, windowType);
}

bool WindowSDL3::Initialize(std::string name, UINT width, UINT height, WindowType windowType)
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

	return true;
}

bool WindowSDL3::ToggleFullscreen()
{
	if (!SDL_SetWindowFullscreen(_window, !_isFullscreen))
		return false;

	_isFullscreen = !_isFullscreen;
	SDL_GetWindowSize(_window, &_physicalWidth, &_physicalHeight);
	return true;
}

bool WindowSDL3::UpdateWindowSize()
{
	return SDL_GetWindowSize(_window, &_physicalWidth, &_physicalHeight);
}

bool WindowSDL3::UpdateWindow(Input* input)
{		
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
#ifdef USE_IMGUI
		ImGui_ImplSDL3_ProcessEvent(&event);
#endif

		switch (event.type)
		{
		case SDL_EVENT_QUIT:
			_isClosing = true;
			break;

		case SDL_EVENT_WINDOW_RESIZED:
			UpdateWindowSize();
			break;

		case SDL_EVENT_MOUSE_WHEEL: // Scroll wheel event, couldn't be fetched from Input's update.
			input->SetMouseScroll(event.wheel.x, event.wheel.y);
			break;

		default:
			break;
		}
	}

	return true;
}


SDL_Surface* WindowSDL3::GetSurface() const
{
	return _surface;
}

SDL_Window* WindowSDL3::GetWindow() const
{
	return _window;
}

#endif

