#pragma once
#include "lua.hpp"
#include <atomic>
#include <vector>
#include <string>
#include "dep/EnTT/entt.hpp"

struct CmdState
{
    std::string cmdInput = "";
    std::atomic_bool pauseCmdInput = false;
};

void ConsoleThreadFunction(CmdState *cmdState);
void ExecuteCommandList(lua_State *L, CmdState *cmdState, const entt::registry &reg);
