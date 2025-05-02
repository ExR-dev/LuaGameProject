#include "stdafx.h"
#include "LuaGame.h"
#include "../Scene.h"

#define lua_GetGameUpValue(L) (LuaGame *)lua_topointer(L, lua_upvalueindex(1))
#define lua_GetGame(L) lua_GetGameUpValue(L)

void LuaGame::LuaGame::lua_opengame(lua_State *L, LuaGame *game)
{
	ZoneScopedC(RandomUniqueColor());

	lua_newtable(L);

	luaL_Reg methods[] = {
	//  { "NameInLua",			NameInCpp			},
		{ "PlaySound",			lua_PlaySound		},
		{ "SetTimeScale",		lua_SetTimeScale	},
		{ NULL,					NULL				}
	};

	lua_pushlightuserdata(L, game);
	luaL_setfuncs(L, methods, 1); // 1 : one upvalue (lightuserdata)

	lua_setglobal(L, "game");

	tracy::LuaRegister(L);
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

	/*
	std::string path = ResourceManager::Instance().GetSoundResourcePath(soundName);
	raylib::Sound sound;

	try
	{
		sound = raylib::Sound(path);
	}
	catch (const raylib::RaylibException &)
	{
		DbgMsg(std::format("Sound '{}' not found!", soundName).c_str());
		return 1;
	}
	*/

	raylib::Sound *sound = ResourceManager::Instance().GetSoundResource(soundName);

	if (!sound)
	{
		ResourceManager::Instance().LoadSoundResource(soundName);
		sound = ResourceManager::Instance().GetSoundResource(soundName);
	}

	float volume = lua_tonumber(L, 2);

	sound->SetVolume(volume);
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
