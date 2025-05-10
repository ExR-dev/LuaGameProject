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

		int Start(WindowInfo *windowInfo, CmdState *cmdState) override;
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

		enum SceneUpdateMode {
			Paused, // No updates
			Frozen, // Updates with delta time set to 0
			Running // Normal updates
		} m_sceneUpdateMode = Paused;

		raylib::RenderTexture m_renderTexture;

		bool m_sceneViewOpen = true;
		raylib::Rectangle m_sceneViewRect{};

		enum EditorMode { // TODO: Implement
			Sandbox, // Spawn entities to see their interactions
			PresetCreator, // weapons, ammo, enemies, etc
			PrefabCreator, // entities/groups of entities
			LevelCreator
		} m_editorMode = Sandbox;

		int m_selectedEntity = -1;


		int RenderUI();

		raylib::Vector2 ScreenToWorldPos(const raylib::Vector2 &pos) const;
		bool IsWithinSceneView(const raylib::Vector2 &pos) const;
	};
}
