#include "stdafx.h"
#include "LuaConsole.h"
#include <crtdbg.h>

#include "Game/Scenes/MenuScene.h"
#include "Game/Scenes/GameScene.h"
#include "Game/Scenes/EditorScene.h"
#include <Game/Utilities/InputHandler.h>

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    ZoneScopedC(RandomUniqueColor());

#ifdef LEAK_DETECTION
    int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    flag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(flag);
#endif

	srand(time(NULL));

    WindowInfo windowInfo;

    InitWindow(windowInfo.p_screenWidth, windowInfo.p_screenHeight, "Lua Game");
    InitAudioDevice();

    Time::Instance();
    ResourceManager::Instance().LoadResources();
    SetTargetFPS(144);

    rlImGuiSetup(true);
#ifdef IMGUI_HAS_DOCK
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif

	std::unique_ptr<MenuScene::MenuScene> menuScene = std::make_unique<MenuScene::MenuScene>();
    std::unique_ptr<EditorScene::EditorScene> editorScene = std::make_unique<EditorScene::EditorScene>();
    std::unique_ptr<GameScene::GameScene>  gameScene = std::make_unique<GameScene::GameScene>();

    menuScene->Start(&windowInfo);
    editorScene->Start(&windowInfo);
    gameScene->Start(&windowInfo);

	SceneTemplate::SceneTemplate *currentScene = static_cast<SceneTemplate::SceneTemplate*>(menuScene.get());
    Game::SceneState sceneState = Game::SceneState::InMenu;

    FrameMark;

	bool isQuitting = false;
    while (!isQuitting)
    {
        ZoneNamedNC(innerLoopZone, "Loop", RandomUniqueColor(), true);

        Time::Update();
        Input::UpdateInput();

        Game::SceneState newState = currentScene->Loop();

        if (WindowShouldClose())
        {
            if (currentScene == menuScene.get())
                newState = Game::SceneState::Quitting;
            else
                newState = Game::SceneState::InMenu;
        }

		switch (newState)
		{
		case Game::SceneState::InMenu:
            EnableCursor();
			currentScene = menuScene.get();
			break;

		case Game::SceneState::InGame:
            DisableCursor();
			currentScene = gameScene.get();
			break;

		case Game::SceneState::InEditor:
            EnableCursor();
			currentScene = editorScene.get();
			break;

        case Game::SceneState::ReloadGame:
            if (currentScene == gameScene.get())
                currentScene = nullptr;

            gameScene = nullptr;
			gameScene = std::make_unique<GameScene::GameScene>();
            gameScene->Start(&windowInfo);

            if (!currentScene)
				currentScene = gameScene.get();
            break;

        case Game::SceneState::ReloadEditor:
            if (currentScene == editorScene.get())
                currentScene = nullptr;

            editorScene = nullptr;
            editorScene = std::make_unique<EditorScene::EditorScene>();
            editorScene->Start(&windowInfo);

            if (!currentScene)
				currentScene = editorScene.get();
            break;

        case Game::SceneState::Quitting:
			isQuitting = true;
            break;

        case Game::SceneState::None:
		default:
			break;
		}   

        FrameMark;
    }

    Game::IsQuitting = false;

    CloseWindow();
    ResourceManager::Instance().UnloadResources();
    CloseAudioDevice();

    FrameMark;

#ifdef LEAK_DETECTION
    _CrtDumpMemoryLeaks();
#endif
    return 0;
}
