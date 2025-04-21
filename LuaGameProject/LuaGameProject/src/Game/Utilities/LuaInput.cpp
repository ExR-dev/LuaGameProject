#include "stdafx.h"
#include "LuaInput.h"

#include "LuaUtils.h"

#include "InputHandler.h"

#include <iostream>

/*
	--- CheckKeyHeld ---

	arg1: Key

	ret : boolean value based on key state
*/
static int LuaCheckKeyHeld(lua_State *L)
{
	const int keyValue = lua_tonumber(L, -1);
	lua_pop(L, 1);

	const bool state = Input::CheckKeyHeld((Input::GameKey)keyValue);

	lua_pushboolean(L, state);

	return 1;
}

void BindLuaInput(lua_State *L)
{
	lua_pushcfunction(L, LuaCheckKeyHeld);
	lua_setglobal(L, "CheckKeyHeld");
}
