#pragma once
#include <cstring>
#include <string>
#include <vector>
#include "Game/Game.h"
#include "lua.hpp"
#include "LuaUtils.h"

#include "Game/Utilities/WindowsWrapped.h"

#include "box2d/box2D.h"


namespace ECS
{
	struct Active
	{
		bool IsActive = true;

		void LuaPush(lua_State *L) const
		{
			ZoneScopedC(RandomUniqueColor());
			// Create the main active table
			lua_createtable(L, 0, 1);

			// Add IsActive to the active table
			lua_pushnumber(L, IsActive);
			lua_setfield(L, -2, "isActive");
		}
		void LuaPull(lua_State *L, int index)
		{
			ZoneScopedC(RandomUniqueColor());
			// Make sure the index is absolute (in case it's negative)
			if (index < 0)
			{
				index = lua_gettop(L) + index + 1;
			}

			// Verify that the value at the given index is a table
			if (!lua_istable(L, index))
			{
				luaL_error(L, "Expected a table for Active");
				return;
			}

			// Get Max field
			lua_getfield(L, index, "isActive");
			if (lua_isboolean(L, -1))
			{
				IsActive = lua_toboolean(L, -1);
			}
			lua_pop(L, 1); // Remove the isActive value from stack
		}

		void RenderUI()
		{
			ImGui::Checkbox("Active", &IsActive);
		}
	};

	struct Behaviour
	{
	public:
		static constexpr int SCRIPT_PATH_LENGTH = 65;
		char ScriptPath[SCRIPT_PATH_LENGTH];
		int LuaRef;


		// Create a constructor in order to initialize the char array
		Behaviour(const char *path, int entity, lua_State *L) : m_refState(L), m_entity(entity)
		{
			Initialize(path);
		}

		void Initialize(const char *path)
		{
			lua_State *L = m_refState;
			ZoneScopedC(RandomUniqueColor());

			// Returns the behaviour table on top of the stack
			LuaDoFileCleaned(L, LuaFilePath(path));

			// luaL_ref pops the value of the stack, so we push the table again before luaL_ref
			lua_pushvalue(L, -1);
			LuaRef = luaL_ref(L, LUA_REGISTRYINDEX);

			// Populate the behaviour table with the information the behaviour should know about
			lua_pushinteger(L, m_entity);
			lua_setfield(L, -2, "ID");

			lua_pushstring(L, path);
			lua_setfield(L, -2, "path");

			// Let the behaviour construct itself
			lua_getfield(L, -1, "OnCreate");

			// Check if the method exists before calling it
			if (lua_isnil(L, -1))
			{
				lua_pop(L, 1); // Pop nil
			}
			else
			{
				lua_pushvalue(L, -2); // Push the table as argument
				LuaChk(lua_pcall(L, 1, 0, 0));
			}

			memset(ScriptPath, '\0', SCRIPT_PATH_LENGTH);
			strcpy_s(ScriptPath, path);
		}

		void Destroy(lua_State *L)
		{
			ZoneScopedC(RandomUniqueColor());

			if (LuaRef != LUA_NOREF && !Game::IsQuitting)
			{
				// Remove the reference to the behaviour table
				luaL_unref(m_refState, LUA_REGISTRYINDEX, LuaRef);
				LuaRef = LUA_NOREF;
			}
		}

		void LuaPush(lua_State *L) const
		{
			ZoneScopedC(RandomUniqueColor());
			// Retrieve the behaviour table to the top of the stack
			lua_rawgeti(L, LUA_REGISTRYINDEX, LuaRef);
		}

		// Check if name exists in the unowned methods, return false if it does
		bool IsUnownedMethod(const std::string &name)
		{
			for (const auto &method : m_unownedMethods)
			{
				if (method == name)
					return true;
			}

			return false;
		}
		void AddUnownedMethod(const std::string &name)
		{
			m_unownedMethods.push_back(name);
		}

