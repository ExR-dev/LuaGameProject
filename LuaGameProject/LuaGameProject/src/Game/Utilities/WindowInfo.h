#pragma once

#include "lua.hpp"

struct WindowInfo
{
    int p_screenWidth = 1280;
    int p_screenHeight = 720;

    inline void BindLuaWindow(lua_State *L) const
    {
	    lua_createtable(L, 0, 2);

		lua_pushnumber(L, p_screenWidth);
		lua_setfield(L, -2, "width");

		lua_pushnumber(L, p_screenHeight);
		lua_setfield(L, -2, "height");

	    lua_setglobal(L, "Window");
    }

	inline void UpdateWindowSize(int width, int height)
	{
		p_screenWidth = width;
		p_screenHeight = height;
	}
};
