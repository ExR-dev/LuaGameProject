#include "stdafx.h"
#include "LuaConsole.h"
#include <crtdbg.h>

#include "Game/Scenes/MenuScene.h"
#include "Game/Scenes/GameScene.h"
#include "Game/Scenes/EditorScene.h"
#include "Game/Utilities/InputHandler.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

static int init(WindowInfo &windowInfo, raylib::RenderTexture &screenRT)
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

	screenRT = raylib::RenderTexture(windowInfo.p_screenWidth, windowInfo.p_screenHeight);

	Time::Instance();
	ResourceManager::Instance().LoadResources();
	//SetTargetFPS(144);

#ifdef IMGUI_HAS_DOCK
	auto &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
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
	raylib::RenderTexture screenRT;

	init(windowInfo, screenRT);

	std::unique_ptr<MenuScene::MenuScene> menuScene = std::make_unique<MenuScene::MenuScene>();
	std::unique_ptr<EditorScene::EditorScene> editorScene = std::make_unique<EditorScene::EditorScene>();
	std::unique_ptr<GameScene::GameScene>  gameScene = std::make_unique<GameScene::GameScene>();

	menuScene->Start(&windowInfo, &cmdState, &screenRT);
	editorScene->Start(&windowInfo, &cmdState, &screenRT);
	gameScene->Start(&windowInfo, &cmdState, &screenRT);

	SceneTemplate::SceneTemplate *currentScene = static_cast<SceneTemplate::SceneTemplate*>(menuScene.get());
	Game::SceneState sceneState = Game::SceneState::InMenu;
	currentScene->OnSwitchToScene();

	SetExitKey(0); // None


	// Threads do not work with windows leak detection, so we disable it
#ifndef LEAK_DETECTION 
	std::this_thread::sleep_for(std::chrono::milliseconds(16));
	// Start Lua console thread
	std::thread consoleThread(ConsoleThreadFunction, &cmdState);
	consoleThread.detach();
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
#endif

	FrameMark;

	bool isQuitting = false;
	while (!isQuitting)
	{
		ZoneNamedNC(innerLoopZone, "Loop", RandomUniqueColor(), true);

		if (IsWindowResized())
		{
			windowInfo.UpdateWindowSize(GetScreenWidth(), GetScreenHeight());
			screenRT = raylib::RenderTexture(windowInfo.p_screenWidth, windowInfo.p_screenHeight);
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
			gameScene->Start(&windowInfo, &cmdState, &screenRT);

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
			editorScene->Start(&windowInfo, &cmdState, &screenRT);

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


	// Draw the render texture to the screen
	BeginDrawing();
	{
		DrawTextureRec(
			screenRT.GetTexture(),
			raylib::Rectangle(0, 0, windowInfo.p_screenWidth, -windowInfo.p_screenHeight),
			raylib::Vector2(0, 0),
			raylib::Color(255, 255, 255, 255)
		);    

#ifdef TRACY_ENABLE
		// Add text to notify that tracy is enabled
		const raylib::Font &font = *ResourceManager::GetFontResource("LSANS.TTF");
		const std::string text = "Tracy is Enabled";

		const float 
			fontSize = 16.0f, 
			spacing = 1.6f,
			bgExtents = 4.0f;

		const raylib::Vector2 
			rect = font.MeasureText(text, fontSize, spacing),
			pos(6, 6);

		const raylib::Rectangle textBG(
			pos.x - bgExtents, pos.y - bgExtents,
			rect.x + 2 * bgExtents, rect.y + 2 * bgExtents
		);

		textBG.Draw(raylib::Color(0, 0, 0, 192));
		font.DrawText(text, pos, {0, 0}, 0, fontSize, spacing, { 255, 128, 128, 255 });
#endif
	}
	EndDrawing();

#ifdef TRACY_SCREEN_CAPTURE
		{
			ZoneNamedNC(screenCaptureZone, "Tracy Screen Capture", RandomUniqueColor(), true);

			raylib::Image screenImg = screenRT.GetTexture().GetData();
			screenImg.Resize(320, 180);

			{
				ZoneNamedNC(frameImageZone, "Tracy Frame Image", RandomUniqueColor(), true);
				FrameImage(
					screenImg.data,
					screenImg.width,
					screenImg.height,
					0,
					false
				);
			}
		}
#endif

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
