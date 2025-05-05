#include "stdafx.h"
#include "LuaConsole.h"

#include "Game/Scenes/MenuScene.h"
#include "Game/Scenes/GameScene.h"
#include "Game/Scenes/EditorScene.h"

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

    menuScene.Start();
    editorScene.Start();
    gameScene.Start();

    Game::SceneState sceneState = Game::SceneState::InMenu;

    Start();
    FrameMark;

    while (!WindowShouldClose())
    {
        ZoneNamedNC(innerLoopZone, "Loop", RandomUniqueColor(), true);
        Time::Update();

        Input::UpdateInput();

        Update();

        Render();

        FrameMark;
    }

    CloseWindow();
    ResourceManager::Instance().UnloadResources();
    CloseAudioDevice();

    FrameMark;
    return 0;
}
