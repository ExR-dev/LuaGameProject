#include "stdafx.h"
#include "GameScene.h"

#include "../../LuaConsole.h"
#include "../Utilities/DungeonGenerator.h"

#include "../Utilities/InputHandler.h"
#include "../Utilities/LuaInput.h"
#include "../Utilities/ModLoader.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

GameScene::GameScene::GameScene()
{
    ZoneScopedC(RandomUniqueColor());

	Game::IsQuitting = false;
}
GameScene::GameScene::~GameScene()
{
    ZoneScopedC(RandomUniqueColor());

	Game::IsQuitting = true;

	if (m_dungeon)
	{
		delete m_dungeon;
		m_dungeon = nullptr;
	}

	if (L)
	{
		lua_close(L);
		L = nullptr;
	}
}

int GameScene::GameScene::Start(WindowInfo *windowInfo)
{
    ZoneScopedC(RandomUniqueColor());

	m_windowInfo = windowInfo;

    // Setup Box2D
    m_physicsHandler.Setup();

    // Setup Lua enviroment

    // Create internal lua state
    L = luaL_newstate();
    luaL_openlibs(L);

    m_windowInfo->BindLuaWindow(L);

    Scene::lua_openscene(L, &m_scene);

    m_luaGame = LuaGame::LuaGame(L, &m_scene);
    LuaGame::LuaGame::lua_opengame(L, &m_luaGame);

    m_freeCam.speed = 350;
    m_freeCam.position = raylib::Vector2(400, 280);

    m_camera.target = m_freeCam.position;
    m_camera.offset = raylib::Vector2(m_windowInfo->p_screenWidth / 2.0f, m_windowInfo->p_screenHeight / 2.0f);
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;

    m_cameraUpdater = std::bind(&GameScene::UpdatePlayerCamera, this);
    m_cameraOption = 0;

    Assert(!m_dungeon, "m_dungeon is not nullptr!");
    m_dungeon = new DungeonGenerator(raylib::Vector2(200, 200));
    m_dungeon->Generate(100);

    BindLuaInput(L);

    // Add Lua require path
    std::string luaScriptPath = std::format("{}/{}?{}", fs::current_path().generic_string(), FILE_PATH, FILE_EXT);
    LuaDoString(std::format("package.path = \"{};\" .. package.path", luaScriptPath).c_str());

    // Start Lua console thread
#ifndef LEAK_DETECTION
    m_cmdList.clear();
    std::thread consoleThread(ConsoleThreadFunction, &m_cmdList, &m_pauseCmdInput);
    consoleThread.detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Wait for the console thread to start
#endif

    // Initialize Lua data & mods
    ModLoader::LuaLoadData(L, DATA_PATH);
	ModLoader::LuaLoadMods(L, MOD_PATH);

    m_scene.SystemsInitialize(L);

    LuaDoFileCleaned(L, LuaFilePath("InitDevScene")); // Creates entities

    return 1;
}

Game::SceneState GameScene::GameScene::Loop()
{
    ZoneScopedC(RandomUniqueColor());

#if defined(LUA_DEBUG) && !defined(LEAK_DETECTION)
    if (Game::Game::Instance().CmdStepMode)
    {
		if (Game::Game::Instance().CmdTakeSteps > 0)
		{
            Game::Game::Instance().CmdTakeSteps--;
		}
		else
		{
            ExecuteCommandList(L, &m_cmdList, &m_pauseCmdInput, m_scene.GetRegistry());
            Windows::SleepW(16);
            return Game::SceneState::None;
		}
    }
#endif

	auto state = Update();

	Render();

#ifndef LEAK_DETECTION
	ExecuteCommandList(L, &m_cmdList, &m_pauseCmdInput, m_scene.GetRegistry());
#endif

    return state;
}

void GameScene::GameScene::OnSwitchToScene()
{
    ZoneScopedC(RandomUniqueColor());

    OnResizeWindow();
}

void GameScene::GameScene::OnResizeWindow()
{
    ZoneScopedC(RandomUniqueColor());

    m_windowInfo->BindLuaWindow(L);
}

