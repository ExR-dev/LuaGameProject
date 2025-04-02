#pragma once

#ifdef USE_SDL3

#include <Windows.h>
#include <iostream>
#include <d3d11.h>
#include <wrl/client.h>

#include <SDL3/SDL.h>

class Input;

class WindowSDL3: public Window {
private:
	SDL_Surface* _surface = nullptr;
	SDL_Window* _window = nullptr;

	bool SetupWindow();

public:
	WindowSDL3();
	WindowSDL3(std::string name, UINT width, UINT height, WindowType windowType = WindowType::MAIN);
	~WindowSDL3() = default;

	bool Initialize(std::string name, UINT width, UINT height, WindowType windowType = WindowType::MAIN) override;
	bool ToggleFullscreen() override;
	bool UpdateWindowSize() override;

	bool UpdateWindow(Input* input) override;

	SDL_Surface* GetSurface() const;
	SDL_Window* GetWindow() const;
};

#endif

