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

void BindLuaInput(lua_State *L)
{
	const unsigned int nTableElements = GAME_KEY_COUNT + 3;

	lua_createtable(L, 0, nTableElements);

	// Set Input Function
	lua_pushcfunction(L, LuaCheckKeyHeld);
	lua_setfield(L, -2, "KeyHeld");

	lua_pushcfunction(L, LuaCheckKeyPressed);
	lua_setfield(L, -2, "KeyPressed");

	lua_pushcfunction(L, LuaCheckKeyReleased);
	lua_setfield(L, -2, "KeyReleased");

	// Set Input Keys
	for (int key = 0; key < GAME_KEY_COUNT; key++)
	{
		lua_pushnumber(L, key);
		lua_setfield(L, -2, GetKeyName((GameKey)key).c_str());
	}

	lua_setglobal(L, "Input");
}
