#include "stdafx.h"
#include "LuaConsole.h"
#include "Game/Utilities/WindowsWrapped.h"
#include <iostream>
#include <string>
#include <format>
#include <filesystem>
#include <chrono>
#include <thread>
#include "LuaUtils.h"

#include "Game/Game.h"
#include "Game/Tools/ErrMsg.h"
#include "Game/Utilities/LuaInput.h"

#define WINDOWS_DEF
#include <dep/tracy-0.11.1/public/tracy/Tracy.hpp>
#include <dep/tracy-0.11.1/public/tracy/TracyLua.hpp>
#undef WINDOWS_DEF

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

namespace fs = std::filesystem;

struct CVector2
{
	float p_x, p_y;
	CVector2(float x = 0.0f, float y = 0.0f) :
		p_x(x), p_y(y) {}
};

static CVector2 lua_tovector(lua_State *L, int index)
{
	CVector2 v;

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

	CVector2 v = lua_tovector(L, 1);
	std::cout << "(" << v.p_x << ", " << v.p_y << ")" << std::endl;

	LuaDumpStack(L);

	return 0;
}

void ConsoleThreadFunction(CmdState *cmdState)
{
	std::string input;

	while (Windows::GetConsoleWindowW())
	{
		// Wait for the command list to be empty
		while (cmdState->pauseCmdInput.load())
			std::this_thread::sleep_for(std::chrono::milliseconds(50));

		std::cout << "> ";
		std::getline(std::cin, input);

		if (input != "")
		{
			cmdState->cmdInput = input;

			// Pause console input until the command list is executed
			cmdState->pauseCmdInput.store(true);
		}
	}
}

void ExecuteCommandList(lua_State *L, CmdState *cmdState, const entt::registry &reg)
{
	if (!cmdState->pauseCmdInput.load())
		return;

	// Execute the command list
	std::string &input = cmdState->cmdInput;

	if (GameMath::EqualsAny(input, "test", "Test")) // Run all tests
	{
		LuaRunTests(L, TEST_PATH);
	}
#ifdef LUA_DEBUG
	else if (input.starts_with("Step") || input.starts_with("step"))
	{
		if (Game::Game::Instance().CmdStepMode)
		{
			// See if a number is provided
			if (input.size() > sizeof("step"))
			{
				int steps = std::stoi(input.substr(sizeof("step")));

				if (steps > 0)
				{
					Game::Game::Instance().CmdTakeSteps += steps;
				}
				else
				{
					Warn(std::format("Invalid number of steps: {}", steps));
				}
			}
			else
			{
				// Default to 1 step
				Game::Game::Instance().CmdTakeSteps++;
			}
		}
	}
	else if (GameMath::EqualsAny(input, "Break", "break"))
	{
		Game::Game::Instance().CmdStepMode = true;
	}
	else if (GameMath::EqualsAny(input, "Continue", "continue"))
	{
		Game::Game::Instance().CmdStepMode = false;
	}
#endif
	else if (input.starts_with(FILE_CMD)) // File command
	{
		input = input.substr(FILE_CMD.size());
		LuaDoFileCleaned(L, LuaFilePath(input));
	}
	else if (input == "DumpStack")
	{
		LuaDumpStack(L);
	}
	else if (input == "DumpEnv")
	{
		LuaDumpEnv(L);
	}
	else if (input == "DumpECS")
	{
		LuaDumpECS(L, reg);
	}
	else if (GameMath::EqualsAny(input, "help", "Help"))
	{

		std::cout << std::endl << std::format(
			"To run a \"{}\" file located in \"{}\", begin your command with \"{}\" followed by the file name.",
			LUA_EXT, FILE_PATH, FILE_CMD
		) << std::endl;
		std::cout << std::endl;

		std::cout << "To run all tests, type \"[T/t]est\"." << std::endl;
		std::cout << "To enable debug stepping, type \"[B/b]reak\"." << std::endl;
		std::cout << "To disable debug stepping, type \"[C/c]ontinue\"." << std::endl;
		std::cout << "In debug stepping, step 1 or X frame(s) using \"[S/s]tep <opt. X>\"." << std::endl;
		std::cout << std::endl;

		std::cout << "To dump the Lua stack, use \"DumpStack\"." << std::endl;
		std::cout << "To dump the Lua environment, use \"DumpEnv\"." << std::endl;
		std::cout << "To dump the EnTT registry, use \"DumpECS\"." << std::endl;
		std::cout << std::endl;

		std::cout << "Anything not following a format listed above will execute as Lua code." << std::endl;
		std::cout << std::endl;

		std::cout << "To see the full contents of a Lua table, use \"PrintTable([table])\"." << std::endl;
		std::cout << "Tip: Try using \"PrintTable()\" on the output from \"DumpEnv\"." << std::endl;
		std::cout << std::endl;

		std::cout << "Notes:" << std::endl;
		std::cout << "[x/y] \tboth x and y works" << std::endl;
		std::cout << "<x>   \tparameter" << std::endl;
		std::cout << "opt.  \tinput is optional" << std::endl;
	}
	else // String command
	{
		if (!LuaDoString(L, input.c_str()))
			std::cout << "Type help for a list of commands." << std::endl;
	}

	std::cout << std::endl;
	cmdState->cmdInput = "";
	
	cmdState->pauseCmdInput.store(false); // Allow console input again
}
