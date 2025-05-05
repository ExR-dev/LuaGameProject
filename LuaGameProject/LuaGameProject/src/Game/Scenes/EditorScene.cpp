#include "stdafx.h"
#include "EditorScene.h"

#include "../../LuaConsole.h"

#include "../Utilities/InputHandler.h"
#include "../Utilities/LuaInput.h"

EditorScene::EditorScene::~EditorScene()
{
    ZoneScopedC(RandomUniqueColor());

    Game::IsQuitting = true;

    if (L)
    {
        lua_close(L);
        L = nullptr;
    }

    ResourceManager::Instance().UnloadResources();
    CloseAudioDevice();
}

int EditorScene::EditorScene::Run()
{
    ZoneScopedC(RandomUniqueColor());

    Start();
    FrameMark;

    while (!WindowShouldClose())
    {
        ZoneNamedNC(innerLoopZone, "EditorScene::EditorScene::Run Loop", RandomUniqueColor(), true);

        Time::Update();
        Input::UpdateInput();

        Update();

        Render();

        FrameMark;
    }

    CloseWindow();
    FrameMark;
    return 0;
}

int EditorScene::EditorScene::Start()
{
    ZoneScopedC(RandomUniqueColor());

    // Setup Box2D
    m_physicsHandler.Setup();

    // Setup Lua enviroment

    // Create internal lua state
    L = luaL_newstate();
    luaL_openlibs(L);

    m_windowInfo.BindLuaWindow(L);

    Scene::lua_openscene(L, &m_scene);

    m_luaGame = LuaGame::LuaGame(L, &m_scene);
    LuaGame::LuaGame::lua_opengame(L, &m_luaGame);

    // Create console-bound lua state
    lua_State *consoleL = luaL_newstate();
    luaL_openlibs(consoleL);
    Scene::lua_openscene(consoleL, &m_scene);

    InitWindow(m_windowInfo.p_screenWidth, m_windowInfo.p_screenHeight, "Lua Game");
    InitAudioDevice();

    Time::Instance();
    ResourceManager::Instance().LoadResources();

    m_camera.target = raylib::Vector2(0, 0);
    m_camera.offset = raylib::Vector2(m_windowInfo.p_screenWidth / 2.0f, m_windowInfo.p_screenHeight / 2.0f);
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;

    BindLuaInput(L);

    SetTargetFPS(144);

    // Add lua require path
    std::string luaScriptPath = std::format("{}/{}?{}", fs::current_path().generic_string(), FILE_PATH, FILE_EXT);
    LuaDoString(std::format("package.path = \"{};\" .. package.path", luaScriptPath).c_str());

    // Initialize Lua
    LuaDoFileCleaned(L, LuaFilePath("Data")); // Load data
    // TODO: Reuse code for running tests to autmoatically run all lua files located in Data

    m_scene.SystemsInitialize(L);

    LuaDoFileCleaned(L, LuaFilePath("InitDevScene")); // Creates entities

    return 1;
}

int EditorScene::EditorScene::Update()
{
    ZoneScopedC(RandomUniqueColor());

    // Update systems
    m_scene.SystemsOnUpdate(Time::DeltaTime());

    // Update Physics
    m_physicsHandler.Update(L, &m_scene);

    std::function<void(entt::registry& registry)> createPhysicsBodies = [&](entt::registry& registry) {
        ZoneNamedNC(createPhysicsBodiesZone, "Lambda Create Physics Bodies", RandomUniqueColor(), true);

        auto view = registry.view<ECS::Collider, ECS::Transform>();
        view.use<ECS::Collider>();

        view.each([&](const entt::entity entity, ECS::Collider& collider, ECS::Transform& transform) {
            ZoneNamedNC(drawSpriteZone, "Lambda Create Physics Bodies", RandomUniqueColor(), true);

            // Create body
            if (!collider.createBody)
            {
                b2Body_SetTransform(collider.bodyId, { transform.Position[0], transform.Position[1] }, { cosf(transform.Rotation * DEG2RAD), sinf(transform.Rotation * DEG2RAD) });
            }
            else
            {
                collider.bodyId = m_physicsHandler.CreateRigidBody(static_cast<int>(entity), collider, transform);
                collider.createBody = false;
            }
        });
    };

    m_scene.RunSystem(createPhysicsBodies);

    m_scene.CleanUp();

    return 0;
}

int EditorScene::EditorScene::Render()
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

        EndMode2D();
    }

    // UI
    {
        ZoneNamedNC(EditorSceneRenderScene, "Render UI", RandomUniqueColor(), true);

        DrawFPS(340, 10);
    }

    EndDrawing();

    return 1;
}
