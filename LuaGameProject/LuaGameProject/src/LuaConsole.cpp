#include "LuaConsole.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <format>
#include <filesystem>
#include "LuaUtils.h"

namespace fs = std::filesystem;

#define FILE_EXT std::string(".lua")
#define FILE_PATH std::string("src/Lua/")
#define FILE_CMD std::string("f:")

struct Vector2
{
	float p_x, p_y;
	Vector2(float x = 0.0f, float y = 0.0f) :
		p_x(x), p_y(y) {}
};

Vector2 lua_tovector(lua_State *L, int index)
{
	Vector2 v;

	lua_getfield(L, -1, "x");
	v.p_x = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "y");
	v.p_y = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_pop(L, 1);

	return v;
}

static int PrintVector(lua_State *L)
{
	LuaDumpStack(L);

	Vector2 v = lua_tovector(L, 1);
	std::cout << "(" << v.p_x << ", " << v.p_y << ")" << std::endl;

	LuaDumpStack(L);

	return 0;
}

void ConsoleThreadFunction(lua_State *L)
{
	// Add lua require path
	std::string luaScriptPath = std::format("{}/{}?{}", fs::current_path().generic_string(), FILE_PATH, FILE_EXT);
	LuaDoString(std::format("package.path = \"{};\" .. package.path", luaScriptPath).c_str());

	LuaDoString(std::format(
		"print('To run a \"{}\" file located in \"{}\", begin your command with \"{}\" followed by the file name.')", 
						FILE_EXT,			   FILE_PATH,					   FILE_CMD
	).c_str());

	std::string input;
	
	lua_pushcfunction(L, PrintVector);
	lua_setglobal(L, "PrintVector");

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
