#include "LuaConsole.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <format>
#include "LuaUtils.h"

#define FILE_EXT std::string(".lua")
#define FILE_PATH std::string("src/Lua/")
#define FILE_CMD std::string("f:")

void ConsoleThreadFunction(lua_State *L)
{
	LuaDoString(std::format(
		"print('To run a \"{}\" file located in \"{}\", begin your command with \"{}\" followed by the file name.')", 
						FILE_EXT,			   FILE_PATH,					   FILE_CMD
	).c_str());

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