		void RenderUI()
		{
			if (ImGui::TreeNode("Behaviour"))
			{
				if (ImGui::Button("Select File"))
				{
					std::string fileName, filePath;
					if (Windows::OpenFileCatalog(fileName, filePath))
					{
						size_t lastindex = fileName.find_last_of("."); 
						fileName = fileName.substr(0, lastindex); 
						std::string localPath = "Behaviours/" + fileName;
						Destroy(m_refState);
						Initialize(localPath.c_str());
					}
				}

				ImGui::InputText("Path", ScriptPath, SCRIPT_PATH_LENGTH);


				ImGui::SeparatorText("Lua Gui");
				// Run OnGUI if it exists
				do
				{
					const std::string name("OnGUI");

					if (IsUnownedMethod(name))
						break;

					// Retrieve the behaviour table to the top of the stack
					lua_rawgeti(m_refState, LUA_REGISTRYINDEX, LuaRef);

					// Retrieve the requested method from the table
					lua_getfield(m_refState, -1, name.c_str());

					// Check if the method exists before calling it
					if (lua_isnil(m_refState, -1))
					{
						lua_pop(m_refState, 1); // Pop nil
						AddUnownedMethod(name);
					}
					else
					{
						// Push the table as the first argument to the method
						lua_pushvalue(m_refState, -2);

						// Call the method, pops the method and its arguments from the stack
						LuaChkL(m_refState, lua_pcall(m_refState, 1, 0, 0));
					}

					// Pop the behaviour table from the stack
					lua_pop(m_refState, 1);

				} while (false);
				ImGui::Separator();

				ImGui::TreePop();
			}
		}

	private:
		lua_State *m_refState = nullptr;
		int m_entity;

		std::vector<std::string> m_unownedMethods;
	};

	struct Transform
	{
		float Position[2];
		float Rotation;
		float Scale[2];

		void LuaPush(lua_State *L) const
		{
			ZoneScopedC(RandomUniqueColor());
			// Create the main transform table
			lua_createtable(L, 0, 3);  // Create a new table for Transform

			lua_createtable(L, 0, 2);  // Create a new table for Position
			lua_pushnumber(L, Position[0]);
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, Position[1]);
			lua_setfield(L, -2, "y");
			lua_setfield(L, -2, "position");  // Add Position to main table

			// Create and push the Rotation
			lua_pushnumber(L, Rotation);
			lua_setfield(L, -2, "rotation");  // Add Rotation to main table

			// Create and push the Scale subtable
			lua_createtable(L, 0, 2);  // Create a new table for Scale
			lua_pushnumber(L, Scale[0]);
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, Scale[1]);
			lua_setfield(L, -2, "y");
			lua_setfield(L, -2, "scale");  // Add Scale to main table
			
