#include "stdafx.h"
#include "LuaInput.h"

#include "LuaUtils.h"

#include "InputHandler.h"

#include <iostream>

using namespace Input;

/*
	--- CheckKeyHeld ---

	arg1: Key

	ret : boolean value based on key state
*/
static int LuaCheckKeyHeld(lua_State *L)
{
	const int keyValue = lua_tonumber(L, -1);
	lua_pop(L, 1);

	const bool state = CheckKeyHeld((GameKey)keyValue);

	lua_pushboolean(L, state);

	return 1;
}

/*
	--- CheckKeyPressed ---

	arg1: Key

	ret : boolean value based on key state
*/
static int LuaCheckKeyPressed(lua_State *L)
{
	const int keyValue = lua_tonumber(L, -1);
	lua_pop(L, 1);

	const bool state = CheckKeyPressed((GameKey)keyValue);

	lua_pushboolean(L, state);

	return 1;
}

/*
	--- CheckKeyReleased ---

	arg1: Key

	ret : boolean value based on key state
*/
static int LuaCheckKeyReleased(lua_State *L)
{
	const int keyValue = lua_tonumber(L, -1);
	lua_pop(L, 1);

	const bool state = CheckKeyReleased((GameKey)keyValue);

	lua_pushboolean(L, state);

	return 1;
}



/*
	--- CheckMouseHeld ---

	arg1: Button

	ret : boolean value based on key state
*/
static int LuaCheckMouseHeld(lua_State *L)
{
	const int btnValue = lua_tonumber(L, -1);
	lua_pop(L, 1);

	const bool state = CheckMouseHeld((GameMouse)btnValue);

	lua_pushboolean(L, state);

	return 1;
}

/*
	--- CheckMousePressed ---

	arg1: Button

	ret : boolean value based on key state
*/
static int LuaCheckMousePressed(lua_State *L)
{
	const int btnValue = lua_tonumber(L, -1);
	lua_pop(L, 1);

	const bool state = CheckMousePressed((GameMouse)btnValue);

	lua_pushboolean(L, state);

	return 1;
}

/*
	--- CheckMouseReleased ---

	arg1: Button

	ret : boolean value based on key state
*/
static int LuaCheckMouseReleased(lua_State *L)
{
	const int btnValue = lua_tonumber(L, -1);
	lua_pop(L, 1);

	const bool state = CheckMouseReleased((GameMouse)btnValue);

	lua_pushboolean(L, state);

	return 1;
}

/*
	--- GetMouseInfo ---

	ret : table with the following structure
			{
				position = {
					x = [mouseX],
					y = [mouseY]
				},
				delta = {
					x = [deltaX],
					y = [deltaY]
				},
				scroll = [scroll amount]
			}
*/
static int LuaGetMouseInfo(lua_State *L)
{
	// TODO: Set mouse info as vectors?
	// Get mouse info
	MouseInfo mi = GetMouseInfo();

	lua_createtable(L, 0, 3);

	// Set mouse position
	lua_createtable(L, 0, 2);
	lua_pushnumber(L, mi.position.x);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, mi.position.y);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "Position");

	// Set mouse delta
	lua_createtable(L, 0, 2);
	lua_pushnumber(L, mi.delta.x);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, mi.delta.y);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "Delta");

	// Set mouse scroll
	lua_pushnumber(L, mi.scroll);
	lua_setfield(L, -2, "Scroll");

	return 1;
}

void BindLuaInput(lua_State *L)
{
	const unsigned int nFunctions = 7;

	lua_createtable(L, 0, nFunctions);

	const std::pair<const char*, lua_CFunction> functions[nFunctions]{
		{"KeyHeld"			,	LuaCheckKeyHeld		  },
		{"KeyPressed"		,	LuaCheckKeyPressed	  },
		{"KeyReleased"		,	LuaCheckKeyReleased	  },

		{"MouseHeld"		,	LuaCheckMouseHeld	  },
		{"MousePressed"		,	LuaCheckMousePressed  },
		{"MouseReleased"	,	LuaCheckMouseReleased },
		{"GetMouseInfo"		,	LuaGetMouseInfo		  }
	};

	// Set Input Function
	for (int i = 0; i < nFunctions; i++)
	{
		lua_pushcfunction(L, functions[i].second);
		lua_setfield(L, -2, functions[i].first);
	}

	// Set Input Keys
	lua_createtable(L, 0, GAME_KEY_COUNT);
	for (int key = 0; key < GAME_KEY_COUNT; key++)
	{
		lua_pushnumber(L, key);
		lua_setfield(L, -2, GetKeyName((GameKey)key).c_str());
	}
	lua_setfield(L, -2, "Key");


	// Set Mouse Buttons
	lua_createtable(L, 0, GAME_MOUSE_COUNT);
	for (int btn = 0; btn < GAME_MOUSE_COUNT; btn++)
	{
		lua_pushnumber(L, btn);
		lua_setfield(L, -2, GetMouseName((GameMouse)btn).c_str());
	}
	lua_setfield(L, -2, "Mouse");

	// Set Input Table
	lua_setglobal(L, "Input");
}
