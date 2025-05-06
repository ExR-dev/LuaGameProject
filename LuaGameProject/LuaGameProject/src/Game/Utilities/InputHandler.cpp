#include "stdafx.h"

#include "InputHandler.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace Input;

KeyState keyStates[(int)GAME_KEY_COUNT];
KeyState mouseStates[(int)GAME_MOUSE_COUNT];

constexpr size_t MAX_BUFFER_SIZE = 16;
unsigned char bufferIndex = 0;
std::string unicodeBuffer(MAX_BUFFER_SIZE, 0);

std::unordered_map<GameKey, int> keyMapping =
{
	// Alphabetic Keys
	{GameKey::GAME_KEY_A					,		(int)KEY_A								},
	{GameKey::GAME_KEY_B					,		(int)KEY_B								},
	{GameKey::GAME_KEY_C					,		(int)KEY_C								},
	{GameKey::GAME_KEY_D					,		(int)KEY_D								},
	{GameKey::GAME_KEY_E					,		(int)KEY_E								},
	{GameKey::GAME_KEY_F					,		(int)KEY_F								},
	{GameKey::GAME_KEY_G					,		(int)KEY_G								},
	{GameKey::GAME_KEY_H					,		(int)KEY_H								},
	{GameKey::GAME_KEY_I					,		(int)KEY_I								},
	{GameKey::GAME_KEY_J					,		(int)KEY_J								},
	{GameKey::GAME_KEY_K					,		(int)KEY_K								},
	{GameKey::GAME_KEY_L					,		(int)KEY_L								},
	{GameKey::GAME_KEY_M					,		(int)KEY_M								},
	{GameKey::GAME_KEY_N					,		(int)KEY_N								},
	{GameKey::GAME_KEY_O					,		(int)KEY_O								},
	{GameKey::GAME_KEY_P					,		(int)KEY_P								},
	{GameKey::GAME_KEY_Q					,		(int)KEY_Q								},
	{GameKey::GAME_KEY_R					,		(int)KEY_R								},
	{GameKey::GAME_KEY_S					,		(int)KEY_S								},
	{GameKey::GAME_KEY_T					,		(int)KEY_T								},
	{GameKey::GAME_KEY_U					,		(int)KEY_U								},
	{GameKey::GAME_KEY_V					,		(int)KEY_V								},
	{GameKey::GAME_KEY_W					,		(int)KEY_W								},
	{GameKey::GAME_KEY_X					,		(int)KEY_X								},
	{GameKey::GAME_KEY_Y					,		(int)KEY_Y								},
	{GameKey::GAME_KEY_Z					,		(int)KEY_Z								},
	{GameKey::GAME_KEY_0					,		(int)KEY_ZERO							},
	{GameKey::GAME_KEY_1					,		(int)KEY_ONE							},
	{GameKey::GAME_KEY_2					,		(int)KEY_TWO							},
	{GameKey::GAME_KEY_3					,		(int)KEY_THREE							},
	{GameKey::GAME_KEY_4					,		(int)KEY_FOUR							},
	{GameKey::GAME_KEY_5					,		(int)KEY_FIVE							},
	{GameKey::GAME_KEY_6					,		(int)KEY_SIX							},
	{GameKey::GAME_KEY_7					,		(int)KEY_SEVEN							},
	{GameKey::GAME_KEY_8					,		(int)KEY_EIGHT							},
	{GameKey::GAME_KEY_9					,		(int)KEY_NINE							},
	{GameKey::GAME_KEY_APOSTROPHE			,		(int)KEY_APOSTROPHE						},
	{GameKey::GAME_KEY_COMMA				,		(int)KEY_COMMA							},
	{GameKey::GAME_KEY_MINUS				,		(int)KEY_MINUS							},
	{GameKey::GAME_KEY_PERIOD				,		(int)KEY_PERIOD							},
	{GameKey::GAME_KEY_SLASH				,		(int)KEY_SLASH							},
	{GameKey::GAME_KEY_SEMICOLON			,		(int)KEY_SEMICOLON						},
	{GameKey::GAME_KEY_EQUAL				,		(int)KEY_EQUAL							},
	{GameKey::GAME_KEY_LEFT_BRACKET			,		(int)KEY_LEFT_BRACKET					},
	{GameKey::GAME_KEY_RIGHT_BRACKET		,		(int)KEY_RIGHT_BRACKET					},
	{GameKey::GAME_KEY_BACKSLASH			,		(int)KEY_BACKSLASH						},
	{GameKey::GAME_KEY_GRAVE				,		(int)KEY_GRAVE							},

	// Function Keys		
	{GameKey::GAME_KEY_SPACE				,		(int)KEY_SPACE							},
	{GameKey::GAME_KEY_ESCAPE				,		(int)KEY_ESCAPE							},
	{GameKey::GAME_KEY_ENTER				,		(int)KEY_ENTER							},
	{GameKey::GAME_KEY_TAB					,		(int)KEY_TAB							},
	{GameKey::GAME_KEY_BACKSPACE			,		(int)KEY_BACKSPACE						},
	{GameKey::GAME_KEY_INSERT				,		(int)KEY_INSERT							},
	{GameKey::GAME_KEY_DELETE				,		(int)KEY_DELETE							},
	{GameKey::GAME_KEY_RIGHT				,		(int)KEY_RIGHT							},
	{GameKey::GAME_KEY_LEFT					,		(int)KEY_LEFT							},
	{GameKey::GAME_KEY_DOWN					,		(int)KEY_DOWN							},
	{GameKey::GAME_KEY_UP					,		(int)KEY_UP								},
	{GameKey::GAME_KEY_PAGE_UP				,		(int)KEY_PAGE_UP						},
	{GameKey::GAME_KEY_PAGE_DOWN			,		(int)KEY_PAGE_DOWN						},
	{GameKey::GAME_KEY_HOME					,		(int)KEY_HOME							},
	{GameKey::GAME_KEY_END					,		(int)KEY_END							},
	{GameKey::GAME_KEY_CAPS_LOCK			,		(int)KEY_CAPS_LOCK						},
	{GameKey::GAME_KEY_SCROLL_LOCK			,		(int)KEY_SCROLL_LOCK					},
	{GameKey::GAME_KEY_NUM_LOCK				,		(int)KEY_NUM_LOCK						},
	{GameKey::GAME_KEY_PRINT_SCREEN			,		(int)KEY_PRINT_SCREEN					},
	{GameKey::GAME_KEY_PAUSE				,		(int)KEY_PAUSE							},
	{GameKey::GAME_KEY_F1					,		(int)KEY_F1								},
	{GameKey::GAME_KEY_F2					,		(int)KEY_F2								},
	{GameKey::GAME_KEY_F3					,		(int)KEY_F3								},
	{GameKey::GAME_KEY_F4					,		(int)KEY_F4								},
	{GameKey::GAME_KEY_F5					,		(int)KEY_F5								},
	{GameKey::GAME_KEY_F6					,		(int)KEY_F6								},
	{GameKey::GAME_KEY_F7					,		(int)KEY_F7								},
	{GameKey::GAME_KEY_F8					,		(int)KEY_F8								},
	{GameKey::GAME_KEY_F9					,		(int)KEY_F9								},
	{GameKey::GAME_KEY_F10					,		(int)KEY_F10							},
	{GameKey::GAME_KEY_F11					,		(int)KEY_F11							},
	{GameKey::GAME_KEY_F12					,		(int)KEY_F12							},
	{GameKey::GAME_KEY_LEFT_SHIFT			,		(int)KEY_LEFT_SHIFT						},
	{GameKey::GAME_KEY_LEFT_CONTROL			,		(int)KEY_LEFT_CONTROL					},
	{GameKey::GAME_KEY_LEFT_ALT				,		(int)KEY_LEFT_ALT						},
	{GameKey::GAME_KEY_LEFT_SUPER			,		(int)KEY_LEFT_SUPER						},
	{GameKey::GAME_KEY_RIGHT_SHIFT			,		(int)KEY_RIGHT_SHIFT					},
	{GameKey::GAME_KEY_RIGHT_CONTROL		,		(int)KEY_RIGHT_CONTROL					},
	{GameKey::GAME_KEY_RIGHT_ALT			,		(int)KEY_RIGHT_ALT						},
	{GameKey::GAME_KEY_RIGHT_SUPER			,		(int)KEY_RIGHT_SUPER					},
	{GameKey::GAME_KEY_KB_MENU				,		(int)KEY_KB_MENU						},

	// Numpad Keys
	{GameKey::GAME_KEY_NP_0					,		(int)KEY_KP_0							},
	{GameKey::GAME_KEY_NP_1					,		(int)KEY_KP_1							},
	{GameKey::GAME_KEY_NP_2					,		(int)KEY_KP_2							},
	{GameKey::GAME_KEY_NP_3					,		(int)KEY_KP_3							},
	{GameKey::GAME_KEY_NP_4					,		(int)KEY_KP_4							},
	{GameKey::GAME_KEY_NP_5					,		(int)KEY_KP_5							},
	{GameKey::GAME_KEY_NP_6					,		(int)KEY_KP_6							},
	{GameKey::GAME_KEY_NP_7					,		(int)KEY_KP_7							},
	{GameKey::GAME_KEY_NP_8					,		(int)KEY_KP_8							},
	{GameKey::GAME_KEY_NP_9					,		(int)KEY_KP_9							},
	{GameKey::GAME_KEY_NP_DECIMAL			,		(int)KEY_KP_DECIMAL						},
	{GameKey::GAME_KEY_NP_DIVIDE			,		(int)KEY_KP_DIVIDE						},
	{GameKey::GAME_KEY_NP_MULTIPLY			,		(int)KEY_KP_MULTIPLY					},
	{GameKey::GAME_KEY_NP_SUBTRACT			,		(int)KEY_KP_SUBTRACT					},
	{GameKey::GAME_KEY_NP_ADD				,		(int)KEY_KP_ADD							},
	{GameKey::GAME_KEY_NP_ENTER				,		(int)KEY_KP_ENTER						},
	{GameKey::GAME_KEY_NP_EQUAL				,		(int)KEY_KP_EQUAL						}
};

