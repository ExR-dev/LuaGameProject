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

		void LuaPush(lua_State* L)
		{
			// Create the main transform table
			lua_createtable(L, 0, 3);  // Create a new table for Transform

			lua_createtable(L, 0, 3);  // Create a new table for Position
			lua_pushnumber(L, Position[0]);
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, Position[1]);
			lua_setfield(L, -2, "y");
			lua_pushnumber(L, Position[2]);
			lua_setfield(L, -2, "z");
			lua_setfield(L, -2, "position");  // Add Position to main table

			// Create and push the Rotation subtable
			lua_createtable(L, 0, 3);  // Create a new table for Rotation
			lua_pushnumber(L, Rotation[0]);
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, Rotation[1]);
			lua_setfield(L, -2, "y");
			lua_pushnumber(L, Rotation[2]);
			lua_setfield(L, -2, "z");
			lua_setfield(L, -2, "rotation");  // Add Rotation to main table

			// Create and push the Scale subtable
			lua_createtable(L, 0, 3);  // Create a new table for Scale
			lua_pushnumber(L, Scale[0]);
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, Scale[1]);
			lua_setfield(L, -2, "y");
			lua_pushnumber(L, Scale[2]);
			lua_setfield(L, -2, "z");
			lua_setfield(L, -2, "scale");  // Add Scale to main table

			// The main transform table is now at the top of the stack
		}

		void LuaPull(lua_State* L, int index)
		{
			// Make sure the index is absolute
			if (index < 0)
			{
				index = lua_gettop(L) + index + 1;
			}

			// Get Position subtable
			lua_getfield(L, index, "position");
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "x");
				Position[0] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, -1, "y");
				Position[1] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, -1, "z");
				Position[2] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);
			}
			lua_pop(L, 1); // Pop Position table

			// Get Rotation subtable
			lua_getfield(L, index, "rotation");
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "x");
				Rotation[0] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, -1, "y");
				Rotation[1] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, -1, "z");
				Rotation[2] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);
			}
			lua_pop(L, 1); // Pop Rotation table

			// Get Scale subtable
			lua_getfield(L, index, "scale");
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "x");
				Scale[0] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, -1, "y");
				Scale[1] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, -1, "z");
				Scale[2] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);
			}
			lua_pop(L, 1); // Pop Scale table
		}
	};

	struct Sprite
	{
		static const int SPRITE_NAME_LENGTH = 64;
		char SpriteName[SPRITE_NAME_LENGTH];
		float Color[4];

		Sprite(const char *name, const float color[4])
		{
			memcpy(Color, color, sizeof(float) * 4);

			memset(SpriteName, '\0', SPRITE_NAME_LENGTH);
			strcpy_s(SpriteName, name);
		}

		void LuaPush(lua_State* L)
		{
			// TODO: Implement this function
		}

		void LuaPull(lua_State* L, int index)
		{
			// TODO: Implement this function
		}
	};

	struct Health
	{
		float Value;
	};

	struct Camera
	{
		int width;
		int height;
	};
}
