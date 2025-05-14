#pragma once
#include "dep/raylib-cpp/raylib-cpp.hpp"
#include "../Utilities/WindowInfo.h"
#include "Game/Game.h"
#include "LuaConsole.h"

namespace SceneTemplate
{
    class SceneTemplate
    {
    public:
        virtual int Start(WindowInfo *windowInfo, CmdState *cmdState, raylib::RenderTexture *screenRT) { return 0; };
        virtual Game::SceneState Loop() { return Game::SceneState::None; };

        virtual void OnSwitchToScene() {}
        virtual void OnSwitchFromScene() {}
        virtual void OnResizeWindow() {}

    protected:
        raylib::Camera2D m_camera{};
        WindowInfo *m_windowInfo = nullptr;
        CmdState *m_cmdState = nullptr;
		raylib::RenderTexture *m_screenRT = nullptr;

        virtual Game::SceneState Update() { return Game::SceneState::None; };
        virtual int Render() { return 0; };

    private:

    };
}
