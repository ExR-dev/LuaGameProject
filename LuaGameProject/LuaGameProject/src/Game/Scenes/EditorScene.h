#pragma once
#include "dep/raylib-cpp/raylib-cpp.hpp"
#include "../Scene.h"
#include "SceneTemplate.h"

#include "../Utilities/WindowInfo.h"
#include "../Utilities/LuaGame.h"
#include "../PhysicsHandler.h"

namespace EditorScene
{
	class EditorScene : public SceneTemplate::SceneTemplate
    {
    public:
        EditorScene();
        ~EditorScene();

        int Start(WindowInfo *windowInfo) override;
        Game::SceneState Loop() override;

    protected:
        Game::SceneState Update() override;
        int Render() override;

    private:
        lua_State *L = nullptr;
        Scene m_scene{};
        LuaGame::LuaGame m_luaGame;
        PhysicsHandler m_physicsHandler;

        int RenderUI();
    };
}
