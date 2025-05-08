#pragma once
#include "dep/raylib-cpp/raylib-cpp.hpp"

#include "SceneTemplate.h"
#include "GameScene.h"
#include "EditorScene.h"

namespace MenuScene
{
    struct MenuButton
    {
    public:
        std::string name;

        raylib::Rectangle rect;

        raylib::Color baseColor;
        raylib::Color hoverColor;
        raylib::Color pressColor;

        bool Update();
        void Render();
		bool IsActivated() const { return isHovered && isReleased; }

    private:
        raylib::Color color;

        bool isHovered = false;
        bool isPressed = false;
        bool isReleased = false;
    };

    class MenuScene : public SceneTemplate::SceneTemplate
    {
    public:
        MenuScene() = default;

        int Start(WindowInfo *windowInfo) override;
        Game::SceneState Loop() override;

    protected:
        Game::SceneState Update() override;
        int Render() override;

    private:
		MenuButton m_playButton{};
		MenuButton m_restartButton{};
		MenuButton m_editorButton{};
		MenuButton m_quitButton{};

		std::vector<MenuButton*> m_buttons{};
    };
}
