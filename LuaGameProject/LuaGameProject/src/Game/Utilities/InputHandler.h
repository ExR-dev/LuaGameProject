#pragma once

#include <string>

#include "dep/raylib-cpp/raylib-cpp.hpp"

namespace Input
{

struct MouseInfo
{
	Vector2 position;
	Vector2 delta;
	float scroll;
};

enum KeyState 
{
	NONE		= 0b0000,
	PRESSED		= 0b0001,
	HELD		= 0b0010,
	RELEASED	= 0b0100,
};

// Physical Keys
enum GameKey 
{
	// Alphabetic Keys
	GAME_KEY_A,
	GAME_KEY_B,
	GAME_KEY_C,
	GAME_KEY_D,
	GAME_KEY_E,
	GAME_KEY_F,
	GAME_KEY_G,
	GAME_KEY_H,
	GAME_KEY_I,
	GAME_KEY_J,
	GAME_KEY_K,
	GAME_KEY_L,
	GAME_KEY_M,
	GAME_KEY_N,
	GAME_KEY_O,
	GAME_KEY_P,
	GAME_KEY_Q,
	GAME_KEY_R,
	GAME_KEY_S,
	GAME_KEY_T,
	GAME_KEY_U,
	GAME_KEY_V,
	GAME_KEY_W,
	GAME_KEY_X,
	GAME_KEY_Y,
	GAME_KEY_Z,	
	GAME_KEY_0,
	GAME_KEY_1,
	GAME_KEY_2,
	GAME_KEY_3,
	GAME_KEY_4,
	GAME_KEY_5,
	GAME_KEY_6,
	GAME_KEY_7,
	GAME_KEY_8,
	GAME_KEY_9,
	GAME_KEY_APOSTROPHE,
	GAME_KEY_COMMA,
	GAME_KEY_MINUS,
	GAME_KEY_PERIOD,
	GAME_KEY_SLASH,
	GAME_KEY_SEMICOLON,
	GAME_KEY_EQUAL,
	GAME_KEY_LEFT_BRACKET,
	GAME_KEY_RIGHT_BRACKET,
	GAME_KEY_BACKSLASH,
	GAME_KEY_GRAVE,

	// Function Keys
	GAME_KEY_SPACE,
	GAME_KEY_ESCAPE,
	GAME_KEY_ENTER,
	GAME_KEY_TAB,
	GAME_KEY_BACKSPACE,
	GAME_KEY_INSERT,
	GAME_KEY_DELETE,
	GAME_KEY_RIGHT,
	GAME_KEY_LEFT,
	GAME_KEY_DOWN,
	GAME_KEY_UP,
	GAME_KEY_PAGE_UP,
	GAME_KEY_PAGE_DOWN,
	GAME_KEY_HOME,
	GAME_KEY_END,
	GAME_KEY_CAPS_LOCK,
	GAME_KEY_SCROLL_LOCK,
	GAME_KEY_NUM_LOCK,
	GAME_KEY_PRINT_SCREEN,
	GAME_KEY_PAUSE,
	GAME_KEY_F1,
	GAME_KEY_F2,
	GAME_KEY_F3,
	GAME_KEY_F4,
	GAME_KEY_F5,
	GAME_KEY_F6,
	GAME_KEY_F7,
	GAME_KEY_F8,
	GAME_KEY_F9,
	GAME_KEY_F10,
	GAME_KEY_F11,
	GAME_KEY_F12,
	GAME_KEY_LEFT_SHIFT,
	GAME_KEY_LEFT_CONTROL,
	GAME_KEY_LEFT_ALT,
	GAME_KEY_LEFT_SUPER,
	GAME_KEY_RIGHT_SHIFT,
	GAME_KEY_RIGHT_CONTROL,
	GAME_KEY_RIGHT_ALT,
	GAME_KEY_RIGHT_SUPER,
	GAME_KEY_KB_MENU,

	// Numpad Keys
	GAME_KEY_NP_0,
	GAME_KEY_NP_1,
	GAME_KEY_NP_2,
	GAME_KEY_NP_3,
	GAME_KEY_NP_4,
	GAME_KEY_NP_5,
	GAME_KEY_NP_6,
	GAME_KEY_NP_7,
	GAME_KEY_NP_8,
	GAME_KEY_NP_9,
	GAME_KEY_NP_DECIMAL,
	GAME_KEY_NP_DIVIDE,
	GAME_KEY_NP_MULTIPLY,
	GAME_KEY_NP_SUBTRACT,
	GAME_KEY_NP_ADD,
	GAME_KEY_NP_ENTER,
	GAME_KEY_NP_EQUAL,

	GAME_KEY_COUNT
};

enum GameMouse
{
	GAME_MOUSE_LEFT,
	GAME_MOUSE_RIGHT,
	GAME_MOUSE_MIDDLE,
	GAME_MOUSE_SIDE,
	GAME_MOUSE_EXTRA,
	GAME_MOUSE_FORWARD,
	GAME_MOUSE_BACK,

	GAME_MOUSE_COUNT
};

KeyState GetKeyState(GameKey key);
KeyState GetMouseState(GameMouse btn);

MouseInfo GetMouseInfo();
void SetMousePos(Vector2 pos);

bool CheckKeyPressed(GameKey key);
bool CheckKeyHeld(GameKey key);
bool CheckKeyReleased(GameKey key);

bool CheckMousePressed(GameMouse btn);
bool CheckMouseHeld(GameMouse btn);
bool CheckMouseReleased(GameMouse btn);

void SetKeyState(GameKey key, KeyState state);
void SetMouseState(GameMouse btn, KeyState state);


// Retuns string with characters pressed, regardless of keyboard layout
std::string GetUnicodeInput();

void UpdateInput();

std::string GetKeyName(GameKey key);
std::string GetMouseName(GameMouse btn);

}
