#include "stdafx.h"
#include "Input.h"
#include "ErrMsg.h"
#include <winuser.h>

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

Input::Input()
{
	for (int i = 0; i < 255; i++)
	{
		_vKeys[i] = (GetAsyncKeyState(i) & 0x8000) ? true : false;
	}

	SDL_GetMouseState(&_mousePos.x, &_mousePos.y);
	_mouseX = _mousePos.x;
	_mouseY = _mousePos.y;
}

bool Input::Update(Window& window)
{
	_isInFocus = SDL_GetWindowFlags(window.GetWindow()) & SDL_WINDOW_INPUT_FOCUS;

	if (_cursorLocked)
	{
		SDL_GetRelativeMouseState(&_mousePos.x, &_mousePos.y);
	}
	else
	{
		SDL_GetMouseState(&_mousePos.x, &_mousePos.y);
	}

	_realWindowWidth = window.GetPhysicalWidth();
	_realWindowHeight = window.GetPhysicalHeight();

	_isFullscreen = window.IsFullscreen();
	if (_isFullscreen)
	{
		_mousePos.x = (_mousePos.x * (float)_windowWidth / (float)_realWindowWidth);
		_mousePos.y = (_mousePos.y * (float)_windowHeight / (float)_realWindowHeight);
	}

	_mouseX = _mousePos.x;
	_mouseY = _mousePos.y;

	for (int i = 0; i < 255; i++)
	{
		_lvKeys[i] = _vKeys[i];
		_vKeys[i] = (GetAsyncKeyState(i) & 0x8000) ? true : false;
	}

	return true;
}

KeyState Input::GetKey(const KeyCode keyCode) const
{
	if (_disable) return KeyState::None;

	const unsigned char key = static_cast<unsigned char>(keyCode);
	return GetKey(static_cast<UCHAR>(keyCode));
}
KeyState Input::GetKey(const UCHAR keyCode) const
{
	if (_disable) return KeyState::None;

	const bool
		thisFrame = _vKeys[keyCode],
		lastFrame = _lvKeys[keyCode];

	if (thisFrame)
	{
		if (lastFrame)
			return KeyState::Held;

		return KeyState::Pressed;
	}

	if (lastFrame)
		return KeyState::Released;

	return KeyState::None;
}

void Input::GetPressedKeys(std::vector<KeyCode> &keys) const
{
	if (_disable) return;

	for (int i = 0; i < 255; i++)
	{
		if (_vKeys[i])
			keys.push_back(static_cast<KeyCode>(i));
	}
}
KeyCode Input::GetPressedKey() const
{
	if (_disable) return KeyCode::None;

	for (int i = 0; i < 255; i++)
	{
		if (_vKeys[i])
			return static_cast<KeyCode>(i);
	}
	return KeyCode::None;
}

MouseState Input::GetMouse() const
{
	return {
		_mouseX,
		_mouseY,
		_mouseX - _lMouseX,
		_mouseY - _lMouseY,
		_scrollX,
		_scrollY
	};
}

WindowSize Input::GetWindowSize() const
{
	return {
		_windowWidth,
		_windowHeight
	};
}

WindowSize Input::GetRealWindowSize() const
{
	return {
		_realWindowWidth,
		_realWindowHeight
	};
}

WindowSize Input::GetActiveWindowSize() const
{
	if (_isFullscreen)
		return GetWindowSize();
	else
		return GetRealWindowSize();
}

void Input::SetWindowSize(WindowSize &size)
{
	_windowWidth = size.width;
	_windowHeight = size.height;
}

void Input::SetMouseScroll(float scrollX, float scrollY)
{
	_scrollX = scrollX;
	_scrollY = scrollY;
}

void Input::SetMousePosition(Window &window, float mouseX, float mouseY)
{
	_mouseX = mouseX;
	_mouseY = mouseY;
	SDL_WarpMouseInWindow(window.GetWindow(), _mouseX, _mouseY);
}

bool Input::IsPressedOrHeld(KeyCode keyCode) const
{
	if (_disable) return false;

	const KeyState state = GetKey(keyCode);

	return (state == KeyState::Pressed || state == KeyState::Held);
}

bool Input::IsInFocus() const { return _isInFocus; }
bool Input::IsCursorLocked() const { return _cursorLocked; }

void Input::DisableAllInput()
{
	_disable = true;
}

void Input::EnableAllInput()
{
	_disable = false;
}

bool Input::ToggleLockCursor(Window& window)
{
	SDL_Window* sdl_window = window.GetWindow();

	// Toggle cursor lock.
	SDL_SetWindowRelativeMouseMode(sdl_window, !_cursorLocked);
	_cursorLocked = SDL_GetWindowRelativeMouseMode(sdl_window);

	// Warp cursor to center of window if unlocked.
	if(!_cursorLocked)
		SDL_WarpMouseInWindow(sdl_window, window.GetWidth()/2.0f, window.GetHeight()/2.0f);

	return _cursorLocked;
}
