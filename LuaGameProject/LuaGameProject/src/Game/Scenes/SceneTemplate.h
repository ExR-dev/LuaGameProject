#pragma once
#include "dep/raylib-cpp/raylib-cpp.hpp"
#include "../Utilities/WindowInfo.h"
#include <Game/Game.h>

namespace SceneTemplate
{
    class SceneTemplate
    {
    public:
        SceneTemplate(WindowInfo *windowInfo) : m_windowInfo(windowInfo) {}

        virtual int Start() = 0;
        virtual Game::SceneState Loop() = 0;

    protected:
        raylib::Camera2D m_camera{};
        WindowInfo *m_windowInfo;

        virtual int Update() = 0;
        virtual int Render() = 0;

    private:
    };
}
