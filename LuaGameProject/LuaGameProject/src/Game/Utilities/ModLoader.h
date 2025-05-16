#pragma once
#include <lua.hpp>
#include <string>

namespace ModLoader
{
	void LuaLoadData(lua_State *L, const std::string &dataDir);
	void LuaLoadMods(lua_State *L, const std::string &modsDir);
	bool LuaLoadMod(lua_State *L, const std::string &fullPath);
}
