#include "stdafx.h"
#include "main3D.h"
#include "../LuaConsole.h"

namespace Main3D
{
#define MAX_COLUMNS 500

    //------------------------------------------------------------------------------------
    // Game loop
    //------------------------------------------------------------------------------------
    int Run()
    {
        entt::registry registry;

        lua_State *L = luaL_newstate();
        luaL_openlibs(L);

        if (luaL_dostring(L, "print('Hello from Lua!')") != LUA_OK)
        {
            DumpLuaError(L);
        }

        // Initialization
        //--------------------------------------------------------------------------------------
        const int screenWidth = 1600;
        const int screenHeight = 900;

        SetConfigFlags(FLAG_MSAA_4X_HINT);
        InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera first person");

        // Define the camera to look into our 3d world (position, target, up vector)
        raylib::Camera camera{};
        camera.position = { 0.0f, 2.0f, 4.0f };    // Camera position
        camera.target = { 0.0f, 2.0f, 0.0f };      // Camera looking at point
        camera.up = { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
        camera.fovy = 60.0f;                                // Camera field-of-view Y
        camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

        int cameraMode = CAMERA_FIRST_PERSON;

        // Generates some random column entities
        for (int i = 0; i < MAX_COLUMNS; i++)
        {
            float height = ((float)GetRandomValue(10, 180)) / 10.0f;
            raylib::Vector3 position = { ((float)GetRandomValue(-3500, 3500)) / 100.0f, height / 2.0f, ((float)GetRandomValue(-3500, 3500)) / 100.0f };
            raylib::Color color = { (uint8_t)GetRandomValue(20, 255), (uint8_t)GetRandomValue(10, 55), 30, 255 };

            raylib::Matrix matrix = raylib::Matrix::Translate(position.x, position.y, position.z);

            // Create entity
            auto ent = registry.create();

            // Add components
            registry.emplace<Component::Transform>(ent, matrix);
            registry.emplace<Component::Render>(ent, color, true);
            registry.emplace<Component::Cube>(ent, raylib::Vector3{ 0.0f, 0.0f, 0.0f }, raylib::Vector3{ 0.2f, height, 0.2f });
        }

        DisableCursor();                    // Limit cursor to relative movement inside the window
        bool cursorEnabled = false;

        //SetTargetFPS(144);                   // Set our game to run at 144 frames-per-second
        //--------------------------------------------------------------------------------------

        // Load content
        raylib::Model model = LoadModel("res/Meshes/Maxwell.obj");
        raylib::Texture2D texture = LoadTexture("res/Textures/Maxwell.png");
        GenTextureMipmaps(&texture);
        SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);

        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

        // Start Lua console thread
        std::thread consoleThread(ConsoleThreadFunction, L);
        consoleThread.detach();

        // Main game loop
        while (!WindowShouldClose())        // Detect window close button or ESC key
        {
            // Update
            //----------------------------------------------------------------------------------
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

            // Switch camera mode
            if (IsKeyPressed(KEY_ONE))
            {
                cameraMode = CAMERA_FREE;
                camera.up = { 0.0f, 1.0f, 0.0f }; // Reset roll
            }

            if (IsKeyPressed(KEY_TWO))
            {
                cameraMode = CAMERA_FIRST_PERSON;
                camera.up = { 0.0f, 1.0f, 0.0f }; // Reset roll
            }

            if (IsKeyPressed(KEY_THREE))
            {
                cameraMode = CAMERA_THIRD_PERSON;
                camera.up = { 0.0f, 1.0f, 0.0f }; // Reset roll
            }

            if (IsKeyPressed(KEY_FOUR))
            {
                cameraMode = CAMERA_ORBITAL;
                camera.up = { 0.0f, 1.0f, 0.0f }; // Reset roll
            }

            // Switch camera projection
            if (IsKeyPressed(KEY_P))
            {
                if (camera.projection == CAMERA_PERSPECTIVE)
                {
                    // Create isometric view
                    cameraMode = CAMERA_THIRD_PERSON;
                    // Note: The target distance is related to the render distance in the orthographic projection
                    camera.position = { 0.0f, 2.0f, -100.0f };
                    camera.target = { 0.0f, 2.0f, 0.0f };
                    camera.up = { 0.0f, 1.0f, 0.0f };
                    camera.projection = CAMERA_ORTHOGRAPHIC;
                    camera.fovy = 20.0f; // near plane width in CAMERA_ORTHOGRAPHIC
                    CameraYaw(&camera, -135 * DEG2RAD, true);
                    CameraPitch(&camera, -45 * DEG2RAD, true, true, false);
                }
                else if (camera.projection == CAMERA_ORTHOGRAPHIC)
                {
                    // Reset to default view
                    cameraMode = CAMERA_THIRD_PERSON;
                    camera.position = { 0.0f, 2.0f, 10.0f };
                    camera.target = { 0.0f, 2.0f, 0.0f };
                    camera.up = { 0.0f, 1.0f, 0.0f };
                    camera.projection = CAMERA_PERSPECTIVE;
                    camera.fovy = 60.0f;
                }
            }

            // Update camera computes movement internally depending on the camera mode
            // Some default standard keyboard/mouse inputs are hardcoded to simplify use
            // For advanced camera controls, it's recommended to compute camera movement manually
            UpdateCamera(&camera, cameraMode);                  // Update camera

            /*
                    // Camera PRO usage example (EXPERIMENTAL)
                    // This new camera function allows custom movement/rotation values to be directly provided
                    // as input parameters, with this approach, rcamera module is internally independent of raylib inputs
                    UpdateCameraPro(&camera,
                        (Vector3){
                            (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))*0.1f -      // Move forward-backward
                            (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))*0.1f,
                            (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))*0.1f -   // Move right-left
                            (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))*0.1f,
                            0.0f                                                // Move up-down
                        },
                        (Vector3){
                            GetMouseDelta().x*0.05f,                            // Rotation: yaw
                            GetMouseDelta().y*0.05f,                            // Rotation: pitch
                            0.0f                                                // Rotation: roll
                        },
                        GetMouseWheelMove()*2.0f);                              // Move to target (zoom)
            */
            //----------------------------------------------------------------------------------

            // Draw
            //----------------------------------------------------------------------------------
            BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

            DrawPlane({ 0.0f, 0.0f, 0.0f }, { 72.0f, 72.0f }, LIGHTGRAY);  // Draw ground
            DrawCube({ -36.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 72.0f, BLUE);     // Draw a blue wall
            DrawCube({ 36.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 72.0f, LIME);      // Draw a green wall
            DrawCube({ 0.0f, 2.5f, 36.0f }, 72.0f, 5.0f, 1.0f, GOLD);      // Draw a yellow wall

            // Draw all entities with Render component
            {
                // Draw entities with render and cube, not transform
                {
                    auto view = registry.view<Component::Render, Component::Cube>(entt::exclude<Component::Transform>);

                    view.each([](const Component::Render &render, const Component::Cube &cube) {
                        if (!render.visible)
                            return;

                        raylib::Vector3 pos = cube.position;
                        raylib::Vector3 size = cube.size;
                        raylib::Color color = render.color;

                        DrawCubeV(pos, size, color);
                        //DrawCubeWiresV(pos, size, MAROON);
                        });
                }

                // Draw entities with transform, render and cube
                {
                    auto view = registry.view<Component::Transform, Component::Render, Component::Cube>();

                    view.each([](const Component::Transform &transform, const Component::Render &render, const Component::Cube &cube) {
                        if (!render.visible)
                            return;

                        raylib::Vector3 pos = cube.position;
                        raylib::Vector3 size = cube.size;
                        raylib::Color color = render.color;

                        pos = Vector3Transform(pos, transform.transform);

                        DrawCubeV(pos, size, color);
                        //DrawCubeWiresV(pos, size, MAROON);
                        });
                }
            }

            // Draw player cube
            if (cameraMode == CAMERA_THIRD_PERSON)
            {
                DrawCube(camera.target, 0.5f, 0.5f, 0.5f, PURPLE);
                DrawCubeWires(camera.target, 0.5f, 0.5f, 0.5f, DARKPURPLE);
            }

            DrawModel(model, { 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);

            EndMode3D();

            // Draw info boxes
            DrawRectangle(5, 5, 330, 100, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(5, 5, 330, 100, BLUE);

            DrawText("Camera controls:", 15, 15, 10, BLACK);
            DrawText("- Move keys: W, A, S, D, Space, Left-Ctrl", 15, 30, 10, BLACK);
            DrawText("- Look around: arrow keys or mouse", 15, 45, 10, BLACK);
            DrawText("- Camera mode keys: 1, 2, 3, 4", 15, 60, 10, BLACK);
            DrawText("- Zoom keys: num-plus, num-minus or mouse scroll", 15, 75, 10, BLACK);
            DrawText("- Camera projection key: P", 15, 90, 10, BLACK);

            DrawRectangle(600, 5, 195, 100, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(600, 5, 195, 100, BLUE);

            DrawText("Camera status:", 610, 15, 10, BLACK);
            DrawText(TextFormat("- Mode: %s", (cameraMode == CAMERA_FREE) ? "FREE" :
                (cameraMode == CAMERA_FIRST_PERSON) ? "FIRST_PERSON" :
                (cameraMode == CAMERA_THIRD_PERSON) ? "THIRD_PERSON" :
                (cameraMode == CAMERA_ORBITAL) ? "ORBITAL" : "CUSTOM"), 610, 30, 10, BLACK);
            DrawText(TextFormat("- Projection: %s", (camera.projection == CAMERA_PERSPECTIVE) ? "PERSPECTIVE" :
                (camera.projection == CAMERA_ORTHOGRAPHIC) ? "ORTHOGRAPHIC" : "CUSTOM"), 610, 45, 10, BLACK);
            DrawText(TextFormat("- Position: (%06.3f, %06.3f, %06.3f)", camera.position.x, camera.position.y, camera.position.z), 610, 60, 10, BLACK);
            DrawText(TextFormat("- Target: (%06.3f, %06.3f, %06.3f)", camera.target.x, camera.target.y, camera.target.z), 610, 75, 10, BLACK);
            DrawText(TextFormat("- Up: (%06.3f, %06.3f, %06.3f)", camera.up.x, camera.up.y, camera.up.z), 610, 90, 10, BLACK);

            DrawFPS(340, 10);

            EndDrawing();
            //----------------------------------------------------------------------------------
        }

        // De-Initialization
        //--------------------------------------------------------------------------------------
        CloseWindow();              // Close window and OpenGL context
        //--------------------------------------------------------------------------------------

        return 0;
    }
}