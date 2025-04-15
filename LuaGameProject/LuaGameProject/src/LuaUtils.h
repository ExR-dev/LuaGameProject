#pragma once
#include "lua.hpp"

#define LuaDoString(str) if (luaL_dostring(L, str) != LUA_OK) LuaDumpError(L);
#define LuaDoFile(str) if (luaL_dofile(L, str) != LUA_OK) LuaDumpError(L);

void LuaDumpError(lua_State *L);

void LuaDumpStack(lua_State *L);
