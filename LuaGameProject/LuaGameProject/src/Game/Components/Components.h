#pragma once
#include <cstring>

namespace ECS
{
	struct Behaviour
	{
		static const int SCRIPT_PATH_LENGTH = 64;
		char ScriptPath[SCRIPT_PATH_LENGTH];
		int LuaRef;

		// Create a constructor in order to initialize the char array.
		Behaviour(const char *path, int luaRef) : LuaRef(luaRef)
		{
			memset(ScriptPath, '\0', SCRIPT_PATH_LENGTH);
			strcpy_s(ScriptPath, path);
		}
	};

	struct Transform
	{
		float Position[3];
		float Rotation[3];
		float Scale[3];
	};

	struct Sprite
	{
		static const int SPRITE_NAME_LENGTH = 64;
		char SpriteName[SPRITE_NAME_LENGTH];

		Sprite(const char *name)
		{
			memset(SpriteName, '\0', SPRITE_NAME_LENGTH);
			strcpy_s(SpriteName, name);
		}
	};

	struct Health
	{
		float Value;
	};
}
