#pragma once
#include "dep/raylib-cpp/raylib-cpp.hpp"
#include "../Utilities/WindowInfo.h"
#include <Game/Game.h>

namespace SceneTemplate
{
    class SceneTemplate
    {
    public:
        virtual int Start(WindowInfo *windowInfo) { return 0; };
        virtual Game::SceneState Loop() { return Game::SceneState::None; };

    protected:
        raylib::Camera2D m_camera{};
        WindowInfo *m_windowInfo;

        virtual Game::SceneState Update() { return Game::SceneState::None; };
        virtual int Render() { return 0; };

    private:
    };
}