			// The main transform table is now at the top of the stack
		}
		void LuaPull(lua_State* L, int index)
		{
			ZoneScopedC(RandomUniqueColor());
			// Make sure the index is absolute
			if (index < 0)
			{
				index = lua_gettop(L) + index + 1;
			}

			// Get Position subtable
			lua_getfield(L, index, "position");
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "x");
				Position[0] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, -1, "y");
				Position[1] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);
			}
			lua_pop(L, 1); // Pop Position table

			// Get Rotation
			lua_getfield(L, index, "rotation");
			Rotation = (float)lua_tonumber(L, -1);
			lua_pop(L, 1); // Pop Rotation table

			// Get Scale subtable
			lua_getfield(L, index, "scale");
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "x");
				Scale[0] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, -1, "y");
				Scale[1] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);
			}
			lua_pop(L, 1); // Pop Scale table
		}

		void RenderUI()
		{
			if (ImGui::TreeNode("Transform"))
			{
				ImGui::DragFloat2("Position", Position, 0.1f, -1000, 1000);
				ImGui::DragFloat2("Scale", Scale, 0.1f, -1000, 1000);
				ImGui::DragFloat("Rotation", &Rotation, 0.1f, -1000, 1000);

				ImGui::TreePop();
			}
		}
	};

	struct Collider
	{
		b2BodyId bodyId = b2_nullBodyId;
		bool createBody = false;
		bool debug = false;
		int luaRef;
		static constexpr int MAX_TAG_LENGTH = 32;
		char tag[MAX_TAG_LENGTH];
		float offset[2] { 0 };
		float extents[2] { 1, 1 };

		Collider(): createBody(true) {}

		void Destroy(lua_State* L)
		{
			b2DestroyBody(bodyId);
			bodyId = b2_nullBodyId;
			luaL_unref(L, LUA_REGISTRYINDEX, luaRef);
		}

		void LuaPush(lua_State* L) const
		{
			ZoneScopedC(RandomUniqueColor());

			lua_createtable(L, 0, 4);

			lua_pushstring(L, tag);
			lua_setfield(L, -2, "tag");

			lua_pushboolean(L, debug);
			lua_setfield(L, -2, "debug");

			lua_createtable(L, 0, 2);
			lua_pushnumber(L, offset[0]);
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, offset[1]);
			lua_setfield(L, -2, "y");
			lua_setfield(L, -2, "offset");

			lua_createtable(L, 0, 2);
			lua_pushnumber(L, extents[0]);
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, extents[1]);
			lua_setfield(L, -2, "y");
			lua_setfield(L, -2, "extents");	

            lua_rawgeti(L, LUA_REGISTRYINDEX, luaRef);
			lua_setfield(L, -2, "callback");
		}
		void LuaPull(lua_State *L, int index)
		{
			ZoneScopedC(RandomUniqueColor());
			// Make sure the index is absolute (in case it's negative)
			if (index < 0)
			{
				index = lua_gettop(L) + index + 1;
			}

			if (!lua_istable(L, index)) {
				luaL_error(L, "Expected a table for Collider");
				return;
			}

			createBody = true;

			lua_getfield(L, index, "tag");
			if (lua_isstring(L, -1)) 
			{
				const char* tempTag = lua_tostring(L, -1);
				memset(tag, '\0', MAX_TAG_LENGTH);
				strncpy_s(tag, tempTag, MAX_TAG_LENGTH - 1);
			}
			lua_pop(L, 1); // Remove the spriteName value from stack

			lua_getfield(L, index, "debug");
			if (lua_isboolean(L, -1)) 
				debug = lua_toboolean(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "offset");
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "x");
				offset[0] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, -1, "y");
				offset[1] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);
			}

			lua_getfield(L, index, "extents");
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "x");
				float temp = (float)lua_tonumber(L, -1);
				if (temp != extents[0])
				{
					extents[0] = temp;
					createBody = true;
				}
				lua_pop(L, 1);

				lua_getfield(L, -1, "y");
				temp = (float)lua_tonumber(L, -1);
				if (temp != extents[1])
				{
					extents[1] = temp;
					createBody = true;
				}
				lua_pop(L, 1);
			}

			lua_getfield(L, index, "callback");
			if (lua_isfunction(L, -1))
				luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
		}
	
		void RenderUI()
		{
			if (ImGui::TreeNode("Collider"))
			{
				// TODO: This is'nt working
				ImGui::DragFloat2("Offset", offset, 0.1f, -1000, 1000);
				createBody |= ImGui::DragFloat2("Extents", extents, 0.1f, -1000, 1000);

				ImGui::Checkbox("Debug", &debug);

				ImGui::TreePop();
			}
		}
	};

	struct Sprite
	{
		static const int SPRITE_NAME_LENGTH = 64;
		char SpriteName[SPRITE_NAME_LENGTH];
		float Color[4];
		int Priority;

		Sprite() : Priority(0)
		{
			memset(Color, 1.0f, sizeof(float) * 4);

			memset(SpriteName, '\0', SPRITE_NAME_LENGTH);
		}
		Sprite(const char *name, const float color[4], const int priority) : Priority(priority)
		{
			memcpy(Color, color, sizeof(float) * 4);

			memset(SpriteName, '\0', SPRITE_NAME_LENGTH);
			strcpy_s(SpriteName, name);
		}

		// Comparison function for sorting draw order by priority
		inline static bool Compare(const Sprite &left, const Sprite &right) 
		{
			return left.Priority < right.Priority;
		}

		void LuaPush(lua_State* L) const
		{
			ZoneScopedC(RandomUniqueColor());
			// Create the main sprite table
			lua_createtable(L, 0, 3);

			// Add spriteName to the sprite table
			lua_pushstring(L, SpriteName);
			lua_setfield(L, -2, "spriteName");
			
			// Create a new table for color
			lua_createtable(L, 0, 4);  
			lua_pushnumber(L, Color[0]);
			lua_setfield(L, -2, "r");
			lua_pushnumber(L, Color[1]);
			lua_setfield(L, -2, "g");
			lua_pushnumber(L, Color[2]);
			lua_setfield(L, -2, "b");
			lua_pushnumber(L, Color[3]);
			lua_setfield(L, -2, "a");
			lua_setfield(L, -2, "color");  // Add color to the sprite table

			// Add priority to the sprite table
			lua_pushnumber(L, Priority);
			lua_setfield(L, -2, "priority");
		}
		void LuaPull(lua_State* L, int index)
		{
			ZoneScopedC(RandomUniqueColor());
			// Make sure the index is absolute (in case it's negative)
			if (index < 0) {
				index = lua_gettop(L) + index + 1;
			}

			// Verify that the value at the given index is a table
			if (!lua_istable(L, index)) {
				luaL_error(L, "Expected a table for Sprite");
				return;
			}

			// Get spriteName field
			lua_getfield(L, index, "spriteName");
			if (lua_isstring(L, -1)) {
				const char* name = lua_tostring(L, -1);
				memset(SpriteName, '\0', SPRITE_NAME_LENGTH);
				strncpy_s(SpriteName, name, SPRITE_NAME_LENGTH - 1);
			}
			lua_pop(L, 1); // Remove the spriteName value from stack

			// Get color table
			lua_getfield(L, index, "color");
			if (lua_istable(L, -1)) {
				// Get r component
				lua_getfield(L, -1, "r");
				if (lua_isnumber(L, -1)) {
					Color[0] = (float)lua_tonumber(L, -1);
				}
				lua_pop(L, 1);

				// Get g component
				lua_getfield(L, -1, "g");
				if (lua_isnumber(L, -1)) {
					Color[1] = (float)lua_tonumber(L, -1);
				}
				lua_pop(L, 1);

				// Get b component
				lua_getfield(L, -1, "b");
				if (lua_isnumber(L, -1)) {
					Color[2] = (float)lua_tonumber(L, -1);
				}
				lua_pop(L, 1);

				// Get a component
				lua_getfield(L, -1, "a");
				if (lua_isnumber(L, -1)) {
					Color[3] = (float)lua_tonumber(L, -1);
				}
				lua_pop(L, 1);
			}
			lua_pop(L, 1); // Remove the color table from stack

			// Get priority field
			lua_getfield(L, index, "priority");
			if (lua_isnumber(L, -1)) {
				Priority = (int)lua_tonumber(L, -1);
			}
			lua_pop(L, 1); // Remove the priority value from stack
		}

		void RenderUI()
		{
			if (ImGui::TreeNode("Sprite"))
			{
				ImGui::InputInt("Priority", &Priority);

				static int selected = -1;
				std::vector<std::string> items = ResourceManager::Instance().GetTextureNames();
				items.push_back("None");
				std::string title = "Select Texture";

				if (selected != -1)
					title = items[selected];

				if (ImGui::BeginCombo("Texture", title.c_str()))
				{
					for (int n = 0; n < items.size(); n++)
					{
						std::string current = items[n];
						bool isSelected = n == selected;
						if (ImGui::Selectable(current.c_str(), isSelected))
						{
							if (current != "None")
							{
								strcpy_s(SpriteName, current.c_str());
								selected = n;
							}
							else
							{
								memset(SpriteName, '\0', SPRITE_NAME_LENGTH);
								selected = -1;
							}
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				ImGui::ColorPicker4("Color", Color);

				ImGui::TreePop();
			}
		}
	};
	
	struct Health
	{
		float Current;
		float Max;

		void LuaPush(lua_State* L) const
		{
			ZoneScopedC(RandomUniqueColor());
			// Create the main health table
			lua_createtable(L, 0, 2);

			// Add Current and Max to the health table
			lua_pushnumber(L, Current);
			lua_setfield(L, -1, "current");

			lua_pushnumber(L, Max);
			lua_setfield(L, -2, "max");
		}
		void LuaPull(lua_State* L, int index)
		{
			ZoneScopedC(RandomUniqueColor());
			// Make sure the index is absolute (in case it's negative)
			if (index < 0)
			{
				index = lua_gettop(L) + index + 1;
			}

			// Verify that the value at the given index is a table
			if (!lua_istable(L, index))
			{
				luaL_error(L, "Expected a table for Health");
				return;
			}

			// Get Current field
			lua_getfield(L, index, "current");
			if (lua_isnumber(L, -1))
			{
				Current = (float)lua_tonumber(L, -1);
			}
			lua_pop(L, 1); // Remove the current value from stack

			// Get Max field
			lua_getfield(L, index, "max");
			if (lua_isnumber(L, -1))
			{
				Max = (float)lua_tonumber(L, -1);
			}
			lua_pop(L, 1); // Remove the max value from stack
		}
	};

	struct CameraData
	{
		int Size[2];
		float Zoom;

		void LuaPush(lua_State* L) const
		{
			ZoneScopedC(RandomUniqueColor());
			// Create the main CameraData table
			lua_createtable(L, 0, 2);

			lua_createtable(L, 0, 2);  // Create a new table for Size
			lua_pushnumber(L, Size[0]);
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, Size[1]);
			lua_setfield(L, -2, "y");
			lua_setfield(L, -2, "size");  // Add Size to main table

			lua_pushnumber(L, Zoom);
			lua_setfield(L, -2, "zoom");
		}
		void LuaPull(lua_State* L, int index)
		{
			ZoneScopedC(RandomUniqueColor());
			// Make sure the index is absolute
			if (index < 0)
			{
				index = lua_gettop(L) + index + 1;
			}

			// Get Size subtable
			lua_getfield(L, index, "size");
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "x");
				Size[0] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, -1, "y");
				Size[1] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);
			}
			lua_pop(L, 1); // Pop Size table

			// Get Zoom field
			lua_getfield(L, index, "zoom");
			if (lua_isnumber(L, -1))
			{
				Zoom = (float)lua_tonumber(L, -1);
			}
			lua_pop(L, 1); // Remove the zoom value from stack
		}
	};

	struct Remove {
		//bool _ : 1; // Place holder
		int _; // Place holder
	};
}
