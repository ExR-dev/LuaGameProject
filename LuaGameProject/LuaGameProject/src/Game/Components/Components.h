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
			lua_pushboolean(L, IsActive);
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
			if (lua_istable(L, index))
			{
				lua_getfield(L, index, "isActive");
				if (lua_isboolean(L, -1))
					IsActive = lua_toboolean(L, -1);
				lua_pop(L, 1);
			}
		}

		void RenderUI()
		{
			if (ImGui::TreeNode("Active"))
			{
				ImGui::Checkbox("IsActive", &IsActive);

				ImGui::Separator();
				ImGui::TreePop();
			}
		}
	};

	struct Behaviour
	{
	public:
		static constexpr int SCRIPT_PATH_LENGTH = 78;
		char ScriptPath[SCRIPT_PATH_LENGTH];
		int LuaRef;

		// Create a constructor in order to initialize the char array
		Behaviour(const char *path, int entity, lua_State *L) : m_refState(L), m_entity(entity)
		{
			Initialize(path);
		}

		void Initialize(const char *path)
		{
			ZoneScopedC(RandomUniqueColor());

			lua_State *L = m_refState;
			m_unownedMethods.clear();

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
				LuaChk(L, lua_pcall(L, 1, 0, 0));
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
					if (Windows::OpenFileCatalog(fileName, filePath, FILE_PATH))
					{
						size_t lastindex = fileName.find_last_of("."); 
						fileName = fileName.substr(0, lastindex); 
						std::string localPath = "Behaviours/" + fileName;
						Destroy(m_refState);
						Initialize(localPath.c_str());
					}
				}

				ImGui::InputText("Path", ScriptPath, SCRIPT_PATH_LENGTH);

				// Run OnGUI if it exists
				do
				{
					const std::string name("OnGUI");

					if (IsUnownedMethod(name))
						break;

					ImGui::SeparatorText("Lua Gui");

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
						LuaChk(m_refState, lua_pcall(m_refState, 1, 0, 0));
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
		float Position[2] = { 0.0f, 0.0f };
		float Rotation = 0.0f;
		float Scale[2] = { 1.0f, 1.0f };

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
				if (lua_isnumber(L, -1))
					Position[0] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, -1, "y");
				if (lua_isnumber(L, -1))
					Position[1] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);
			}
			lua_pop(L, 1); // Pop Position table

			// Get Rotation
			lua_getfield(L, index, "rotation");
			if (lua_isnumber(L, -1))
				Rotation = (float)lua_tonumber(L, -1);
			lua_pop(L, 1); // Pop Rotation number

			// Get Scale subtable
			lua_getfield(L, index, "scale");
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "x");
				if (lua_isnumber(L, -1))
					Scale[0] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, -1, "y");
				if (lua_isnumber(L, -1))
					Scale[1] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);
			}
			lua_pop(L, 1); // Pop Scale table
		}

		void RenderUI()
		{
			if (ImGui::TreeNode("Transform"))
			{
				ImGui::DragFloat2("Position", Position, 0.05f);
				ImGui::DragFloat2("Scale", Scale, 0.025f);

				float rotRad = Rotation * DEG2RAD;
				if (ImGui::SliderAngle("Rotation", &rotRad, 0.0f, 360.0f))
					Rotation = rotRad * RAD2DEG;

				ImGui::Separator();
				ImGui::TreePop();
			}
		}
	};

	struct Collider
	{
		b2BodyId bodyId = b2_nullBodyId;
		bool createBody = false;
		bool debug = false;
		bool rotateWithTransform = false;
		int onEnterRef, onExitRef;
		static constexpr int MAX_TAG_LENGTH = 32;
		char tag[MAX_TAG_LENGTH];
		float offset[2] { 0 };
		float extents[2] { 1, 1 };
		float rotation = 0;

		Collider(bool recreating = false)
		{
			createBody = !recreating;
		}

		void Destroy(lua_State* L)
		{
			if (!b2Body_IsValid(bodyId))
				return;
			b2DestroyBody(bodyId);
			bodyId = b2_nullBodyId;
			luaL_unref(L, LUA_REGISTRYINDEX, onEnterRef);
			luaL_unref(L, LUA_REGISTRYINDEX, onExitRef);
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

			lua_pushnumber(L, rotation);
			lua_setfield(L, -2, "rotation");

            lua_rawgeti(L, LUA_REGISTRYINDEX, onEnterRef);
			lua_setfield(L, -2, "onEnterCallback");

			lua_rawgeti(L, LUA_REGISTRYINDEX, onExitRef);
			lua_setfield(L, -2, "onExitCallback");
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

			lua_getfield(L, index, "rotateWithTransform");
			if (lua_isboolean(L, -1))
				rotateWithTransform = lua_toboolean(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "offset");
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "x");
				if (lua_isnumber(L, -1))
					offset[0] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, -1, "y");
				if (lua_isnumber(L, -1))
					offset[1] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);
			}

			lua_getfield(L, index, "extents");
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "x");
				if (lua_isnumber(L, -1))
				{
					float temp = (float)lua_tonumber(L, -1);
					if (temp != extents[0])
					{
						extents[0] = temp;
						createBody = true;
					}
				}
				lua_pop(L, 1);

				lua_getfield(L, -1, "y");
				if (lua_isnumber(L, -1))
				{
					float temp = (float)lua_tonumber(L, -1);
					if (temp != extents[1])
					{
						extents[1] = temp;
						createBody = true;
					}
				}
				lua_pop(L, 1);
			}

			lua_getfield(L, index, "rotation");
			if (lua_isnumber(L, -1))
			{
				rotation = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);
			}


			lua_getfield(L, index, "onEnterCallback");
			if (lua_isfunction(L, -1))
				onEnterRef = luaL_ref(L, LUA_REGISTRYINDEX);

			lua_getfield(L, index, "onExitCallback");
			if (lua_isfunction(L, -1))
				onExitRef = luaL_ref(L, LUA_REGISTRYINDEX);
		}
	
		void RenderUI()
		{
			if (ImGui::TreeNode("Collider"))
			{
				ImGui::InputText("Tag", tag, MAX_TAG_LENGTH);

				ImGui::DragFloat2("Offset", offset, 0.1f, -1000, 1000);
				createBody |= ImGui::DragFloat2("Extents", extents, 0.001f, 0.001f, 1000);
				if (ImGui::DragFloat("Rotation", &rotation, 0.1f, -1.0f, 361.0f))
				{
					if (rotation >= 0)
						rotation = std::fmodf(rotation, 360.0f);
					else
						rotation = 360 - std::fmodf(-1*rotation, 360.0f);
				}

				ImGui::Checkbox("Debug", &debug);

				ImGui::Separator();
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
			memset(Color, (int)1.0f, sizeof(float) * 4);

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
			lua_pushinteger(L, Priority);
			lua_setfield(L, -2, "priority");
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
			if (lua_istable(L, index)) 
			{
				// Get spriteName field
				lua_getfield(L, index, "spriteName");
				if (lua_isstring(L, -1)) 
				{
					const char* name = lua_tostring(L, -1);
					memset(SpriteName, '\0', SPRITE_NAME_LENGTH);
					strncpy_s(SpriteName, name, SPRITE_NAME_LENGTH - 1);
				}
				lua_pop(L, 1); // Remove the spriteName value from stack

				// Get color table
				lua_getfield(L, index, "color");
				if (lua_istable(L, -1)) 
				{
					// Get r component
					lua_getfield(L, -1, "r");
					if (lua_isnumber(L, -1)) 
						Color[0] = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);

					// Get g component
					lua_getfield(L, -1, "g");
					if (lua_isnumber(L, -1)) 
						Color[1] = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);

					// Get b component
					lua_getfield(L, -1, "b");
					if (lua_isnumber(L, -1)) 
						Color[2] = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);

					// Get a component
					lua_getfield(L, -1, "a");
					if (lua_isnumber(L, -1))
						Color[3] = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
				}
				lua_pop(L, 1); // Remove the color table from stack

				// Get priority field
				lua_getfield(L, index, "priority");
				if (lua_isnumber(L, -1))
					Priority = (int)lua_tonumber(L, -1);
				lua_pop(L, 1); // Remove the priority value from stack
			}
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

				ImGui::Separator();
				ImGui::TreePop();
			}
		}
	};

	struct TextRender
	{
		static const int TEXT_LENGTH = 256;
		char Text[TEXT_LENGTH];
		static const int FONTNAME_LENGTH = 32;
		char Font[FONTNAME_LENGTH];
		float FontSize;
		float Spacing;
		float TextColor[4];
		float Offset[2];
		float Rotation;
		float BgThickness;
		float BgColor[4];

		TextRender() : FontSize(12.0f), Spacing(1.0f), Rotation(0.0f), BgThickness(0.0f)
		{
			memset(Text, '\0', TEXT_LENGTH);
			memset(Font, '\0', FONTNAME_LENGTH);

			memset(Offset, 0.0f, sizeof(float) * 2);

			memset(TextColor, (int)0.0f, sizeof(float) * 3);
			memset(BgColor, (int)0.0f, sizeof(float) * 4);
			TextColor[3] = 1.0f; // Alpha
		}

		void LuaPush(lua_State* L) const
		{
			ZoneScopedC(RandomUniqueColor());
			// Create the main textRender table
			lua_createtable(L, 0, 9);

			lua_pushstring(L, Text);
			lua_setfield(L, -2, "text");

			lua_pushstring(L, Font);
			lua_setfield(L, -2, "font");

			lua_pushnumber(L, FontSize);
			lua_setfield(L, -2, "fontSize");

			lua_pushnumber(L, Spacing);
			lua_setfield(L, -2, "spacing");

			lua_createtable(L, 0, 4);
			lua_pushnumber(L, TextColor[0]);
			lua_setfield(L, -2, "r");
			lua_pushnumber(L, TextColor[1]);
			lua_setfield(L, -2, "g");
			lua_pushnumber(L, TextColor[2]);
			lua_setfield(L, -2, "b");
			lua_pushnumber(L, TextColor[3]);
			lua_setfield(L, -2, "a");
			lua_setfield(L, -2, "textColor");

			lua_createtable(L, 0, 2);
			lua_pushnumber(L, Offset[0]);
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, Offset[1]);
			lua_setfield(L, -2, "y");
			lua_setfield(L, -2, "offset");

			lua_pushnumber(L, Rotation);
			lua_setfield(L, -2, "rotation");

			lua_pushnumber(L, BgThickness);
			lua_setfield(L, -2, "bgThickness");
			
			lua_createtable(L, 0, 4);  
			lua_pushnumber(L, BgColor[0]);
			lua_setfield(L, -2, "r");
			lua_pushnumber(L, BgColor[1]);
			lua_setfield(L, -2, "g");
			lua_pushnumber(L, BgColor[2]);
			lua_setfield(L, -2, "b");
			lua_pushnumber(L, BgColor[3]);
			lua_setfield(L, -2, "a");
			lua_setfield(L, -2, "bgColor");
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
			if (lua_istable(L, index)) 
			{
				lua_getfield(L, index, "text");
				if (lua_isstring(L, -1)) 
				{
					const char* name = lua_tostring(L, -1);
					memset(Text, '\0', TEXT_LENGTH);
					strncpy_s(Text, name, TEXT_LENGTH - 1);
				}
				lua_pop(L, 1);

				lua_getfield(L, index, "font");
				if (lua_isstring(L, -1)) 
				{
					const char* font = lua_tostring(L, -1);
					memset(Font, '\0', FONTNAME_LENGTH);
					strncpy_s(Font, font, FONTNAME_LENGTH - 1);
				}
				lua_pop(L, 1);

				lua_getfield(L, index, "fontSize");
				if (lua_isnumber(L, -1))
					FontSize = (int)lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, index, "spacing");
				if (lua_isnumber(L, -1))
					Spacing = (int)lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, index, "textColor");
				if (lua_istable(L, -1)) 
				{
					lua_getfield(L, -1, "r");
					if (lua_isnumber(L, -1)) 
						TextColor[0] = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);

					lua_getfield(L, -1, "g");
					if (lua_isnumber(L, -1)) 
						TextColor[1] = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);

					lua_getfield(L, -1, "b");
					if (lua_isnumber(L, -1)) 
						TextColor[2] = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);

					lua_getfield(L, -1, "a");
					if (lua_isnumber(L, -1))
						TextColor[3] = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
				}
				lua_pop(L, 1);

				lua_getfield(L, index, "offset");
				if (lua_istable(L, -1)) 
				{
					lua_getfield(L, -1, "x");
					if (lua_isnumber(L, -1)) 
						Offset[0] = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);

					lua_getfield(L, -1, "y");
					if (lua_isnumber(L, -1)) 
						Offset[1] = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
				}
				lua_pop(L, 1);

				lua_getfield(L, index, "rotation");
				if (lua_isnumber(L, -1))
					Rotation = (int)lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, index, "bgThickness");
				if (lua_isnumber(L, -1))
					BgThickness = (int)lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, index, "bgColor");
				if (lua_istable(L, -1)) 
				{
					lua_getfield(L, -1, "r");
					if (lua_isnumber(L, -1)) 
						BgColor[0] = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);

					lua_getfield(L, -1, "g");
					if (lua_isnumber(L, -1)) 
						BgColor[1] = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);

					lua_getfield(L, -1, "b");
					if (lua_isnumber(L, -1)) 
						BgColor[2] = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);

					lua_getfield(L, -1, "a");
					if (lua_isnumber(L, -1))
						BgColor[3] = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
				}
				lua_pop(L, 1);
			}
		}

		void RenderUI()
		{
			if (ImGui::TreeNode("TextRender"))
			{
				ImGui::InputTextMultiline("Text", Text, TEXT_LENGTH);

				static int selected = -1;
				std::vector<std::string> items = ResourceManager::Instance().GetFontNames();
				items.push_back("None");
				std::string title = "Select Font";

				if (selected != -1)
					title = items[selected];

				if (ImGui::BeginCombo("Font", title.c_str()))
				{
					for (int n = 0; n < items.size(); n++)
					{
						std::string current = items[n];
						bool isSelected = n == selected;
						if (ImGui::Selectable(current.c_str(), isSelected))
						{
							if (current != "None")
							{
								strcpy_s(Font, current.c_str());
								selected = n;
							}
							else
							{
								memset(Font, '\0', FONTNAME_LENGTH);
								selected = -1;
							}
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				if (ImGui::DragFloat("Font Size", &FontSize, 0.2f))
					FontSize = std::fmaxf(0.0f, FontSize);

				if (ImGui::DragFloat("Spacing", &Spacing, 0.02f))
					Spacing = std::fmaxf(0.0f, Spacing);

				ImGui::DragFloat("Background Thickness", &BgThickness, 0.01f);

				ImGui::DragFloat2("Offset", Offset, 0.1f);

				float rotRad = Rotation * DEG2RAD;
				if (ImGui::SliderAngle("Rotation", &rotRad, 0.0f, 360.0f))
					Rotation = rotRad * RAD2DEG;

				ImGui::ColorEdit4("Text Color", &TextColor[0]);
				ImGui::ColorEdit4("Background Color", &BgColor[0]);

				ImGui::Separator();
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
			lua_setfield(L, -2, "current");

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
			if (lua_istable(L, index))
			{
				// Get Current field
				lua_getfield(L, index, "current");
				if (lua_isnumber(L, -1))
					Current = (float)lua_tonumber(L, -1);
				lua_pop(L, 1); // Remove the current value from stack

				// Get Max field
				lua_getfield(L, index, "max");
				if (lua_isnumber(L, -1))
					Max = (float)lua_tonumber(L, -1);
				lua_pop(L, 1); // Remove the max value from stack
			}
		}

		void RenderUI()
		{
			if (ImGui::TreeNode("Health"))
			{
				if (ImGui::DragFloat("Max", &Max, 0.1f))
					Max = std::fmaxf(1.0f, Max);

				if (ImGui::SliderFloat("Current", &Current, 0.0f, Max))
					Current = std::clamp(Current, 0.0f, Max);

				ImGui::Separator();
				ImGui::TreePop();
			}
		}
	};

	struct Hardness
	{
		float hardness;

		Hardness(float h = 0) : hardness(h) {}

		void LuaPush(lua_State *L) const
		{
			ZoneScopedC(RandomUniqueColor());

			lua_createtable(L, 0, 1);
			lua_pushnumber(L, hardness);
			lua_setfield(L, -2, "hardness");
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
			if (lua_istable(L, index))
			{
				lua_getfield(L, index, "hardness");
				if (lua_isnumber(L, -1))
					hardness = lua_tonumber(L, -1);
				lua_pop(L, 1);
			}
		}

		void RenderUI()
		{
			if (ImGui::TreeNode("Hardness"))
			{
				if (ImGui::DragFloat("Hardness", &hardness, 0.05f))
					hardness = std::fmaxf(0.0f, hardness);

				ImGui::Separator();
				ImGui::TreePop();
			}
		}
	};

	struct Room
	{
		static const int ROOM_NAME_LENGTH = 32;
		char RoomName[ROOM_NAME_LENGTH];

		Room()
		{
			memset(RoomName, '\0', ROOM_NAME_LENGTH);
		}

		Room(const char *name)
		{
			memset(RoomName, '\0', ROOM_NAME_LENGTH);
			strcpy_s(RoomName, name);
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
				if (lua_isnumber(L, -1))
					Size[0] = (int)lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, -1, "y");
				if (lua_isnumber(L, -1))
					Size[1] = (int)lua_tonumber(L, -1);
				lua_pop(L, 1);
			}
			lua_pop(L, 1); // Pop Size table

			// Get Zoom field
			lua_getfield(L, index, "zoom");
			if (lua_isnumber(L, -1))
				Zoom = (float)lua_tonumber(L, -1);
			lua_pop(L, 1); // Remove the zoom value from stack
		}

		void RenderUI()
		{
			if (ImGui::TreeNode("CameraData"))
			{
				// TODO

				ImGui::Separator();
				ImGui::TreePop();
			}
		}
	};

	struct UIElement
	{
		// TODO: Add flag for if to use UV or pixel coordinates
		char _;

		void LuaPush(lua_State *L) const
		{
			ZoneScopedC(RandomUniqueColor());
			lua_createtable(L, 0, 1);

			lua_pushnumber(L, _);
			lua_setfield(L, -2, "_");
		}
		void LuaPull(lua_State *L, int index)
		{
			ZoneScopedC(RandomUniqueColor());
			if (index < 0)
			{
				index = lua_gettop(L) + index + 1;
			}

			// Verify that the value at the given index is a table
			if (lua_istable(L, index))
			{
				lua_getfield(L, index, "_");
				if (lua_isnumber(L, -1))
					_ = lua_tonumber(L, -1);
				lua_pop(L, 1);
			}
		}

		void RenderUI()
		{
			if (ImGui::TreeNode("UIElement"))
			{
				ImGui::TextWrapped("Entities with this component are transformed to view-space before being rendered.");

				ImGui::Separator();
				ImGui::TreePop();
			}
		}
	};

	struct Remove 
	{
		char _; // Place holder

		void RenderUI()
		{
			if (ImGui::TreeNode("Remove"))
			{
				// TODO

				ImGui::Separator();
				ImGui::TreePop();
			}
		}
	};

	struct Debug
	{
		char _; // Place holder
	};
}
