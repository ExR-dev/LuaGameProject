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

static int init(WindowInfo &windowInfo, CmdState &cmdState)
{
    ZoneScopedC(RandomUniqueColor());

#ifdef LEAK_DETECTION
    int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    flag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(flag);
#endif

	srand(time(NULL));
	SetTraceLogLevel(LOG_WARNING);

	// This warns that transparency must be set before window initialization.
	// Should not be a problem, as the window is clearly not created yet.
    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_TRANSPARENT);

    InitWindow(windowInfo.p_screenWidth, windowInfo.p_screenHeight, "Lua Game");
    InitAudioDevice();
    rlImGuiSetup(true);

    Time::Instance();
    ResourceManager::Instance().LoadResources();
    //SetTargetFPS(144);

#ifdef IMGUI_HAS_DOCK
    auto &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif
    
	// Threads do not work with windows leak detection, so we disable it
#ifndef LEAK_DETECTION 
    // Start Lua console thread
    std::thread consoleThread(ConsoleThreadFunction, &cmdState);
    consoleThread.detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Wait for the console thread to start
#endif

    return 0;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    ZoneScopedC(RandomUniqueColor());

    WindowInfo windowInfo;
    CmdState cmdState;

	init(windowInfo, cmdState);

	std::unique_ptr<MenuScene::MenuScene> menuScene = std::make_unique<MenuScene::MenuScene>();
    std::unique_ptr<EditorScene::EditorScene> editorScene = std::make_unique<EditorScene::EditorScene>();
    std::unique_ptr<GameScene::GameScene>  gameScene = std::make_unique<GameScene::GameScene>();

    menuScene->Start(&windowInfo, &cmdState);
    editorScene->Start(&windowInfo, &cmdState);
    gameScene->Start(&windowInfo, &cmdState);

	SceneTemplate::SceneTemplate *currentScene = static_cast<SceneTemplate::SceneTemplate*>(menuScene.get());
    Game::SceneState sceneState = Game::SceneState::InMenu;
    currentScene->OnSwitchToScene();

    FrameMark;

	bool isQuitting = false;
    while (!isQuitting)
    {
        ZoneNamedNC(innerLoopZone, "Loop", RandomUniqueColor(), true);

		if (IsWindowResized())
        {
            windowInfo.UpdateWindowSize(GetScreenWidth(), GetScreenHeight());
			currentScene->OnResizeWindow();
        }

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
            currentScene->OnSwitchFromScene();
            if (currentScene == gameScene.get())
                EnableCursor();
			currentScene = menuScene.get();
			currentScene->OnSwitchToScene();
			break;

		case Game::SceneState::InGame:
            currentScene->OnSwitchFromScene();
            if (currentScene != gameScene.get())
                DisableCursor();
			currentScene = gameScene.get();
            currentScene->OnSwitchToScene();
			break;

		case Game::SceneState::InEditor:
            currentScene->OnSwitchFromScene();
            if (currentScene == gameScene.get())
                EnableCursor();
			currentScene = editorScene.get();
            currentScene->OnSwitchToScene();
			break;

        case Game::SceneState::ReloadGame:
            currentScene->OnSwitchFromScene();
            if (currentScene == gameScene.get())
                currentScene = nullptr;

            gameScene = nullptr;
			gameScene = std::make_unique<GameScene::GameScene>();
            gameScene->Start(&windowInfo, &cmdState);

            if (!currentScene)
				currentScene = gameScene.get();
            currentScene->OnSwitchToScene();
            break;

        case Game::SceneState::ReloadEditor:
            currentScene->OnSwitchFromScene();
            if (currentScene == editorScene.get())
                currentScene = nullptr;

            editorScene = nullptr;
            editorScene = std::make_unique<EditorScene::EditorScene>();
            editorScene->Start(&windowInfo, &cmdState);

            if (!currentScene)
				currentScene = editorScene.get();
            currentScene->OnSwitchToScene();
            break;

        case Game::SceneState::Quitting:
            currentScene->OnSwitchFromScene();
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
