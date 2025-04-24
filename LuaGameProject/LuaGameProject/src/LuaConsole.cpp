#include "LuaConsole.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <format>
#include <filesystem>
#include <chrono>
#include <thread>
#include "LuaUtils.h"

#include "Game/Game.h"
#include "Game/Utilities/LuaInput.h"

#define WINDOWS_DEF
#include <dep/tracy-0.11.1/public/tracy/Tracy.hpp>
#include <dep/tracy-0.11.1/public/tracy/TracyLua.hpp>
#undef WINDOWS_DEF

namespace fs = std::filesystem;

struct Vector2
{
	float p_x, p_y;
	Vector2(float x = 0.0f, float y = 0.0f) :
		p_x(x), p_y(y) {}
};

static Vector2 lua_tovector(lua_State *L, int index)
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

	std::cout << std::endl << std::format(
		"To run a \"{}\" file located in \"{}\", begin your command with \"{}\" followed by the file name.", 
				 FILE_EXT,			    FILE_PATH,					    FILE_CMD
	) << std::endl;
	std::cout << "To run all tests located in the tests folder, type \"[T/t]est\"." << std::endl;
	std::cout << "To dump the lua stack, type \"DumpStack\"." << std::endl;
	std::cout << "To dump the lua environment, type \"DumpEnv\"." << std::endl;

	std::string input;
	
	lua_pushcfunction(L, PrintVector);
	lua_setglobal(L, "PrintVector");

	BindLuaInput(L);

	while (GetConsoleWindow())
	{
		std::cout << "> ";
		std::getline(std::cin, input);

		while (Game::UpdateConsole)

		if (input == "Test" || input == "test") // Run all tests
		{
			LuaRunTests(L, TEST_PATH);
		}
		else if (input == "DumpStack")
		{
			LuaDumpStack(L);
		}
		else if (input == "DumpEnv")
		{
			LuaDumpEnv(L);
		}
		else if (input.starts_with(FILE_CMD)) // File command
		{
			input = input.substr(FILE_CMD.size());
			LuaDoFileCleaned(L, LuaFilePath(input));
		}
		else // String command
		{
			LuaDoString(input.c_str());
		}

		lua_getglobal(L, "UpdateInput");
		lua_pcall(L, 0, 0, 0);

		std::cout << std::endl;
	}

	lua_close(L);
}
