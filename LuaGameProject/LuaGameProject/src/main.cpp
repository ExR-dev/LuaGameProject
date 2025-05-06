#include "stdafx.h"
#include "LuaConsole.h"

#include "Game/Scenes/MenuScene.h"
#include "Game/Scenes/GameScene.h"
#include "Game/Scenes/EditorScene.h"
#include <Game/Utilities/InputHandler.h>

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    ZoneScopedC(RandomUniqueColor());

	srand(time(NULL));

    WindowInfo windowInfo;

    InitWindow(windowInfo.p_screenWidth, windowInfo.p_screenHeight, "Lua Game");
    InitAudioDevice();

    Time::Instance();
    ResourceManager::Instance().LoadResources();
    SetTargetFPS(144);

	MenuScene::MenuScene menuScene;
	EditorScene::EditorScene editorScene;
	GameScene::GameScene gameScene;

    menuScene.Start(&windowInfo);
    editorScene.Start(&windowInfo);
    gameScene.Start(&windowInfo);

	SceneTemplate::SceneTemplate *currentScene = static_cast<SceneTemplate::SceneTemplate*>(&menuScene);
    Game::SceneState sceneState = Game::SceneState::InMenu;

    FrameMark;

	bool isQuitting = false;
    while (!isQuitting && !WindowShouldClose())
    {
        ZoneNamedNC(innerLoopZone, "Loop", RandomUniqueColor(), true);

        Time::Update();
        Input::UpdateInput();

        Game::SceneState newState = currentScene->Loop();

		switch (newState)
		{
		case Game::SceneState::InMenu:
            EnableCursor();
			currentScene = &menuScene;
			break;

		case Game::SceneState::InGame:
            DisableCursor();
			currentScene = &gameScene;
			break;

		case Game::SceneState::InEditor:
            EnableCursor();
			currentScene = &editorScene;
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
    return 0;
}
