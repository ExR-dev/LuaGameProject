#pragma once
#include "lua.hpp"

#define TEST_PATH std::string("src/Lua/Tests/")
#define DATA_PATH std::string("src/Lua/Data/")
#define MOD_PATH std::string("src/Mods/")
#define FILE_PATH std::string("src/Lua/")
#define LUA_EXT std::string(".lua")
#define LTS_EXT std::string(".lts")
#define FILE_CMD std::string("f:")

#define LuaFilePath(fileName) (std::string(FILE_PATH + fileName + LUA_EXT).c_str())
#define LuaModPath(fileName) (std::string(MOD_PATH + fileName + LUA_EXT).c_str())

#define LuaDoString(L, str) ((luaL_dostring(L, str) != LUA_OK) ? LuaDumpError(L) : true)
#define LuaDoFile(L, str) ((luaL_dofile(L, str) != LUA_OK) ? LuaDumpError(L) : true)

#define LuaChk(L, ret) ((ret != LUA_OK) ? LuaDumpError(L) : true)

// Reads file and returns the content as a string
std::string LuaLoadFile(lua_State *L, const char *path);

bool LuaDoFileCleaned(lua_State *L, const char *str);

bool LuaDumpError(lua_State *L);
void LuaDumpStack(lua_State *L);
void LuaDumpEnv(lua_State *L);
void LuaDumpECS(lua_State *L, const entt::registry &reg);
void LuaDumpTable(lua_State *L, int i);

// Inspiration from: Programming in Lua, Roberto Ierusalimsch
void CallLuaFunction(lua_State * L, const char *functionName, const char *sig, ...);

void LuaRunTests(lua_State *L, const std::string &testDir);
bool LuaRunTest(lua_State *L, const std::string &fullPath, const std::string &testName);
