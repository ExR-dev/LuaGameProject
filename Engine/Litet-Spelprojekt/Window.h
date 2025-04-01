#pragma once
#include <Windows.h>
#include <iostream>
#include <SDL3/SDL.h>
#include <d3d11.h>
#include <wrl/client.h>

class Input;

enum class WindowType
{
	MAIN,
	SECONDARY
};

class Window {
private:
	std::string _name = "Window";
	HWND _hwnd = NULL;
	UINT _height = NULL;
	UINT _width = NULL;
	int _physicalWidth = NULL;
	int _physicalHeight = NULL;
	bool _isFullscreen = false;

	bool _isClosing = false;

	WindowType _windowType = WindowType::MAIN;

	SDL_Surface* _surface = nullptr;
	SDL_Window* _window = nullptr;

	D3D11_VIEWPORT _viewport = { };
	Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain = nullptr;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> _uav = nullptr;

	bool SetupWindow();

public:
	Window();
	Window(std::string name, UINT width, UINT height, WindowType windowType = WindowType::MAIN);
	~Window() = default;

	bool Initialize(std::string name, UINT width, UINT height, WindowType windowType = WindowType::MAIN);
	bool ToggleFullscreen();
	bool UpdateWindowSize();

	bool UpdateWindow(Input* input);

	HWND GetHWND() const;
	UINT GetHeight() const;
	UINT GetWidth() const;
	int GetPhysicalHeight() const;
	int GetPhysicalWidth() const;
	SDL_Surface* GetSurface() const;
	SDL_Window* GetWindow() const;
	const D3D11_VIEWPORT* GetViewport() const;
	IDXGISwapChain* GetSwapChain() const;
	ID3D11UnorderedAccessView* GetUAV() const;
	WindowType GetWindowType() const;
	bool IsFullscreen() const;
	bool IsClosing() const;

	IDXGISwapChain** GetSwapChainAddress();
	ID3D11UnorderedAccessView** GetUAVAddress();
};
