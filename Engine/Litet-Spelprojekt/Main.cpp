#include "stdafx.h"
#include "Game.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

int main(int argc, char* argv[])
{
	int returnCode = 1;
	bool running = true;

#ifdef PIX_TIMELINING
	PIXScopedEvent(0, "Litet Spelprojekt");
#endif
#ifdef FAST_LOAD_MODE
	ErrMsg("WARNING! WARNING! WARNING! WARNING! WARNING! WARNING! WARNING! WARNING! WARNING! WARNING!");
	ErrMsg("FAST_LOAD_MODE preprocessor flag is defined.");
	ErrMsg("This means that most content will be skipped to keep the startup time short.");
	ErrMsg("If you do not want this, disable the flag in EngineSettings.h.");
	ErrMsg("WARNING! WARNING! WARNING! WARNING! WARNING! WARNING! WARNING! WARNING! WARNING! WARNING!");
#endif
#ifdef LEAK_DETECTION
	// Get current flag
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

	// Turn on leak-checking bit.
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

	// Set flag to the new value.
	_CrtSetDbgFlag( tmpFlag );
#endif

	ErrMsg("\n=======| Start |===========================================================================");

	// Seed random number generator
	srand(static_cast<unsigned>(time(0)));

	// Setup of window and input
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		ErrMsg("Failed to CoInitializeEx!");
		return 0;
	}

	WindowSize windowSize = {
		//2560, 1440
		1920, 1080
		//1280, 720
	};

	Input *input = Input::GetInstance();
	input->SetWindowSize(windowSize);

	Window *window = nullptr;
#ifdef USE_SDL3
	window = new WindowSDL3();
#endif

	if (!window)
	{
		ErrMsg("No window library has been configured!");
		return -1;
	}

	if (!window->Initialize("Lurks Below", input->GetWindowSize().width, input->GetWindowSize().height))
	{
		ErrMsg("Failed to setup window!");
		return -1;
	}

	Time time = Time::GetInstance();
	MSG msg = { };

	BindingCollection *inputBindings = BindingCollection::GetInstance();
	inputBindings->LoadBindings("Content/Bindings.txt");

	// Setup of game. Loads all assets into memory like meshes, textures, shaders etc. Also creates the graphics manager.
	Game game = { };
	if (!game.Setup(time, windowSize.width, windowSize.height, window))
	{
		ErrMsg("Failed to setup game!");
		return -1;
	}

	// Print content load times.
	ErrMsg(std::format("Content Load: {} s", time.CompareSnapshots("LoadContent")));

	// Create scenes. All are initialized immediately, which spawns all entities.
#ifdef DEBUG_BUILD
	Scene *scenes = new Scene[5];
	int sceneCount = 5;
#else
	Scene *scenes = new Scene[4];
	int sceneCount = 4;
#endif

	int sceneIndexer = 0;
	time.TakeSnapshot("AddSceneMenu");
	if (!game.AddScene(&scenes[sceneIndexer++], true, "MenuSave"))
	{
		ErrMsg("Failed to add scene!");
		return -1;
	}
	time.TakeSnapshot("AddSceneMenu");
	ErrMsg(std::format("Add Menu Scene: {} s", time.CompareSnapshots("AddSceneMenu")));

	time.TakeSnapshot("AddSceneMap");
	if (!game.AddScene(&scenes[sceneIndexer++], false, "MapSave"))
	{
		ErrMsg("Failed to add scene!");
		return -1;
	}
	time.TakeSnapshot("AddSceneMap");
	ErrMsg(std::format("Add Map Scene: {} s", time.CompareSnapshots("AddSceneMap")));

	time.TakeSnapshot("AddSceneCredit");
	if (!game.AddScene(&scenes[sceneIndexer++], false, "CreditSave"))
	{
		ErrMsg("Failed to add scene!");
		return -1;
	}
	time.TakeSnapshot("AddSceneCredit");
	ErrMsg(std::format("Add Credit Scene: {} s", time.CompareSnapshots("AddSceneCredit")));

	/*
	time.TakeSnapshot("AddStartCutscene");
	if (!game.AddScene(&scenes[sceneIndexer++], true, "StartCutsceneSave"))
	{
		ErrMsg("Failed to add scene!");
		return -1;
	}
	time.TakeSnapshot("AddStartCutscene");
	std::cout << std::format("Add Menu Scene: {} s\n", time.CompareSnapshots("AddSceneMenu"));
	*/

#ifdef DEBUG_BUILD
	time.TakeSnapshot("AddSceneDev");
	if (!game.AddScene(&scenes[sceneIndexer++], false, "Save1"))
	{
		ErrMsg("Failed to add scene!");
		return -1;
	}
	time.TakeSnapshot("AddSceneDev");
	ErrMsg(std::format("Add Dev Scene: {} s", time.CompareSnapshots("AddSceneDev")));
#endif


#ifdef EDIT_MODE
	// Set active scene to the game scene
	if (!game.SetScene(GAME_SCENE))
	{
		ErrMsg("Failed to set scene!");
		return -1;
	}
