#pragma once
#include "lua.hpp"

#define LuaDoString(str) if (luaL_dostring(L, str) != LUA_OK) LuaDumpError(L);
#define LuaDoFile(str) if (luaL_dofile(L, str) != LUA_OK) LuaDumpError(L);

#define LuaChk(ret) {if (ret != LUA_OK) LuaDumpError(L);}

void LuaDumpError(lua_State *L);
void LuaDumpStack(lua_State *L);

void LuaDumpStack(lua_State *L);

// Inspiration from: Programming in Lua, Roberto Ierusalimsch
void CallLuaFunction(lua_State * L, const char *functionName, const char *sig, ...);

void LuaRunTests(lua_State *L, const std::string &testDir);
bool LuaRunTest(lua_State *L, const std::string &fullPath, const std::string &testName);
