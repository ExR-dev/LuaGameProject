#pragma once
//#include <lua.h>
#include "lua.hpp"
//#include "dep/raylib-cpp/raylib-cpp.hpp"

class Scene;

namespace LuaGame
{
	struct LuaGame
	{
	public:
		LuaGame() = default;
		LuaGame(lua_State *L, Scene *scene) : L(L), m_scene(scene)
		{ 

		}

		static void lua_opengame(lua_State *L, LuaGame *game);

	private:
		lua_State *L = nullptr;
		Scene *m_scene = nullptr;


		// Arguments: sound (string), volume (float)
		// Returns: none
		static int lua_PlaySound(lua_State *L);

		// Arguments: scale (float)
		// Returns: none
		static int lua_SetTimeScale(lua_State *L);
	};
}