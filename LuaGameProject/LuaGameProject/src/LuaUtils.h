#pragma once
#include "lua.hpp"

#define TEST_PATH std::string("src/Lua/Tests/")
#define FILE_PATH std::string("src/Lua/")
#define FILE_EXT std::string(".lua")
#define FILE_CMD std::string("f:")

#define LuaFilePath(fileName) (std::string(FILE_PATH + fileName + FILE_EXT).c_str())

#define LuaDoString(str) if (luaL_dostring(L, str) != LUA_OK) LuaDumpError(L);
#define LuaDoFile(str) if (luaL_dofile(L, str) != LUA_OK) LuaDumpError(L);

#define LuaChk(ret) {if (ret != LUA_OK) LuaDumpError(L);}

// Reads file and returns the content as a string
std::string LuaLoadFile(lua_State *L, const char *path);

void LuaDoFileCleaned(lua_State *L, const char *str);

void LuaDumpError(lua_State *L);
void LuaDumpStack(lua_State *L);
void LuaDumpEnv(lua_State *L);
void LuaDumpECS(lua_State *L, const entt::registry &reg);
void LuaDumpTable(lua_State *L, int i);

// Inspiration from: Programming in Lua, Roberto Ierusalimsch
void CallLuaFunction(lua_State * L, const char *functionName, const char *sig, ...);

void LuaRunTests(lua_State *L, const std::string &testDir);
bool LuaRunTest(lua_State *L, const std::string &fullPath, const std::string &testName);
