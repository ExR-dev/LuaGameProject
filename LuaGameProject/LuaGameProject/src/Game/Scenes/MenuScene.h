#pragma once
#include "dep/raylib-cpp/raylib-cpp.hpp"

#include "SceneTemplate.h"
#include "GameScene.h"
#include "EditorScene.h"

namespace MenuScene
{
    class MenuScene : public SceneTemplate::SceneTemplate
    {
    public:
        MenuScene() = default;

        int Start(WindowInfo *windowInfo) override;
        Game::SceneState Loop() override;

    protected:
        Game::SceneState Update() override;
        int Render() override;
    };
}
