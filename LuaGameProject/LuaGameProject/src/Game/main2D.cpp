#include "stdafx.h"
#include "main2D.h"

#include "../LuaConsole.h"
#include "Utilities/DungeonGenerator.h"
#include "Scene.h"

#include "Utilities/InputHandler.h"
#include "Utilities/LuaInput.h"

namespace Main2D
{
    Main2D::~Main2D()
    {
		if (m_dungeon)
		{
			delete m_dungeon;
			m_dungeon = nullptr;
		}
    }

    int Main2D::Run()
    {
        Start();

        while (!WindowShouldClose())
        {
            Time::Update();

			Update();

			Render();
        }

        CloseWindow();
        return 0;
    }


    int Main2D::Start()
    {
        L = luaL_newstate();
        luaL_openlibs(L);

        Scene::lua_openscene(L, &m_scene);

        Time::Instance();

        InitWindow(m_screenWidth, m_screenHeight, "Lua Game");

        m_player = { 0 };
        m_player.position = raylib::Vector2(400, 280);
        m_player.speed = 0;

        m_camera = {};
        m_camera.target = m_player.position;
        m_camera.offset = raylib::Vector2(m_screenWidth / 2.0f, m_screenHeight / 2.0f);
        m_camera.rotation = 0.0f;
        m_camera.zoom = 1.0f;

        Assert(!m_dungeon, "m_dungeon is not nullptr!");
        m_dungeon = new DungeonGenerator(raylib::Vector2(200, 200));
        m_dungeon->Generate(100);

		m_cameraUpdater = std::bind(&Main2D::UpdateCameraCenter, this);
		m_cameraOption = 0;

        // Limit cursor to relative movement inside the window
        DisableCursor();                    
        bool cursorEnabled = false;
        
        BindLuaInput(L);

        SetTargetFPS(144);

        // Start Lua console thread
        std::thread consoleThread(ConsoleThreadFunction);
        consoleThread.detach();
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Wait for the console thread to start

        m_scene.InitializeSystems();

        LuaDoFile(LuaFilePath("InitDevScene")) // Creates entities

        return 1;
    }

    int Main2D::Update()
    {
        // Toggle mouse
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        {
            // Update
            //----------------------------------------------------------------------------------
            Time::Update();

            Input::UpdateInput();

            // Update all systems
            scene.UpdateSystems(Time::DeltaTime());

            // Toggle mouse
            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
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

        UpdatePlayer();

        m_camera.zoom += ((float)GetMouseWheelMove() * 0.05f);

        if (m_camera.zoom > 3.0f) m_camera.zoom = 3.0f;
        else if (m_camera.zoom < 0.25f) m_camera.zoom = 0.25f;

        if (IsKeyPressed(KEY_R))
        {
            m_camera.zoom = 1.0f;
            m_player.position = raylib::Vector2(400, 280);
        }

        if (IsKeyPressed(KEY_T))
        {
            m_dungeon->Initialize();
            m_dungeon->Generate(100);
        }

        if (IsKeyPressed(KEY_Y)) 
            m_dungeon->SeparateRooms();

        if (IsKeyPressed(KEY_C)) 
        {
            m_cameraOption = (m_cameraOption + 1) % CAMERA_OPTIONS;

            switch (m_cameraOption)
            {
            case 0: default:
                m_cameraUpdater = std::bind(&Main2D::UpdateCameraCenter, this);
                break;

			case 1:
                m_cameraUpdater = std::bind(&Main2D::UpdateCameraFree, this);
                break;
            }
        }

        // Call update camera function by its pointer
        m_cameraUpdater();

        // Update systems
        m_scene.UpdateSystems(Time::DeltaTime());

        return 0;
    }

    int Main2D::Render()
    {
        BeginDrawing();
        ClearBackground(LIGHTGRAY);

        // Scene
        {
            BeginMode2D(m_camera);

            // Draw entities
            std::function<void(entt::registry &registry)> drawSystem = [](entt::registry &registry) {
                auto view = registry.view<ECS::Sprite, ECS::Transform>();
                view.use<ECS::Sprite>();

                view.each([&](const ECS::Sprite &sprite, const ECS::Transform &transform) {
                    // Draw the sprite at the location defined by the transform.

                    const float posX = transform.Position[0];
                    const float posY = transform.Position[1];

                    const float sclX = transform.Scale[0];
                    const float sclY = transform.Scale[1];

                    raylib::Color color(*(raylib::Vector4 *)(&(sprite.Color)));

                    DrawRectangle((int)posX, (int)posY, (int)sclX, (int)sclY, color);
                });
            };
			m_scene.RunSystem(drawSystem);

			// Draw the dungeon
            m_dungeon->Draw();

			// Draw the player
			DrawCircle((int)m_player.position.x, (int)m_player.position.y, 15, raylib::Color(raylib::Vector4(0.5f, 0.8f, 0.2f, 1.0f)));

            EndMode2D();
        }

        // UI
        {
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
        // De-Initialization
        //--------------------------------------------------------------------------------------
        CloseWindow();        // Close window and OpenGL context
        //--------------------------------------------------------------------------------------

		return 1;
    }


    void Main2D::UpdatePlayer()
    {
        if (Input::CheckKeyHeld(Input::GAME_KEY_A)) player->position.x -= PLAYER_HOR_SPD * delta;
        if (Input::CheckKeyHeld(Input::GAME_KEY_D)) player->position.x += PLAYER_HOR_SPD * delta;
        if (Input::CheckKeyHeld(Input::GAME_KEY_W)) player->position.y -= PLAYER_HOR_SPD * delta;
        if (Input::CheckKeyHeld(Input::GAME_KEY_S)) player->position.y += PLAYER_HOR_SPD * delta;

        m_player.position.y += m_player.speed * delta;
    }
    void Main2D::UpdateCameraCenter()
    {
        m_camera.offset = raylib::Vector2{ m_screenWidth / 2.0f, m_screenHeight / 2.0f };
        m_camera.target = m_player.position;
    }
    void Main2D::UpdateCameraFree()
    {
        float delta = Time::DeltaTime();

        raylib::Vector2 move(
            Input::CheckKeyHeld(Input::GAME_KEY_RIGHT) - Input::CheckKeyHeld(Input::GAME_KEY_LEFT),
            Input::CheckKeyHeld(Input::GAME_KEY_DOWN)  - Input::CheckKeyHeld(Input::GAME_KEY_UP)
        );

        move = Vector2Normalize(move);

        const static float moveSpeed = 100;

        m_camera.target = Vector2Add(m_camera.target, Vector2Scale(move, moveSpeed * delta));
    }
}