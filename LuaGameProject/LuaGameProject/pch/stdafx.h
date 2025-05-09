#pragma once

#include "Game/Tools/TrackedAlloc.h"
#include "Game/Tools/DebugNew.h"

#include <iostream>
#include <string>
#include <format>
#include <atomic>
#include <thread>
#include <execution>
#include <mutex>
#include <unordered_map>
#include <filesystem>
namespace fs = std::filesystem;

#include "dep/raylib-cpp/raylib-cpp.hpp"
#include "dep/rcamera.h"
#include "dep/EnTT/entt.hpp"
#include "lua.hpp"
#include "box2d/box2D.h"

#include "dep/tracy-0.11.1/public/tracy/Tracy.hpp"
#include "dep/tracy-0.11.1/public/tracy/TracyLua.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "dep/imgui/imgui.h"
#include "dep/imgui/rlImGui.h"

#include "Game/Game.h"
#include "Game/Tools/ErrMsg.h"
#include "LuaUtils.h"
#include "Game/Utilities/Time.h"
#include "Game/Utilities/WindowsWrapped.h"
#include "Game/Utilities/GameMath.h"
#include "Game/Tools/ConstRand.h"
#include "Game/Tools/ImLua.h"
#include "Game/ResourceManager.h"
#include "Game/Utilities/InputHandler.h"


