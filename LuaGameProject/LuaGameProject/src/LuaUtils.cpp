#include "stdafx.h"
#include "LuaUtils.h"

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
			std::cout << fmt << "¯\\_(ツ)_/¯";
			break;

		case LUA_TNUMBER:
			std::cout << fmt << lua_tonumber(L, i);
			break;

		case LUA_TSTRING:
			std::cout << fmt << std::format("\"{}\"", lua_tostring(L, i)).c_str();
			break;

		case LUA_TTABLE:
			std::cout << fmt << "¯\\_(ツ)_/¯";
			break;

		case LUA_TFUNCTION:
			std::cout << fmt << lua_topointer(L, i);
			break;

		case LUA_TUSERDATA:
			std::cout << fmt << "¯\\_(ツ)_/¯";
			break;

		case LUA_TTHREAD:
			std::cout << fmt << "¯\\_(ツ)_/¯";
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
			break;
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
				TraceLog(LOG_ERROR, "Incorrect return type");
			*va_arg(vl, double*) = n;
			break;
		}
		case 'i': {
			int isnum;
			int n = lua_tointegerx(L, nres, &isnum);
			if (!isnum)
				TraceLog(LOG_ERROR, "Incorrect return type");
			*va_arg(vl, int*) = n;
			break;
		}
		case 's': {
			const char *s = lua_tostring(L, nres);
			if (s == NULL)
				TraceLog(LOG_ERROR, "Incorrect return type");
			*va_arg(vl, const char**) = s;
			break;
		}
		default:
			TraceLog(LOG_ERROR, "Invalid option");
			break;
		}
		nres++;
	}

	va_end(vl);
}