#endif

	if (!game.SetScene(GAME_SCENE))
	{
		ErrMsg("Failed to set scene!");
		return -1;
	}


	window->UpdateWindowSize();

	size_t frameCount = 0;
	while (running)
	{
#ifdef PIX_TIMELINING
		PIXScopedEvent(625862846, std::format("Frame {}", frameCount++).c_str());
#endif
		input->SetMouseScroll(0, 0);

		if (!window->UpdateWindow(input))
		{
			ErrMsg("Failed to update window!");
			returnCode = -1;
			running = false;
		}

		if (window->IsClosing())
		{
			returnCode = 1;
			running = false;
		}
		
		// Toggle fullscreen with [Left Control] + [Enter]
		if (BindingCollection::IsTriggered(InputBindings::InputAction::Fullscreen))
		{
			if (!window->ToggleFullscreen())
			{
				ErrMsg("Failed to toggle fullscreen!");
			}
		}

		if (BindingCollection::IsTriggered(InputBindings::InputAction::Exit))
		{
			running = false;
		}

		// Update delta time, 
		time.Update();

		// Lock cursor to window with [Left Control] + [Tab]
		if (BindingCollection::IsTriggered(InputBindings::InputAction::LockCursor))
			input->ToggleLockCursor(window);

		if (!input->Update(window))
		{
			ErrMsg("Failed to update input!");
			returnCode = -1;
			running = false;
		}

		BindingCollection::Update();

#ifdef PIX_TIMELINING
	// Add PIX bookmark on pressing B
		if (input->GetKey(KeyCode::B) == KeyState::Pressed)
		{
			PIXSetMarker(PIX_COLOR(255, 0, 0), "Bookmark");
		}
#endif

		static UINT sceneIndex = 0;
		static bool changeScene = true;

#ifdef DEBUG_BUILD
		// Debug scene switch hotkeys
		// -------------------------------
		/*
		if (BindingCollection::IsTriggered(InputBindings::InputAction::CycleScene))
		{
			UINT activeIndex = game.GetActiveScene();
			sceneIndex = activeIndex == sceneCount - 1 ? 0 : activeIndex + 1;
			if (!game.SetScene(sceneIndex))
			{
				ErrMsg("Failed to set scene!");
				returnCode = -1;
				running = false;
			}
			changeScene = true;
		}
		*/
#endif

		if (changeScene)
		{
			if (sceneIndex == MENU_SCENE || sceneIndex == CRED_SCENE)
			{
				FogSettingsBuffer fogSettings;
				fogSettings.thickness = 0.03f;
				game.GetGraphics()->SetFogSettings(fogSettings);
			}
			else
			{
				FogSettingsBuffer fogSettings;
				game.GetGraphics()->SetFogSettings(fogSettings);
			}
			changeScene = false;
		}
		// --------------------------------

		static bool isPaused = true;
		static UINT lastScene = 1;
		static bool isTransitioning = false;
		static float transitionTime = 0.0f;

		if (isTransitioning)
		{
			transitionTime -= time.deltaTime;

			if (transitionTime <= 0.0f)
			{
				isTransitioning = false;
				transitionTime = 0.0f;
				game.GetGraphics()->BeginScreenFade(-0.5f);

				if (game.GetActiveScene() != MENU_SCENE) // Into pause menu
				{
					isPaused = false;
					if (!isPaused)
					{
						lastScene = game.GetActiveScene();

						if (input->IsInFocus() && input->IsCursorLocked())
						{
							input->SetMousePosition(window, window->GetWidth() / 2.0f, window->GetHeight() / 2.0f);
							input->ToggleLockCursor(window);
						}

						if (!game.SetScene(MENU_SCENE))
						{
							ErrMsg("Failed to set scene!");
							returnCode = -1;
							running = false;
						}
						isPaused = true;

						FogSettingsBuffer fogSettings;
						fogSettings.thickness = 0.03f;
						game.GetGraphics()->SetFogSettings(fogSettings);

						MeshBehaviour *mesh = nullptr;
						game.GetScene(MENU_SCENE)->GetSceneHolder()->GetEntityByName("StartButton")->GetBehaviourByType<MeshBehaviour>(mesh);
						Material mat = Material(*mesh->GetMaterial());
						mat.textureID = game.GetScene(MENU_SCENE)->GetContent()->GetTextureID("Tex_Button_Continue_Texture");
						if (!mesh->SetMaterial(&mat))
						{
							ErrMsg("Failed to set material to continue button!");
							returnCode = -1;
							running = false;
						}
					}
				}
				else if (game.GetActiveScene() == MENU_SCENE && isPaused) // Out of pause menu
				{
					if (!game.SetScene(lastScene))
					{
						ErrMsg("Failed to set scene!");
						returnCode = -1;
						running = false;
					}

					if (input->IsInFocus() && !input->IsCursorLocked())
					{
						input->SetMousePosition(window, 0.0f, 0.0f);
						input->ToggleLockCursor(window);
					}
					isPaused = false;

					FogSettingsBuffer fogSettings;
					game.GetGraphics()->SetFogSettings(fogSettings);
				}
			}
		}
		else if (BindingCollection::IsTriggered(InputBindings::InputAction::Pause))
		{
			if (game.GetActiveScene() == CRED_SCENE)
			{
				lastScene = MENU_SCENE;
			}
			
			if (game.GetActiveScene() != MENU_SCENE)
			{
				isTransitioning = true;
				transitionTime = 0.5f;
				game.GetGraphics()->BeginScreenFade(transitionTime);
			}
		}

		if (game.GetActiveScene() == MENU_SCENE)
		{
			if (input->IsInFocus() && input->IsCursorLocked())
			{
				input->SetMousePosition(window, window->GetWidth() / 2.0f, window->GetHeight() / 2.0f);
				input->ToggleLockCursor(window);
			}
		}
		else if (game.GetActiveScene() == CRED_SCENE)
		{
			if (input->IsInFocus() && !input->IsCursorLocked())
			{
				input->ToggleLockCursor(window);
			}
		}

		if (!game.Update(time, *input))
		{
			ErrMsg("Failed to update game logic!");
			returnCode = -1;
			running = false;
		}

		if (!game.Render(time, *input))
		{
			ErrMsg("Failed to render frame!");
			returnCode = -1;
			running = false;
		}
	}

	delete[] scenes;

	ErrMsg("========| End |============================================================================\n");
	return returnCode;
}
