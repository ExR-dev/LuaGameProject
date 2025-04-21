#include "stdafx.h"
#include "LuaUtils.h"
#include <fstream>
#include <iostream>
#include <filesystem>

void LuaDumpError(lua_State *L)
{
	if (lua_gettop(L) && lua_isstring(L, -1))
	{
		std::cout << std::format("Lua Error: {}", lua_tostring(L, -1)) << std::endl;
		lua_pop(L, 1);
	}
}

void LuaDumpStack(lua_State *L)
{
	const char separator = ' ';
	const int indexWidth = 2;
	const int nameWidth = 8;
	const int valWidth = 16;

	std::cout << "------------ STACK BEGIN ------------" << std::endl;
	int top = lua_gettop(L);
	for (int i = top; i > 0; --i)
	{
		int type = lua_type(L, i);

		std::cout << std::left << std::setw(indexWidth) << std::setfill(separator) << i;
		std::cout << " | ";
		std::cout << std::left << std::setw(nameWidth) << std::setfill(separator) << lua_typename(L, type);
		#define fmt  std::right << std::setw(valWidth) << std::setfill(separator)
		switch (type)
		{
		case LUA_TNIL:
			std::cout << fmt << "nil";
			break;

		case LUA_TBOOLEAN:
			std::cout << fmt << (lua_toboolean(L, i) ? "true" : "false");
			break;

		case LUA_TLIGHTUSERDATA:
			std::cout << fmt << lua_topointer(L, i);
			break;

		case LUA_TNUMBER:
			std::cout << fmt << lua_tonumber(L, i);
			break;

		case LUA_TSTRING:
			std::cout << fmt << std::format("\"{}\"", lua_tostring(L, i)).c_str();
			break;

		case LUA_TTABLE:
			std::cout << fmt << "Unsupported";
			break;

		case LUA_TFUNCTION:
			std::cout << fmt << lua_topointer(L, i);
			break;

		case LUA_TUSERDATA:
			std::cout << fmt << "Unsupported";
			break;

		case LUA_TTHREAD:
			std::cout << fmt << "Unsupported";
			break;

		default:
			std::cout << fmt << "ERROR";
			break;
		}
		std::cout << " | ";
		std::cout << std::right << std::setw(indexWidth) << std::setfill(separator) << i - 6;
		std::cout << std::endl;
	}
	std::cout << "------------- STACK END -------------" << std::endl;
}

void LuaRunTests(lua_State *L, const std::string &testDir)
{
	std::string ext(".lua");

	std::vector<std::string> testFiles;

	for (auto &p : std::filesystem::recursive_directory_iterator(testDir))
	{
		if (p.path().extension() == ext)
		{
			testFiles.push_back(p.path().stem().string());
		}
	}

	for (auto &t : testFiles)
	{
		const std::string testScript(testDir + t + ".lua");

		std::cout << "\n================================================================================\n";
		std::cout << std::format("================ Running {}\n\n", t);

		if (luaL_dofile(L, testScript.c_str()) != LUA_OK) 
		{
			if (lua_gettop(L) && lua_isstring(L, -1))
			{
				std::cout << "Test Failed with the Error:\n" << lua_tostring(L, -1) << "\n";
				lua_pop(L, 1);
				std::cout << "================================================================================\n";
				continue;
			}
		}

		int passedTests = static_cast<int>(lua_tointeger(L, -1));
		int totalTests = static_cast<int>(lua_tointeger(L, -2));

		std::cout << std::format("======== Tests Passed: {} / {}\n", passedTests, totalTests);
		std::cout << "================================================================================\n";
	}
}
