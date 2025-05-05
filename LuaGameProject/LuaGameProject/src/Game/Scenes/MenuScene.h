#pragma once
#include "dep/raylib-cpp/raylib-cpp.hpp"

#include "SceneTemplate.h"
#include "GameScene.h"
#include "EditorScene.h"

namespace MenuScene
{
    class MenuScene : SceneTemplate::SceneTemplate
    {
    public:
        MenuScene(WindowInfo *windowInfo) : m_windowInfo(windowInfo) {}

        int Start() override;
        Game::SceneState Loop() override;

    protected:
        int Update() override;
        int Render() override;
    };
}