std::unordered_map<GameMouse, int> mouseMapping =
{
	{GameMouse::GAME_MOUSE_LEFT				,		(int)MOUSE_BUTTON_LEFT					},
	{GameMouse::GAME_MOUSE_RIGHT			,		(int)MOUSE_BUTTON_RIGHT					},
	{GameMouse::GAME_MOUSE_MIDDLE			,		(int)MOUSE_BUTTON_MIDDLE				},
	{GameMouse::GAME_MOUSE_SIDE				,		(int)MOUSE_BUTTON_SIDE					},
	{GameMouse::GAME_MOUSE_EXTRA			,		(int)MOUSE_BUTTON_EXTRA					},
	{GameMouse::GAME_MOUSE_FORWARD			,		(int)MOUSE_BUTTON_FORWARD				},
	{GameMouse::GAME_MOUSE_BACK				,		(int)MOUSE_BUTTON_BACK					}
};

KeyState Input::GetKeyState(GameKey key)
{
	return keyStates[(int)key];
}

KeyState Input::GetMouseState(GameMouse btn)
{
	return mouseStates[(int)btn];
}

MouseInfo Input::GetMouseInfo()
{
	return { GetMousePosition(),
			 GetMouseDelta(),
			 GetMouseWheelMove() };
}

void Input::SetMousePos(Vector2 pos)
{
	SetMousePosition((int)pos.x, (int)pos.y);
}

