#pragma once
#include "lua.hpp"

#define LuaDoString(str) if (luaL_dostring(L, str) != LUA_OK) DumpLuaError(L);
#define LuaDoFile(str) if (luaL_dofile(L, str) != LUA_OK) DumpLuaError(L);

void DumpLuaError(lua_State *L);