Game::SceneState GameScene::GameScene::Update()
{
    ZoneScopedC(RandomUniqueColor());

    // Toggle mouse
    if (Input::CheckMousePressed(Input::GAME_MOUSE_RIGHT))
    {
        if (m_cursorEnabled)
        {
            DisableCursor();
            m_cursorEnabled = false;
        }
        else
        {
            EnableCursor();
            m_cursorEnabled = true;
        }
    }

    m_camera.zoom += ((float)GetMouseWheelMove() * 0.05f);

    if (m_camera.zoom > 3.0f) m_camera.zoom = 3.0f;
    else if (m_camera.zoom < 0.25f) m_camera.zoom = 0.25f;

    if (Input::CheckKeyPressed(Input::GAME_KEY_R))
    {
        m_camera.zoom = 1.0f;
        m_freeCam.position = raylib::Vector2(400, 280);
    }

    if (Input::CheckKeyPressed(Input::GAME_KEY_T))
    {
        m_dungeon->Initialize();
        m_dungeon->Generate(100);
    }

    if (Input::CheckKeyPressed(Input::GAME_KEY_Y))
        m_dungeon->SeparateRooms();

    if (Input::CheckKeyPressed(Input::GAME_KEY_C))
    {
        m_cameraOption = (m_cameraOption + 1) % CAMERA_OPTIONS;

        switch (m_cameraOption)
        {
        case 0: default:
            m_cameraUpdater = std::bind(&GameScene::UpdatePlayerCamera, this);
            break;

		case 1:
            m_cameraUpdater = std::bind(&GameScene::UpdateFreeCamera, this);
            break;
        }
    }


    // Update systems
    m_scene.SystemsOnUpdate(Time::DeltaTime());

    // Call update camera function by its pointer
    m_cameraUpdater();

    // Update Physics
    m_physicsHandler.Update(L, &m_scene);

    std::function<void(entt::registry& registry)> createPhysicsBodies = [&](entt::registry& registry) {
        ZoneNamedNC(createPhysicsBodiesZone, "Lambda Update Physics Bodies", RandomUniqueColor(), true);

        auto view = registry.view<ECS::Collider, ECS::Transform>();
        view.use<ECS::Collider>();

        view.each([&](const entt::entity entity, ECS::Collider& collider, ECS::Transform& transform) {
            ZoneNamedNC(drawSpriteZone, "Lambda Update Physics Boddy", RandomUniqueColor(), true);

            if (!collider.createBody)
            {
                b2Body_SetTransform(collider.bodyId, { transform.Position[0] + collider.offset[0], transform.Position[1] + collider.offset[1]}, {cosf(transform.Rotation * DEG2RAD), sinf(transform.Rotation * DEG2RAD)});
            }
            else
            {
                ZoneNamedNC(drawSpriteZone, "Lambda Create Physics Boddy", RandomUniqueColor(), true);
			    collider.bodyId = m_physicsHandler.CreateRigidBody(static_cast<int>(entity), collider, transform);
			    collider.createBody = false;
            }
        });
    };

    m_scene.RunSystem(createPhysicsBodies);

    m_scene.CleanUp(L);

    return Game::SceneState::None;
}

