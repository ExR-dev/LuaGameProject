#include "stdafx.h"
#include "LuaUtils.h"

void DumpLuaError(lua_State *L)
{
	if (lua_gettop(L) && lua_isstring(L, -1))
	{
		std::cout << std::format("Lua Error: {}", lua_tostring(L, -1)) << std::endl;
		lua_pop(L, 1);
	}
}

std::string GetValueString(lua_State *L, int i)
{
	switch (lua_type(L, i))
	{
	case LUA_TNIL:			return "nil";
	case LUA_TBOOLEAN:		return lua_toboolean(L, i) ? "true" : "false";
	case LUA_TNUMBER:		return std::to_string(lua_tonumber(L, i));
	case LUA_TSTRING:		return lua_tostring(L, i);
	default:				return "";
	}
}

void DumpStack(lua_State *L)
{
	int size = lua_gettop(L);
	std::cout << "--- STACK BEGIN ---" << std::endl;
	for (int i = size; i > 0; i--)
		std::cout << i
				  << "\t"
				  << lua_typename(L, lua_type(L, i))
				  << "\t\t"
				  << GetValueString(L, i)
				  << std::endl;
	std::cout << "---- STACK END ----" << std::endl;
}
