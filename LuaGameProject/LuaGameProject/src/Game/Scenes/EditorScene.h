#pragma once
#include "dep/raylib-cpp/raylib-cpp.hpp"
#include "../Scene.h"
#include "SceneTemplate.h"

#include "../Utilities/WindowInfo.h"
#include "../Utilities/LuaGame.h"
#include "../Utilities/DungeonGenerator.h"
#include "../PhysicsHandler.h"


namespace EditorScene
{
	class EditorScene : public SceneTemplate::SceneTemplate
	{
	public:
		EditorScene();
		~EditorScene();

		int Start(WindowInfo *windowInfo, CmdState *cmdState, raylib::RenderTexture *screenRT) override;
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


		struct EditorModeLuaUI
		{
		public:
			void Create(lua_State *L, const char *path)
			{
				if (luaRef != LUA_NOREF)
					Destroy(L);

				// Load the weapon editor UI script
				LuaDoFileCleaned(L, LuaFilePath(path));
				lua_pushvalue(L, -1);
				luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
			}

			void Destroy(lua_State *L)
			{
				if (luaRef != LUA_NOREF)
				{
					luaL_unref(L, LUA_REGISTRYINDEX, luaRef);
					luaRef = LUA_NOREF;
				}
			}

			void Run(lua_State *L, const char *funcName) const
			{
				// Retrieve the editor table to the top of the stack
				lua_rawgeti(L, LUA_REGISTRYINDEX, luaRef);

				// Retrieve the requested method from the table
				lua_getfield(L, -1, funcName);

				// Check if the method exists before calling it
				if (lua_isnil(L, -1))
				{
					lua_pop(L, 1); // Pop nil
				}
				else
				{
					// Push the table as the first argument to the method
					lua_pushvalue(L, -2);

					// Call the method, pops the method and its arguments from the stack
					LuaChk(L, lua_pcall(L, 1, 0, 0));
				}

				// Pop the editor table from the stack
				lua_pop(L, 1);
			}

		private:
			int luaRef = LUA_NOREF;

		};

		struct EditorModeScene
		{
			lua_State *L = nullptr;
			PhysicsHandler physicsHandler{};
			Scene scene{};
			LuaGame::LuaGame luaGame{};
			EditorModeLuaUI luaUI{};

			~EditorModeScene()
			{
				luaUI.Destroy(L);

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

		DungeonGenerator *m_dungeon = nullptr;

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
