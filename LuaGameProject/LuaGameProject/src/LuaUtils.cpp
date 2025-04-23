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

void LuaDumpTable(lua_State *L, int i)
{
	int type = lua_type(L, i);

	std::cout << std::format("({}, {}) {}:", i, i-6, lua_typename(L, type)) << std::endl;

	if (type == LUA_TTABLE)
	{
		LuaDoFile(LuaFilePath("PrintTable"));
		// Run the global lua function PrintTable(table)
		lua_pushvalue(L, i); // Push the table to the top of the stack
		lua_getglobal(L, "PrintTable");
		lua_pushvalue(L, -2); // Push the table again as an argument
		LuaChk(lua_pcall(L, 1, 0, 0)); // Call the function with 1 argument and no return values
		lua_pop(L, 1); // Pop the table from the stack
	}

	std::cout << std::endl;
}

void CallLuaFunction(lua_State *L, const char *functionName, const char *sig, ...)
{
	va_list vl;
	int narg, nres;

	va_start(vl, sig);
	lua_getglobal(L, functionName);

	// Push arguments
	for (narg = 0; *sig; narg++)
	{
		luaL_checkstack(L, 1, "too many arguments");

		switch (*sig++)
		{
		case 'd':
			lua_pushnumber(L, va_arg(vl, double));
			break;
		case 'i':
			lua_pushinteger(L, va_arg(vl, int));
			break;
		case 's':
			lua_pushstring(L, va_arg(vl, char *));
			break;
		case '>':
			goto endargs;
			break;
		default:
			TraceLog(LOG_ERROR, "Invalid option");
			return;
		}
	}
	endargs:

	nres = strlen(sig);

	if (lua_pcall(L, narg, nres, 0) != LUA_OK)
		TraceLog(LOG_ERROR, "Error calling function");

	// Retrive results
	nres = -nres;
	while (*sig)
	{
		switch (*sig++)
		{
		case 'd': {
			int isnum;
			double n = lua_tonumberx(L, nres, &isnum);
			if (!isnum)
			{
				TraceLog(LOG_ERROR, "Incorrect return type");
				return;
			}
			*va_arg(vl, double*) = n;
			break;
		}
		case 'i': {
			int isnum;
			int n = lua_tointegerx(L, nres, &isnum);
			if (!isnum)
			{
				TraceLog(LOG_ERROR, "Incorrect return type");
				return;
			}
			*va_arg(vl, int*) = n;
			break;
		}
		case 's': {
			const char *s = lua_tostring(L, nres);
			if (s == NULL)
			{
				TraceLog(LOG_ERROR, "Incorrect return type");
				return;
			}
			*va_arg(vl, const char**) = s;
			break;
		}
		default:
			TraceLog(LOG_ERROR, "Invalid option");
			return;
		}
		nres++;
	}

	va_end(vl);
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
		LuaRunTest(L, testScript, t);
	}
}

bool LuaRunTest(lua_State *L, const std::string &fullPath, const std::string &testName)
{
	std::cout << "\n================================================================================\n";
	std::cout << std::format("================ Running {}\n\n", testName);

	if (luaL_dofile(L, fullPath.c_str()) != LUA_OK) 
	{
		if (lua_gettop(L) && lua_isstring(L, -1))
		{
			std::cout << "Test Failed with the Error:\n" << lua_tostring(L, -1) << "\n";
			lua_pop(L, 1);
			std::cout << "================================================================================\n";
			return false;
		}
	}

	int passedTests = static_cast<int>(lua_tointeger(L, -1));
	int totalTests = static_cast<int>(lua_tointeger(L, -2));

	std::cout << std::format("================ Tests Passed: {} / {}\n", passedTests, totalTests);
	std::cout << "================================================================================\n";
	return passedTests == totalTests;
}