#pragma once
#include "dep/raylib-cpp/raylib-cpp.hpp"
#include "../Scene.h"

#include "../Utilities/WindowInfo.h"
#include "../Utilities/LuaGame.h"

namespace EditorScene
{
    class EditorScene
    {
    public:
        EditorScene(WindowInfo *windowInfo) : m_windowInfo(windowInfo) {}
        ~EditorScene();

        int Start();
        Game::SceneState Loop();

    private:
        lua_State *L = nullptr;
        Scene m_scene{};
        LuaGame::LuaGame m_luaGame;

        raylib::Camera2D m_camera{};

        WindowInfo *m_windowInfo;

        int Update();
        int Render();
    };
}
