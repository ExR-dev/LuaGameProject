#include "stdafx.h"
#include "MenuScene.h"
#include "../Utilities/InputHandler.h"

bool MenuScene::MenuButton::Update()
{
    auto mInfo = Input::GetMouseInfo();
    auto mLeftState = Input::GetMouseState(Input::GameMouse::GAME_MOUSE_LEFT);

    isHovered = rect.CheckCollision(mInfo.position);

    // Update isPressed
    bool wasPressed = isPressed;
    if (isHovered && (mLeftState & Input::KeyState::PRESSED))
    {
        isPressed = true;
    }
    else if (isPressed && (mLeftState & Input::KeyState::RELEASED))
    {
        isPressed = false;
    }

    // Update isReleased
    isReleased = wasPressed && !isPressed;

    // Set color
    if (isPressed)
    {
        color = pressColor;
    }
    else if (isHovered)
    {
        color = hoverColor;
    }
    else
    {
        color = baseColor;
    }

    return isHovered && isReleased;
}

void MenuScene::MenuButton::Render()
{
    int fontSize = 40;
	raylib::Vector2 textOffset = MeasureTextEx(GetFontDefault(), name.c_str(), fontSize, 0);
	raylib::Vector2 rectMid = raylib::Vector2(rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f);

	DrawRectangleRec(rect, color);
	raylib::DrawText(name.c_str(), rectMid.x - textOffset.x * 0.5f, rectMid.y - textOffset.y * 0.5f, fontSize, BLACK);
}

int MenuScene::MenuScene::Start(WindowInfo *windowInfo)
{
    ZoneScopedC(RandomUniqueColor());

	m_windowInfo = windowInfo;

    m_camera.target = raylib::Vector2(0, 0);
    //m_camera.offset = raylib::Vector2(m_windowInfo->p_screenWidth / 2.0f, m_windowInfo->p_screenHeight / 2.0f);
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;

    // Setup button positions
	m_playButton.name = "Play";
    m_playButton.rect = raylib::Rectangle(20, 20, 300, 80);
	m_playButton.baseColor = raylib::Color(100, 190, 170, 255);
	m_playButton.hoverColor = raylib::Color(110, 160, 140, 255);
	m_playButton.pressColor = raylib::Color(65, 140, 120, 255);

    m_editorButton.name = "Editor";
    m_editorButton.rect = raylib::Rectangle(20, 120, 300, 80);
    m_editorButton.baseColor = raylib::Color(190, 180, 90, 255);
    m_editorButton.hoverColor = raylib::Color(160, 150, 100, 255);
    m_editorButton.pressColor = raylib::Color(130, 120, 55, 255);

    m_quitButton.name = "Quit";
    m_quitButton.rect = raylib::Rectangle(20, 420, 300, 80);
    m_quitButton.baseColor = raylib::Color(200, 90, 80, 255);
    m_quitButton.hoverColor = raylib::Color(160, 95, 85, 255);
    m_quitButton.pressColor = raylib::Color(130, 60, 50, 255);

	m_buttons.push_back(&m_playButton);
	m_buttons.push_back(&m_editorButton);
	m_buttons.push_back(&m_quitButton);

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

    for (auto button : m_buttons)
        button->Update();

    if (m_playButton.IsActivated())
	{
		return Game::SceneState::InGame;
	}
	else if (m_editorButton.IsActivated())
	{
		return Game::SceneState::InEditor;
	}
	else if (m_quitButton.IsActivated())
	{
		return Game::SceneState::Quitting;
	}


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
    BeginMode2D(m_camera);
    ClearBackground(raylib::Color(220, 220, 220));

    for (auto button : m_buttons)
        button->Render();

    auto mInfo = Input::GetMouseInfo();
	DrawRectangle(mInfo.position.x - 5, mInfo.position.y - 5, 10, 10, raylib::Color(255, 0, 0, 255));

    DrawFPS(340, 10);

    EndMode2D();


    rlImGuiBegin();

    bool open = true;
    ImGui::ShowDemoWindow(&open);

    rlImGuiEnd();

    EndDrawing();

    return 1;
}
