#include "stdafx.h"
#include "LuaGame.h"
#include "../Scene.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

#define lua_GetGameUpValue(L) (LuaGame *)lua_topointer(L, lua_upvalueindex(1))
#define lua_GetGame(L) lua_GetGameUpValue(L)

void LuaGame::LuaGame::lua_opengame(lua_State *L, LuaGame *game)
{
	ZoneScopedC(RandomUniqueColor());

	lua_newtable(L);

	luaL_Reg methods[] = {
	//  { "NameInLua",				NameInCpp				},
		{ "PlaySound",				lua_PlaySound			},
		{ "SetTimeScale",			lua_SetTimeScale		},
		{ "GetMouseWorldPos",		lua_GetMouseWorldPos	},
		{ NULL,						NULL					}
	};

	lua_pushlightuserdata(L, game);
	luaL_setfuncs(L, methods, 1); // 1 : one upvalue (lightuserdata)

	lua_setglobal(L, "game");

	tracy::LuaRegister(L);

	ImLua::ImLua::lua_openimgui(L);

	LuaDoFileCleaned(L, LuaFilePath("Utility/InitData"));

	LuaDoFileCleaned(L, LuaFilePath("Utility/TableUtils"));

#ifdef LUA_DEBUG
	LuaDoFileCleaned(L, LuaFilePath("Dev/Commands"));
#endif
}

int LuaGame::LuaGame::lua_PlaySound(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());

	LuaGame *game = lua_GetGame(L);
	std::string soundName = lua_tostring(L, 1);

	if (soundName == "")
	{
		DbgMsg("No sound name given!");
		return 1;
	}

	raylib::Sound *sound = ResourceManager::Instance().GetSoundResource(soundName);

	if (!sound)
	{
		ResourceManager::Instance().LoadSoundResource(soundName);
		sound = ResourceManager::Instance().GetSoundResource(soundName);
	}

	float volume = 1.0f;
	float pan = 0.5f;
	float pitch = 1.0f;

	// Get optional parameters
	do
	{
		if (!lua_isnumber(L, 2))
			break;
		volume = lua_tonumber(L, 2);

		if (!lua_isnumber(L, 3))
			break;
		pan = lua_tonumber(L, 3);

		if (!lua_isnumber(L, 4))
			break;
		pitch = lua_tonumber(L, 4);

	} while (false);

	sound->SetVolume(volume);
	sound->SetPan(pan);
	sound->SetPitch(pitch);

	sound->Play();
	return 1;
}

int LuaGame::LuaGame::lua_SetTimeScale(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());

	if (lua_isnumber(L, 1))
	{
		float newTimeScale = (float)lua_tonumber(L, 1);
		Game::Game::Instance().TimeScale = newTimeScale; // TODO: Doesn't work, TimeScale is not modified
	}
	else
	{
		luaL_error(L, "Expected a number for TimeScale");
	}

	return 1;
}

int LuaGame::LuaGame::lua_GetMouseWorldPos(lua_State *L)
{
	LuaGame *game = lua_GetGame(L);

	lua_createtable(L, 0, 2);
	lua_pushnumber(L, game->m_mouseWorldPos[0]);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, game->m_mouseWorldPos[1]);
	lua_setfield(L, -2, "y");

	return 1;
}
