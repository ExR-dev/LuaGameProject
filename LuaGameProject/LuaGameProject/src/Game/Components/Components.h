#pragma once
#include <cstring>

namespace ECS
{
	struct Transform
	{
		float _;
	};

	struct Health
	{
		float Value;
	};

	struct Behaviour
	{
		char ScriptPath[64];
		int LuaRef;

		// Create a constructor in order to initialize the char array.
		Behaviour(const char *path, int luaRef) : LuaRef(luaRef)
		{
			memset(ScriptPath, '\0', 64);
			strcpy_s(ScriptPath, path);
		}
	};
}
