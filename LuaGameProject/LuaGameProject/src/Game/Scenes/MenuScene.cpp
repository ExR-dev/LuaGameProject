#include "stdafx.h"
#include "MenuScene.h"
#include "../Utilities/InputHandler.h"

int MenuScene::MenuScene::Start(WindowInfo *windowInfo)
{
    ZoneScopedC(RandomUniqueColor());

	m_windowInfo = windowInfo;

    m_camera.target = raylib::Vector2(0, 0);
    m_camera.offset = raylib::Vector2(m_windowInfo->p_screenWidth / 2.0f, m_windowInfo->p_screenHeight / 2.0f);
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;

    return 1;
}

Game::SceneState MenuScene::MenuScene::Loop()
{
    ZoneScopedC(RandomUniqueColor());

    auto state = Update();

    Render();

    return state;
}

Game::SceneState MenuScene::MenuScene::Update()
{
    ZoneScopedC(RandomUniqueColor());


    if (Input::CheckKeyPressed(Input::GAME_KEY_2))
    {
		return Game::SceneState::InEditor;
    }
    else if (Input::CheckKeyPressed(Input::GAME_KEY_3))
    {
		return Game::SceneState::InGame;
    }
    else if (Input::CheckKeyPressed(Input::GAME_KEY_0))
    {
		return Game::SceneState::Quitting;
    }

    return Game::SceneState::None;
}

int MenuScene::MenuScene::Render()
{
    ZoneScopedC(RandomUniqueColor());

    BeginDrawing();
    ClearBackground(LIGHTGRAY);

    DrawText("Controls:", 20, 20, 10, BLACK);
    DrawText("- A/D to move", 40, 40, 10, DARKGRAY);
    DrawText("- W/Space to jump", 40, 60, 10, DARKGRAY);
    DrawText("- Mouse Wheel to Zoom in-out, R to reset zoom", 40, 80, 10, DARKGRAY);
    DrawText("- C to change camera mode", 40, 100, 10, DARKGRAY);
    DrawText("Current camera mode:", 20, 120, 10, BLACK);

    DrawFPS(340, 10);

    EndDrawing();

    return 1;
}
