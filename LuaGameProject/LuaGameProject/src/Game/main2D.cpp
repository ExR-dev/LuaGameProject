#include "stdafx.h"
#include "main2D.h"
#include "../LuaConsole.h"

#include "Utilities/DungeonGenerator.h"
#include "Scene.h"


namespace Main2D
{
    constexpr float PLAYER_HOR_SPD = 200.0f;

    typedef struct Player {
        raylib::Vector2 position;
        float speed;
    } Player;

    typedef struct EnvItem {
        raylib::Rectangle rect;
        int blocking;
        bool platform;
        raylib::Color color;
    } EnvItem;

    //------------------------------------------------------------------------------------
	// Forward declaration
    //------------------------------------------------------------------------------------
    void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength, float delta);
    void UpdateCameraCenter(raylib::Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);
    void UpdateCameraCenterInsideMap(raylib::Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);
    void UpdateFreeCamera(raylib::Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);

    //------------------------------------------------------------------------------------
    // Game
    //------------------------------------------------------------------------------------
    int Run()
    {
        // Construction
        //--------------------------------------------------------------------------------------
        lua_State *L = luaL_newstate();
        luaL_openlibs(L);

        Scene scene(L);
        Scene::lua_openscene(L, &scene);

        Time::Instance();
        //--------------------------------------------------------------------------------------

        // Initialization
        //--------------------------------------------------------------------------------------
        const int screenWidth = 1280;
        const int screenHeight = 720;

        InitWindow(screenWidth, screenHeight, "raylib [core] example - 2d camera");

        Player player = { 0 };
        player.position = raylib::Vector2{ 400, 280 };
        player.speed = 0;
        EnvItem envItems[] = {
            {{ 0, 0, 1000, 400 }, 0, false, LIGHTGRAY }, // Player
            {{ 0, 400, 1000, 200 }, 1, false, GRAY }, // Floor
            {{ 300, 200, 400, 10 }, 1, true, GRAY }, // Platform
            {{ 250, 300, 100, 10 }, 1, true, GRAY }, // Platform
            {{ 650, 300, 100, 10 }, 1, true, GRAY } // Platform
        };

        int envItemsLength = sizeof(envItems) / sizeof(envItems[0]);

        raylib::Camera2D camera = {};
        camera.target = player.position;
        camera.offset = raylib::Vector2{ screenWidth / 2.0f, screenHeight / 2.0f };
        camera.rotation = 0.0f;
        camera.zoom = 1.0f;

        DungeonGenerator dungeon = DungeonGenerator({ 200, 200 });
        dungeon.Generate(100);

        // Store pointers to the multiple update camera functions
        void (*cameraUpdaters[])(raylib::Camera2D *, Player *, EnvItem *, int, float, int, int) = {
            UpdateCameraCenter,
            UpdateFreeCamera,
            UpdateCameraCenterInsideMap
        };

        int cameraOption = 0;
        int cameraUpdatersLength = sizeof(cameraUpdaters) / sizeof(cameraUpdaters[0]);

        const char *cameraDescriptions[] = {
            "Follow player center",
            "Free camera movement",
            "Follow player center, but clamp to map edges"
        };
        
        // Limit cursor to relative movement inside the window
        DisableCursor();                    
        bool cursorEnabled = false;

        SetTargetFPS(144);

        // Start Lua console thread
        std::thread consoleThread(ConsoleThreadFunction, L);
        consoleThread.detach();
        //--------------------------------------------------------------------------------------

        // Main game loop
        while (!WindowShouldClose())
        {
            // Update
            //----------------------------------------------------------------------------------
            Time::Update();

            // Update all systems
            scene.UpdateSystems(Time::DeltaTime());


            // Toggle mouse
            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
            {
                if (cursorEnabled)
                {
                    DisableCursor();
                    cursorEnabled = false;
                }
                else
                {
                    EnableCursor();
                    cursorEnabled = true;
                }
            }

            UpdatePlayer(&player, envItems, envItemsLength, Time::DeltaTime());

            camera.zoom += ((float)GetMouseWheelMove() * 0.05f);

            if (camera.zoom > 3.0f) camera.zoom = 3.0f;
            else if (camera.zoom < 0.25f) camera.zoom = 0.25f;

            if (IsKeyPressed(KEY_R))
            {
                camera.zoom = 1.0f;
                player.position = raylib::Vector2{ 400, 280 };
            }

            if (IsKeyPressed(KEY_T))
            {
                dungeon.Initialize();
                dungeon.Generate(100);
            }

				if (IsKeyPressed(KEY_Y)) dungeon.SeparateRooms();

            if (IsKeyPressed(KEY_C)) cameraOption = (cameraOption + 1) % cameraUpdatersLength;

            // Call update camera function by its pointer
            cameraUpdaters[cameraOption](&camera, &player, envItems, envItemsLength, Time::DeltaTime(), screenWidth, screenHeight);
            //----------------------------------------------------------------------------------

            // Draw
            //----------------------------------------------------------------------------------
            BeginDrawing();

            ClearBackground(LIGHTGRAY);

            BeginMode2D(camera);

            for (int i = 0; i < envItemsLength; i++) 
                DrawRectangleRec(envItems[i].rect, envItems[i].color);

            raylib::Rectangle playerRect = { player.position.x - 20, player.position.y - 40, 40.0f, 40.0f };
            DrawRectangleRec(playerRect, RED);

            DrawCircleV(player.position, 5.0f, GOLD);

            dungeon.Draw();

            EndMode2D();

            DrawText("Controls:", 20, 20, 10, BLACK);
            DrawText("- A/D to move", 40, 40, 10, DARKGRAY);
            DrawText("- W/Space to jump", 40, 60, 10, DARKGRAY);
            DrawText("- Mouse Wheel to Zoom in-out, R to reset zoom", 40, 80, 10, DARKGRAY);
            DrawText("- C to change camera mode", 40, 100, 10, DARKGRAY);
            DrawText("Current camera mode:", 20, 120, 10, BLACK);
            DrawText(cameraDescriptions[cameraOption], 40, 140, 10, DARKGRAY);

            DrawFPS(340, 10);

            EndDrawing();
            //----------------------------------------------------------------------------------
        }

        // De-Initialization
        //--------------------------------------------------------------------------------------
        CloseWindow();        // Close window and OpenGL context
        //--------------------------------------------------------------------------------------

        return 0;
    }

    //------------------------------------------------------------------------------------
    // Module functions declaration
    //------------------------------------------------------------------------------------
    void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength, float delta)
    {
        if (IsKeyDown(KEY_A)) player->position.x -= PLAYER_HOR_SPD * delta;
        if (IsKeyDown(KEY_D)) player->position.x += PLAYER_HOR_SPD * delta;
        if (IsKeyDown(KEY_W)) player->position.y -= PLAYER_HOR_SPD * delta;
        if (IsKeyDown(KEY_S)) player->position.y += PLAYER_HOR_SPD * delta;

        bool hitObstacle = false;
        /*for (int i = 0; i < envItemsLength; i++)
        {
            EnvItem *ei = envItems + i;
            raylib::Vector2 *p = &(player->position);
            if (ei->blocking &&
                ei->rect.x <= p->x &&
                ei->rect.x + ei->rect.width >= p->x &&
                ei->rect.y >= p->y &&
                ei->rect.y <= p->y + player->speed * delta)
            {
                hitObstacle = true;
                player->speed = 0.0f;
                p->y = ei->rect.y;
                break;
            }
        }*/

        if (!hitObstacle)
        {
            player->position.y += player->speed * delta;
            //player->speed += G * delta;
            //player->canJump = false;
        }
        /*else 
            player->canJump = true;*/
    }

    void UpdateCameraCenter(raylib::Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height)
    {
        camera->offset = raylib::Vector2{ width / 2.0f, height / 2.0f };
        camera->target = player->position;
    }

    void UpdateCameraCenterInsideMap(raylib::Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height)
    {
        camera->target = player->position;
        camera->offset = raylib::Vector2{ width / 2.0f, height / 2.0f };
        float minX = 1000, minY = 1000, maxX = -1000, maxY = -1000;

        for (int i = 0; i < envItemsLength; i++)
        {
            EnvItem *ei = envItems + i;
            minX = fminf(ei->rect.x, minX);
            maxX = fmaxf(ei->rect.x + ei->rect.width, maxX);
            minY = fminf(ei->rect.y, minY);
            maxY = fmaxf(ei->rect.y + ei->rect.height, maxY);
        }

        raylib::Vector2 max = GetWorldToScreen2D(raylib::Vector2{ maxX, maxY }, *camera);
        raylib::Vector2 min = GetWorldToScreen2D(raylib::Vector2{ minX, minY }, *camera);

        if (max.x < width) camera->offset.x = width - (max.x - width / 2);
        if (max.y < height) camera->offset.y = height - (max.y - height / 2);
        if (min.x > 0) camera->offset.x = width / 2 - min.x;
        if (min.y > 0) camera->offset.y = height / 2 - min.y;
    }

    void UpdateFreeCamera(raylib::Camera2D* camera, Player* player, EnvItem* envItems, int envItemsLength, float delta, int width, int height)
    {
        raylib::Vector2 move(
            IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT),
            IsKeyDown(KEY_DOWN)  - IsKeyDown(KEY_UP)
        );

        move = Vector2Normalize(move);

        const static float moveSpeed = 100;

        camera->target = Vector2Add(camera->target, Vector2Scale(move, moveSpeed * delta));
    }

}