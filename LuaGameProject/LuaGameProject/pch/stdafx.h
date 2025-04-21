#pragma once

#include <iostream>
#include <string>
#include <format>
#include <atomic>
#include <thread>
#include <execution>
#include <unordered_map>

#include "dep/raylib-cpp/raylib-cpp.hpp"
#include "dep/rcamera.h"
#include "dep/EnTT/entt.hpp"
#include "lua.hpp"

#include "dep/tracy-0.11.1/public/tracy/Tracy.hpp"
#include "dep/tracy-0.11.1/public/tracy/TracyOpenGL.hpp"

#include "Game/Tools/ErrMsg.h"
#include "LuaUtils.h"
#include "Game/Utilities/Time.h"
#include "Game/Utilities/WindowsWrapped.h"
