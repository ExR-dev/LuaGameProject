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
		float Position[2];
		float Rotation;
		float Scale[2];

		void LuaPush(lua_State *L) const
		{
			// Create the main transform table
			lua_createtable(L, 0, 3);  // Create a new table for Transform

			lua_createtable(L, 0, 2);  // Create a new table for Position
			lua_pushnumber(L, Position[0]);
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, Position[1]);
			lua_setfield(L, -2, "y");
			lua_setfield(L, -2, "position");  // Add Position to main table

			// Create and push the Rotation
			lua_pushnumber(L, Rotation);
			lua_setfield(L, -2, "rotation");  // Add Rotation to main table

			// Create and push the Scale subtable
			lua_createtable(L, 0, 2);  // Create a new table for Scale
			lua_pushnumber(L, Scale[0]);
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, Scale[1]);
			lua_setfield(L, -2, "y");
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
			}
			lua_pop(L, 1); // Pop Position table

			// Get Rotation
			lua_getfield(L, index, "rotation");
			Rotation = (float)lua_tonumber(L, -1);
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
			}
			lua_pop(L, 1); // Pop Scale table
		}
	};

	struct Sprite
	{
		static const int SPRITE_NAME_LENGTH = 64;
		char SpriteName[SPRITE_NAME_LENGTH];
		float Color[4];
		int Priority;

		Sprite() : Priority(0)
		{
			memset(Color, 1.0f, sizeof(float) * 4);

			memset(SpriteName, '\0', SPRITE_NAME_LENGTH);
			strcpy_s(SpriteName, "Fallback.png");
		}
		Sprite(const char *name, const float color[4], const int priority) : Priority(priority)
		{
			memcpy(Color, color, sizeof(float) * 4);

			memset(SpriteName, '\0', SPRITE_NAME_LENGTH);
			strcpy_s(SpriteName, name);
		}

		// Comparison operators for sorting draw order
		bool operator==(const Sprite &other) const
		{
			return (Priority == other.Priority);
		}
		bool operator!=(const Sprite &other) const
		{
			return !(*this == other);
		}
		bool operator<(const Sprite &other) const
		{
			return (Priority < other.Priority);
		}
		bool operator>(const Sprite &other) const
		{
			return (Priority > other.Priority);
		}
		bool operator<=(const Sprite &other) const
		{
			return (Priority <= other.Priority);
		}
		bool operator>=(const Sprite &other) const
		{
			return (Priority >= other.Priority);
		}


		void LuaPush(lua_State* L) const
		{
			// Create the main sprite table
			lua_createtable(L, 0, 2);

			// Add spriteName to the sprite table
			lua_pushstring(L, SpriteName);
			lua_setfield(L, -2, "spriteName");
			
			// Create a new table for color
			lua_createtable(L, 0, 4);  
			lua_pushnumber(L, Color[0]);
			lua_setfield(L, -2, "r");
			lua_pushnumber(L, Color[1]);
			lua_setfield(L, -2, "g");
			lua_pushnumber(L, Color[2]);
			lua_setfield(L, -2, "b");
			lua_pushnumber(L, Color[3]);
			lua_setfield(L, -2, "a");
			lua_setfield(L, -2, "color");  // Add color to the sprite table

			// Add priority to the sprite table
			lua_pushnumber(L, Priority);
			lua_setfield(L, -2, "priority");
		}
		void LuaPull(lua_State* L, int index)
		{
			// Make sure the index is absolute (in case it's negative)
			if (index < 0) {
				index = lua_gettop(L) + index + 1;
			}

			// Verify that the value at the given index is a table
			if (!lua_istable(L, index)) {
				luaL_error(L, "Expected a table for Sprite");
				return;
			}

			// Get spriteName field
			lua_getfield(L, index, "spriteName");
			if (lua_isstring(L, -1)) {
				const char* name = lua_tostring(L, -1);
				memset(SpriteName, '\0', SPRITE_NAME_LENGTH);
				strncpy_s(SpriteName, name, SPRITE_NAME_LENGTH - 1);
			}
			lua_pop(L, 1); // Remove the spriteName value from stack

			// Get color table
			lua_getfield(L, index, "color");
			if (lua_istable(L, -1)) {
				// Get r component
				lua_getfield(L, -1, "r");
				if (lua_isnumber(L, -1)) {
					Color[0] = (float)lua_tonumber(L, -1);
				}
				lua_pop(L, 1);

				// Get g component
				lua_getfield(L, -1, "g");
				if (lua_isnumber(L, -1)) {
					Color[1] = (float)lua_tonumber(L, -1);
				}
				lua_pop(L, 1);

				// Get b component
				lua_getfield(L, -1, "b");
				if (lua_isnumber(L, -1)) {
					Color[2] = (float)lua_tonumber(L, -1);
				}
				lua_pop(L, 1);

				// Get a component
				lua_getfield(L, -1, "a");
				if (lua_isnumber(L, -1)) {
					Color[3] = (float)lua_tonumber(L, -1);
				}
				lua_pop(L, 1);
			}
			lua_pop(L, 1); // Remove the color table from stack

			// Get priority field
			lua_getfield(L, index, "priority");
			if (lua_isnumber(L, -1)) {
				Priority = (int)lua_tonumber(L, -1);
			}
			lua_pop(L, 1); // Remove the priority value from stack
		}
	};

	struct Health
	{
		float Current;
		float Max;

		void LuaPush(lua_State* L) const
		{
			// Create the main health table
			lua_createtable(L, 0, 2);

			// Add Current and Max to the health table
			lua_pushnumber(L, Current);
			lua_setfield(L, -2, "current");

			lua_pushnumber(L, Max);
			lua_setfield(L, -2, "max");
		}
		void LuaPull(lua_State* L, int index)
		{
			// Make sure the index is absolute (in case it's negative)
			if (index < 0)
			{
				index = lua_gettop(L) + index + 1;
			}

			// Verify that the value at the given index is a table
			if (!lua_istable(L, index))
			{
				luaL_error(L, "Expected a table for Health");
				return;
			}

			// Get Current field
			lua_getfield(L, index, "current");
			if (lua_isnumber(L, -1))
			{
				Current = (float)lua_tonumber(L, -1);
			}
			lua_pop(L, 1); // Remove the current value from stack

			// Get Max field
			lua_getfield(L, index, "max");
			if (lua_isnumber(L, -1))
			{
				Max = (float)lua_tonumber(L, -1);
			}
			lua_pop(L, 1); // Remove the max value from stack
		}
	};
}
