#pragma once
#include "lua.hpp"
#include <atomic>
#include <vector>
#include <string>

void ConsoleThreadFunction(lua_State *L, std::string *cmdList, std::atomic_bool *pauseCmdInput);
void ExecuteCommandList(lua_State *L, std::string *cmdList, std::atomic_bool *pauseCmdInput);
