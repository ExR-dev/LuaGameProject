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