int GameScene::GameScene::Render()
{
    ZoneScopedC(RandomUniqueColor());

    BeginDrawing();
    ClearBackground(LIGHTGRAY);

    // Update systems
    m_scene.SystemsOnRender(Time::DeltaTime());

    // Scene
    {
        ZoneNamedNC(renderSceneZone, "Render Scene", RandomUniqueColor(), true);
        BeginMode2D(m_camera);

        // Draw sprites
        std::function<void(entt::registry &registry)> drawSystem = [](entt::registry &registry) {
            ZoneNamedNC(drawSpritesZone, "Lambda Draw Sprites", RandomUniqueColor(), true);

            auto view = registry.view<ECS::Sprite, ECS::Transform>();
            view.use<ECS::Sprite>();

            view.each([&](const entt::entity entity, const ECS::Sprite &sprite, const ECS::Transform &transform) {
                ZoneNamedNC(drawSpriteZone, "Lambda Draw Sprite", RandomUniqueColor(), true);

                // If the entity has an active component, check if it is active
                if (registry.all_of<ECS::Active>(entity))
                {
                    ECS::Active &active = registry.get<ECS::Active>(entity);
                    if (!active.IsActive)
						return; // Skip drawing if the entity is not active
                }

                int flip = transform.Scale[1] > 0 ? 1 : -1;

                raylib::Color color(*(raylib::Vector4 *)(&(sprite.Color)));
                raylib::Rectangle rect(
                    transform.Position[0],
                    transform.Position[1],
                    transform.Scale[0],
                    transform.Scale[1] * flip
                );

                raylib::Vector2 origin(
                    (transform.Scale[0] / 2),
                    (transform.Scale[1] / 2) * flip
                );

                std::string textureName = sprite.SpriteName;
                const raylib::Texture2D *texture = nullptr;

                if (textureName != "")
                {
                    texture = ResourceManager::Instance().GetTextureResource(textureName);

                    if (!texture)
                    {
                        ResourceManager::Instance().LoadTextureResource(textureName);
                        texture = ResourceManager::Instance().GetTextureResource(textureName);
                    }
                }

                if (texture)
                {
                    DrawTexturePro(
                        *texture,
                        raylib::Rectangle(0, 0, texture->width, texture->height * flip),
                        rect,
                        origin,
                        transform.Rotation,
                        color
                    );
                }
                else
                {
                    DrawRectanglePro(
                        rect, 
                        origin, 
                        transform.Rotation, 
                        color
                    );
                }
            });
        };
		m_scene.RunSystem(drawSystem);

		std::function<void(entt::registry& registry)> drawPhysicsBodies = [&](entt::registry& registry) {
			ZoneNamedNC(drawPhysicsBodiesZone, "Lambda Draw Physics Bodies", RandomUniqueColor(), true);

			auto view = registry.view<ECS::Collider, ECS::Transform>();
			view.use<ECS::Collider>();

			view.each([&](ECS::Collider& collider, ECS::Transform& transform) {
				ZoneNamedNC(drawSpriteZone, "Lambda Draw Physics Body", RandomUniqueColor(), true);

                if (collider.debug)
                {
					const float w = fabsf(transform.Scale[0] * collider.extents[0]),
								h = fabsf(transform.Scale[1] * collider.extents[1]);
					b2Vec2 p = b2Body_GetWorldPoint(collider.bodyId, { 0, 0});
					b2Transform t;

					b2Rot rotation = b2Body_GetRotation(collider.bodyId);
					float radians = b2Rot_GetAngle(rotation);

					Rectangle rect = { p.x, p.y , w, h };
					DrawRectanglePro(rect, { w/2, h/2 }, radians*RAD2DEG, {0, 228, 46, 100});
                }
			});
		};

		m_scene.RunSystem(drawPhysicsBodies);

		// Draw the dungeon
        m_dungeon->Draw();

        EndMode2D();
    }

    // UI
    {
        ZoneNamedNC(GameSceneRenderScene, "Render UI", RandomUniqueColor(), true);
        static const char *cameraDescriptions[CAMERA_OPTIONS] = {
            "Follow player center",
            "Free camera movement",
        };

        DrawText("Controls:", 20, 20, 10, BLACK);
        DrawText("- A/D to move", 40, 40, 10, DARKGRAY);
        DrawText("- W/Space to jump", 40, 60, 10, DARKGRAY);
        DrawText("- Mouse Wheel to Zoom in-out, R to reset zoom", 40, 80, 10, DARKGRAY);
        DrawText("- C to change camera mode", 40, 100, 10, DARKGRAY);
        DrawText("Current camera mode:", 20, 120, 10, BLACK);
        DrawText(cameraDescriptions[m_cameraOption], 40, 140, 10, DARKGRAY);

        DrawFPS(340, 10);
    }

    EndDrawing();

	return 1;
}

void GameScene::GameScene::UpdatePlayerCamera()
{
    ZoneScopedC(RandomUniqueColor());

    if (!m_scene.IsEntity(m_cameraEntity))
    {
		bool found = false;

        // Find camera entity by CameraData component
		auto view = m_scene.GetRegistry().view<ECS::CameraData>();
		for (entt::entity entity : view)
		{
			// Set the active camera to the first camera entity found
			found = true;
			m_cameraEntity = entity; 
			break;
		}

        if (!found)
			return; // No camera entity found, nowhere to move the camera position to
    }

	ECS::Transform &transform = m_scene.GetComponent<ECS::Transform>(m_cameraEntity);
	ECS::CameraData &cameraData = m_scene.GetComponent<ECS::CameraData>(m_cameraEntity);

	m_camera.zoom = cameraData.Zoom;

    m_camera.offset = raylib::Vector2{ m_windowInfo->p_screenWidth / 2.0f, m_windowInfo->p_screenHeight / 2.0f };
    m_camera.target = raylib::Vector2(transform.Position[0], transform.Position[1]);
}
void GameScene::GameScene::UpdateFreeCamera()
{
    ZoneScopedC(RandomUniqueColor());
    
    raylib::Vector2 move(
        Input::CheckKeyHeld(Input::GAME_KEY_RIGHT) - Input::CheckKeyHeld(Input::GAME_KEY_LEFT),
        Input::CheckKeyHeld(Input::GAME_KEY_DOWN)  - Input::CheckKeyHeld(Input::GAME_KEY_UP)
    );

    move = Vector2Normalize(move);

    float deltaSpeed = m_freeCam.speed * Time::DeltaTime();
    m_freeCam.position = Vector2Add(
        m_freeCam.position, 
        Vector2Scale(move, deltaSpeed)
    );
    
	m_camera.target = m_freeCam.position;
}