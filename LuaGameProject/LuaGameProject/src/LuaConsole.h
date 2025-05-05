#pragma once
#include "lua.hpp"
#include <atomic>
#include <vector>
#include <string>
#include "dep/EnTT/entt.hpp"

void ConsoleThreadFunction(std::string *cmdList, std::atomic_bool *pauseCmdInput);
void ExecuteCommandList(lua_State *L, std::string *cmdList, std::atomic_bool *pauseCmdInput, const entt::registry &reg);
