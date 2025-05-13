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

		enum SceneUpdateMode {
			Paused, // No updates
			Frozen, // Updates with delta time set to 0
			Running // Normal updates
		} m_sceneUpdateMode = Frozen;

		enum EditorMode {
			Sandbox, // Spawn entities to see their interactions
			LevelCreator,
			PresetCreator, // weapons, ammo, enemies, etc
			PrefabCreator, // entities/groups of entities
			DungeonCreator, // Create and generate dungeons
			COUNT
		} m_editorMode = Sandbox;

		std::string m_editorModeNames[EditorMode::COUNT] = {
			"Sandbox",
			"LevelCreator",
			"PresetCreator",
			"PrefabCreator",
			"DungeonCreator"
		};

		struct EditorModeScene
		{
			lua_State *L = nullptr;
			PhysicsHandler physicsHandler{};
			Scene scene{};
			LuaGame::LuaGame luaGame{};

			~EditorModeScene()
			{
				if (L)
				{
					lua_close(L);
					L = nullptr;
				}
			}

			void Init(WindowInfo *windowInfo, const std::string &name);
			void LoadData() const;
		};
		std::unique_ptr<EditorModeScene> m_editorModeScenes[EditorMode::COUNT];

		raylib::RenderTexture m_renderTexture;

		bool m_sceneViewOpen = true;
		raylib::Rectangle m_sceneViewRect{};

		int m_selectedEntity = -1;
		int m_selectedRoom = -1;

		bool m_isDraggingCamera = false;
		raylib::Vector2 m_dragOrigin = raylib::Vector2(0, 0);
		raylib::Vector2 m_dragOffset = raylib::Vector2(0, 0);


		int RenderUI();

		void SwitchEditorMode(EditorMode mode);

		raylib::Vector2 ScreenToWorldPos(const raylib::Vector2 &pos) const;
		raylib::Vector2 WorldToScreenPos(const raylib::Vector2 &pos) const;
		bool IsWithinSceneView(const raylib::Vector2 &pos) const;
	};
}
