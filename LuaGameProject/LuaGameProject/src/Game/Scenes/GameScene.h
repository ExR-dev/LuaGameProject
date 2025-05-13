#pragma once
#include "dep/raylib-cpp/raylib-cpp.hpp"
#include "../Scene.h"
#include "SceneTemplate.h"

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
    class GameScene : public SceneTemplate::SceneTemplate
    {
    public:
        GameScene();
        ~GameScene();

        int Start(WindowInfo *windowInfo, CmdState *cmdState, raylib::RenderTexture *screenRT) override;
        Game::SceneState Loop() override;

		void OnSwitchToScene() override;
		void OnResizeWindow() override;

    protected:
        Game::SceneState Update() override;
        int Render() override;

    private:
        lua_State *L = nullptr; // No m_ because "m_L" feels wrong.
        Scene m_scene{};
        LuaGame::LuaGame m_luaGame;

        FreeCam m_freeCam{};

        DungeonGenerator *m_dungeon = nullptr;

        bool m_cursorEnabled = false;

        entt::entity m_cameraEntity = entt::null;
		int m_cameraOption = 0;
		std::function<void(void)> m_cameraUpdater;

        PhysicsHandler m_physicsHandler;


        void UpdatePlayerCamera();
        void UpdateFreeCamera();
    };
}