bool Input::CheckKeyPressed(GameKey key)
{
	return (bool) (keyStates[(int)key] & KeyState::PRESSED);
}

bool Input::CheckKeyHeld(GameKey key)
{
	return (bool) (keyStates[(int)key] & KeyState::HELD);
}

bool Input::CheckKeyReleased(GameKey key)
{
	return (bool) (keyStates[key] & KeyState::RELEASED);
}

bool Input::CheckMousePressed(GameMouse btn)
{
	return (bool)(mouseStates[(int)btn] & KeyState::PRESSED);
}

bool Input::CheckMouseHeld(GameMouse btn)
{
	return (bool)(mouseStates[(int)btn] & KeyState::HELD);
}

bool Input::CheckMouseReleased(GameMouse btn)
{
	return (bool)(mouseStates[(int)btn] & KeyState::RELEASED);
}

void Input::SetKeyState(GameKey key, KeyState state)
{
	keyStates[key] = state;
}

void Input::SetMouseState(GameMouse btn, KeyState state)
{
	mouseStates[btn] = state;
}

std::string Input::GetUnicodeInput()
{
	return std::string(unicodeBuffer, 0, bufferIndex);
}

void Input::UpdateInput()
{
	ZoneScopedC(RandomUniqueColor());

	// Update key states
	KeyState oldState;
	for (int keyCode = 0; keyCode < (int)GAME_KEY_COUNT; keyCode++)
	{
		oldState = keyStates[keyCode];
		if (IsKeyDown(keyMapping[(GameKey)keyCode]))
			keyStates[keyCode] = (oldState == KeyState::NONE)									? KeyState::PRESSED : KeyState::HELD;
		else
			keyStates[keyCode] = (oldState == KeyState::NONE || oldState == KeyState::RELEASED)	? KeyState::NONE : KeyState::RELEASED;
	}

	// Update unicode buffer
	bufferIndex = 0;
	int keyCode;
	while ((keyCode = GetCharPressed()) != 0)
	{
		unicodeBuffer[bufferIndex] = keyCode;
		bufferIndex = (bufferIndex + 1) % (MAX_BUFFER_SIZE+1);
	}

	// Update mouse states
	for (int buttonCode = 0; buttonCode < (int)GAME_MOUSE_COUNT; buttonCode++)
	{
		oldState = mouseStates[buttonCode];
		if (IsMouseButtonDown(mouseMapping[(GameMouse)buttonCode]))
			mouseStates[buttonCode] = (oldState == KeyState::NONE)										? KeyState::PRESSED : KeyState::HELD;
		else
			mouseStates[buttonCode] = (oldState == KeyState::NONE || oldState == KeyState::RELEASED)	? KeyState::NONE : KeyState::RELEASED;
	}
}

