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

		void OnSwitchToScene() override;
		void OnResizeWindow() override;

	protected:
		Game::SceneState Update() override;
		int Render() override;

	private:
		lua_State *L = nullptr;
		Scene m_scene{};
		LuaGame::LuaGame m_luaGame;
		PhysicsHandler m_physicsHandler;

		raylib::RenderTexture m_renderTexture;

		bool m_sceneViewOpen = true;
		raylib::Rectangle m_sceneViewRect{};

		int RenderUI();


		raylib::Vector2 ScreenToWorldPos(const raylib::Vector2 &pos) const;
		bool IsWithinSceneView(const raylib::Vector2 &pos) const;
	};
}
