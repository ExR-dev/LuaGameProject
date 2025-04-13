#include "LuaConsole.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <format>
#include "LuaUtils.h"

#define FILE_CMD std::string("F:")
#define FILE_PATH std::string("src\\Lua\\")
#define FILE_EXT std::string(".lua")

void ConsoleThreadFunction(lua_State *L)
{
	std::string input;

	while (GetConsoleWindow())
	{
		std::cout << "> ";
		std::getline(std::cin, input);

		if (input.starts_with(FILE_CMD)) // File command
		{
			input = input.substr(FILE_CMD.size());
			input = FILE_PATH + input + FILE_EXT;

			LuaDoFile(input.c_str());
		}
		else // String command
		{
			LuaDoString(input.c_str());
		}

		std::cout << std::endl;
	}
}