std::string Input::GetKeyName(GameKey key)
{
	switch (key)
	{
	case Input::GAME_KEY_A							:		return "KEY_A";
	case Input::GAME_KEY_B							:		return "KEY_B"; 
	case Input::GAME_KEY_C							:		return "KEY_C"; 
	case Input::GAME_KEY_D							:		return "KEY_D"; 
	case Input::GAME_KEY_E							:		return "KEY_E"; 
	case Input::GAME_KEY_F							:		return "KEY_F"; 
	case Input::GAME_KEY_G							:		return "KEY_G"; 
	case Input::GAME_KEY_H							:		return "KEY_H"; 
	case Input::GAME_KEY_I							:		return "KEY_I"; 
	case Input::GAME_KEY_J							:		return "KEY_J"; 
	case Input::GAME_KEY_K							:		return "KEY_K"; 
	case Input::GAME_KEY_L							:		return "KEY_L"; 
	case Input::GAME_KEY_M							:		return "KEY_M"; 
	case Input::GAME_KEY_N							:		return "KEY_N"; 
	case Input::GAME_KEY_O							:		return "KEY_O"; 
	case Input::GAME_KEY_P							:		return "KEY_P"; 
	case Input::GAME_KEY_Q							:		return "KEY_Q"; 
	case Input::GAME_KEY_R							:		return "KEY_R"; 
	case Input::GAME_KEY_S							:		return "KEY_S"; 
	case Input::GAME_KEY_T							:		return "KEY_T"; 
	case Input::GAME_KEY_U							:		return "KEY_U"; 
	case Input::GAME_KEY_V							:		return "KEY_V"; 
	case Input::GAME_KEY_W							:		return "KEY_W"; 
	case Input::GAME_KEY_X							:		return "KEY_X"; 
	case Input::GAME_KEY_Y							:		return "KEY_Y"; 
	case Input::GAME_KEY_Z							:		return "KEY_Z"; 
	case Input::GAME_KEY_0							:		return "KEY_0"; 
	case Input::GAME_KEY_1							:		return "KEY_1"; 
	case Input::GAME_KEY_2							:		return "KEY_2"; 
	case Input::GAME_KEY_3							:		return "KEY_3"; 
	case Input::GAME_KEY_4							:		return "KEY_4"; 
	case Input::GAME_KEY_5							:		return "KEY_5"; 
	case Input::GAME_KEY_6							:		return "KEY_6"; 
	case Input::GAME_KEY_7							:		return "KEY_7"; 
	case Input::GAME_KEY_8							:		return "KEY_8"; 
	case Input::GAME_KEY_9							:		return "KEY_9"; 
	case Input::GAME_KEY_APOSTROPHE					:		return "KEY_APOSTROPHE"; 
	case Input::GAME_KEY_COMMA						:		return "KEY_COMMA"; 
	case Input::GAME_KEY_MINUS						:		return "KEY_MINUS"; 
	case Input::GAME_KEY_PERIOD						:		return "KEY_PERIOD"; 
	case Input::GAME_KEY_SLASH						:		return "KEY_SLASH"; 
	case Input::GAME_KEY_SEMICOLON					:		return "KEY_SEMICOLON"; 
	case Input::GAME_KEY_EQUAL						:		return "KEY_EQUAL"; 
	case Input::GAME_KEY_LEFT_BRACKET				:		return "KEY_LEFT_BRACKET"; 
	case Input::GAME_KEY_RIGHT_BRACKET				:		return "KEY_RIGHT_BRACKER"; 
	case Input::GAME_KEY_BACKSLASH					:		return "KEY_BACKSLASH"; 
	case Input::GAME_KEY_GRAVE						:		return "KEY_GRAVE"; 
	case Input::GAME_KEY_SPACE						:		return "KEY_SPACE"; 
	case Input::GAME_KEY_ESCAPE						:		return "KEY_ESCAPE"; 
	case Input::GAME_KEY_ENTER						:		return "KEY_ENTER"; 
	case Input::GAME_KEY_TAB						:		return "KEY_TAB"; 
	case Input::GAME_KEY_BACKSPACE					:		return "KEY_BACKSPACE"; 
	case Input::GAME_KEY_INSERT						:		return "KEY_INSERT"; 
	case Input::GAME_KEY_DELETE						:		return "KEY_DELETE"; 
	case Input::GAME_KEY_RIGHT						:		return "KEY_RIGHT"; 
	case Input::GAME_KEY_LEFT						:		return "KEY_LEFT"; 
	case Input::GAME_KEY_DOWN						:		return "KEY_DOWN"; 
	case Input::GAME_KEY_UP							:		return "KEY_UP"; 
	case Input::GAME_KEY_PAGE_UP					:		return "KEY_PAGE_UP"; 
	case Input::GAME_KEY_PAGE_DOWN					:		return "KEY_PAGE_DOWN"; 
	case Input::GAME_KEY_HOME						:		return "KEY_HOME"; 
	case Input::GAME_KEY_END						:		return "KEY_END"; 
	case Input::GAME_KEY_CAPS_LOCK					:		return "KEY_CAPS_LOCK"; 
	case Input::GAME_KEY_SCROLL_LOCK				:		return "KEY_SCROLL_LOCK"; 
	case Input::GAME_KEY_PRINT_SCREEN				:		return "KEY_PRINT_SCREEN"; 
	case Input::GAME_KEY_PAUSE						:		return "KEY_PAUSE"; 
	case Input::GAME_KEY_F1							:		return "KEY_F1"; 
	case Input::GAME_KEY_F2							:		return "KEY_F2"; 
	case Input::GAME_KEY_F3							:		return "KEY_F3"; 
	case Input::GAME_KEY_F4							:		return "KEY_F4"; 
	case Input::GAME_KEY_F5							:		return "KEY_F5"; 
	case Input::GAME_KEY_F6							:		return "KEY_F6"; 
	case Input::GAME_KEY_F7							:		return "KEY_F7"; 
	case Input::GAME_KEY_F8							:		return "KEY_F8"; 
	case Input::GAME_KEY_F9							:		return "KEY_F9"; 
	case Input::GAME_KEY_F10						:		return "KEY_F10"; 
	case Input::GAME_KEY_F11						:		return "KEY_F11"; 
	case Input::GAME_KEY_F12						:		return "KEY_F12"; 
	case Input::GAME_KEY_LEFT_SHIFT					:		return "KEY_LEFT_SHIFT"; 
	case Input::GAME_KEY_LEFT_CONTROL				:		return "KEY_LEFT_CONTROL"; 
	case Input::GAME_KEY_LEFT_ALT					:		return "KEY_LEFT_ALT"; 
	case Input::GAME_KEY_LEFT_SUPER					:		return "KEY_LEFT_SUPER"; 
	case Input::GAME_KEY_RIGHT_SHIFT				:		return "KEY_RIGHT_SHIFT"; 
	case Input::GAME_KEY_RIGHT_CONTROL				:		return "KEY_RIGHT_CONTROL"; 
	case Input::GAME_KEY_RIGHT_ALT					:		return "KEY_RIGHT_ALT"; 
	case Input::GAME_KEY_RIGHT_SUPER				:		return "KEY_RIGHT_SUPER"; 
	case Input::GAME_KEY_KB_MENU					:		return "KEY_KB_MENU"; 
	case Input::GAME_KEY_NP_0						:		return "KEY_NP_0"; 
	case Input::GAME_KEY_NP_1						:		return "KEY_NP_1"; 
	case Input::GAME_KEY_NP_2						:		return "KEY_NP_2"; 
	case Input::GAME_KEY_NP_3						:		return "KEY_NP_3"; 
	case Input::GAME_KEY_NP_4						:		return "KEY_NP_4"; 
	case Input::GAME_KEY_NP_5						:		return "KEY_NP_5"; 
	case Input::GAME_KEY_NP_6						:		return "KEY_NP_6"; 
	case Input::GAME_KEY_NP_7						:		return "KEY_NP_7"; 
	case Input::GAME_KEY_NP_8						:		return "KEY_NP_8"; 
	case Input::GAME_KEY_NP_9						:		return "KEY_NP_9"; 
	case Input::GAME_KEY_NP_DECIMAL					:		return "KEY_NP_DECIMAL"; 
	case Input::GAME_KEY_NP_DIVIDE					:		return "KEY_NP_DIVIDE"; 
	case Input::GAME_KEY_NP_MULTIPLY				:		return "KEY_NP_MULTIPLY"; 
	case Input::GAME_KEY_NP_SUBTRACT				:		return "KEY_NP_SUBTRACT"; 
	case Input::GAME_KEY_NP_ADD						:		return "KEY_NP_ADD"; 
	case Input::GAME_KEY_NP_ENTER					:		return "KEY_NP_ENTER"; 
	case Input::GAME_KEY_NP_EQUAL					:		return "KEY_NP_EQUAL"; 

	case Input::GAME_KEY_COUNT						:		return "KEY_ERROR"; 
	default											:		return "KEY_ERROR";
	}
}

std::string Input::GetMouseName(GameMouse btn)
{
	switch (btn)
	{
	case Input::GAME_MOUSE_LEFT						:		return "MOUSE_LEFT";
	case Input::GAME_MOUSE_RIGHT					:		return "MOUSE_RIGHT";
	case Input::GAME_MOUSE_MIDDLE					:		return "MOUSE_MIDDLE";
	case Input::GAME_MOUSE_SIDE						:		return "MOUSE_SIDE";
	case Input::GAME_MOUSE_EXTRA					:		return "MOUSE_EXTRA";
	case Input::GAME_MOUSE_FORWARD					:		return "MOUSE_FORWARD";
	case Input::GAME_MOUSE_BACK						:		return "MOUSE_BACK";

	case Input::GAME_MOUSE_COUNT					:		return "MOUSE_ERROR";
	default											:		return "MOUSE_ERROR";
	}
}
