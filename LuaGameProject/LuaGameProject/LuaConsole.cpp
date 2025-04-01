#include "LuaConsole.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <format>
#include "LuaUtils.h"


void ConsoleThreadFunction(lua_State *L)
{
	std::string input;

	while (GetConsoleWindow())
	{
		std::cout << "> ";
		std::getline(std::cin, input);

		if (luaL_dostring(L, input.c_str()) != LUA_OK)
		{
			DumpLuaError(L);
		}
	}
}