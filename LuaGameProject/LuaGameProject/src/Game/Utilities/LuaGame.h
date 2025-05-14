#pragma once
#include "lua.hpp"

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

		void SetMouseWorldPos(float x, float y)
		{
			m_mouseWorldPos[0] = x;
			m_mouseWorldPos[1] = y;
		}

		static void lua_opengame(lua_State *L, LuaGame *game);

	private:
		lua_State *L = nullptr;
		Scene *m_scene = nullptr;

		float m_mouseWorldPos[2]{ 0.0f, 0.0f };


		// Arguments: sound (string), volume (float)
		// Returns: none
		static int lua_PlaySound(lua_State *L);

		// Arguments: scale (float)
		// Returns: none
		static int lua_SetTimeScale(lua_State *L);

		// Arguments: none
		// Returns: table(x, y)
		static int lua_GetMouseWorldPos(lua_State *L);
	};
}