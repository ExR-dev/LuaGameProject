#pragma once
#include "dep/raylib-cpp/raylib-cpp.hpp"
#include "../Scene.h"
#include "SceneTemplate.h"
#include <atomic>

#include "../Utilities/WindowInfo.h"
#include "../Utilities/LuaGame.h"
#include "../PhysicsHandler.h"

constexpr float PLAYER_HOR_SPD = 200.0f;
constexpr int CAMERA_OPTIONS = 2;

class DungeonGenerator;

struct FreeCam
{
    raylib::Vector2 position;
    float speed;
};

namespace GameScene
{
    class GameScene : SceneTemplate::SceneTemplate
    {
    public:
        ~GameScene();

        int Start() override;
        Game::SceneState Loop() override;

    protected:
        int Update() override;
        int Render() override;

    private:
        lua_State *L = nullptr; // No m_ because "m_L" feels wrong.
        Scene m_scene{};
        LuaGame::LuaGame m_luaGame;

        FreeCam m_freeCam{};
        raylib::Camera2D m_camera{};

        DungeonGenerator *m_dungeon = nullptr;

        bool m_cursorEnabled = false;

        entt::entity m_cameraEntity = entt::null;
		int m_cameraOption = 0;
		std::function<void(void)> m_cameraUpdater;

        WindowInfo *m_windowInfo;

        std::string m_cmdList;
        std::atomic_bool m_pauseCmdInput = false;

        
        PhysicsHandler m_physicsHandler;
        b2BodyId m_box, m_ground;

        void UpdatePlayerCamera();
        void UpdateFreeCamera();
    };
}
