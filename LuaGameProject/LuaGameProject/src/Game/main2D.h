#pragma once
#include "dep/raylib-cpp/raylib-cpp.hpp"
#include "Scene.h"

constexpr float PLAYER_HOR_SPD = 200.0f;
constexpr int CAMERA_OPTIONS = 2;

class DungeonGenerator;

struct Player
{
    raylib::Vector2 position;
    float speed;
};

namespace Main2D
{
    class Main2D
    {
    public:
		Main2D() = default;
		~Main2D();

        int Run();

    private:
        lua_State *L = nullptr; // Not m_ because "m_L" feels wrong.
        Scene m_scene{};

		Player m_player;
		raylib::Camera2D m_camera;

        DungeonGenerator *m_dungeon = nullptr;

        bool m_cursorEnabled = false;

		int m_cameraOption = 0;
		std::function<void(void)> m_cameraUpdater;

        int m_screenWidth = 1280;
        int m_screenHeight = 720;

        int Start();
        int Update();
        int Render();

        void UpdatePlayer();
        void UpdateCameraCenter();
        void UpdateCameraFree();
    };
}
