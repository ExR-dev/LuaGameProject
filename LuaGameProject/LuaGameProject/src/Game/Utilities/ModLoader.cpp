#include "stdafx.h"
#include "ModLoader.h"

namespace ModLoader
{
	void LuaLoadData(lua_State *L, const std::string &dataDir)
	{
		ZoneScopedC(RandomUniqueColor());

		// Ensure the data directory exists
		if (!std::filesystem::exists(dataDir))
		{
			ErrMsg("Data directory does not exist: " + dataDir);
			return;
		}

		std::vector<std::string> dataFiles;

		for (auto &p : std::filesystem::recursive_directory_iterator(dataDir))
		{
			if (p.path().extension() == LUA_EXT)
			{
				// Paths cannot contain ':', so we can use it as a separator
				dataFiles.push_back(p.path().string() + ":" + p.path().stem().string());
			}
		}

		for (auto &d : dataFiles)
		{
			size_t pos = d.find(':');

			// Everything before the ':' is the path
			const std::string dataPath = d.substr(0, pos);

			// Everything after the ':' is the name
			const std::string dataName = d.substr(pos + 1);

			{
				ZoneNamedC(loadDataFileZone, RandomUniqueColor(), true);

				std::cout << std::format("Loading Data '{}'\n", dataName);

				if (luaL_dofile(L, dataPath.c_str()) != LUA_OK)
				{
					if (lua_gettop(L) && lua_isstring(L, -1))
					{
						std::cout << "Data Failed to Load with the Error:\n" << lua_tostring(L, -1) << "\n";
						lua_pop(L, 1);
					}
				}
			}
		}
	}

	void LuaLoadMods(lua_State *L, const std::string &modsDir)
	{
		ZoneScopedC(RandomUniqueColor());

		// Ensure the mods directory exists
		if (!std::filesystem::exists(modsDir))
		{
			ErrMsg("Mods directory does not exist: " + modsDir);
			return;
		}

		std::vector<std::string> modFiles;

		for (auto &p : std::filesystem::recursive_directory_iterator(modsDir))
		{
			if (p.path().extension() == LTS_EXT)
			{
				// Paths cannot contain ':', so we can use it as a separator
				modFiles.push_back(p.path().string() + ":" + p.path().stem().string());
			}
		}

		for (auto &m : modFiles)
		{
			size_t pos = m.find(':');

			// Everything before the ':' is the path
			std::string scriptPath = m.substr(0, pos);

			// Replace '\\' with '/'
			std::replace(scriptPath.begin(), scriptPath.end(), '\\', '/');

			LuaLoadMod(L, scriptPath);
		}
	}

	bool LuaLoadMod(lua_State *L, const std::string &fullPath)
	{
		ZoneScopedC(RandomUniqueColor());

		std::cout << std::format("Loading Mod '{}'\n", fullPath);
		if (luaL_dostring(L, std::format("data.modding.loadLuaTableSave('{}')", fullPath).c_str()) != LUA_OK)
		{
			if (lua_gettop(L) && lua_isstring(L, -1))
			{
				std::cout << "Mod Failed to Load with the Error:\n" << lua_tostring(L, -1) << "\n";
				lua_pop(L, 1);
				return false;
			}
		}

		return true;
	}
}
