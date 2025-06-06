#include "stdafx.h"
#include "ImLua.h"


namespace ImLua
{

	void ImLua::lua_openimgui(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		lua_newtable(L);

			// Not all are implemented yet, check function bodies
		luaL_Reg methods[] = {
			{ "Begin",						lua_Begin					},
			{ "End",						lua_End						},
			{ "BeginChild",					lua_BeginChild				},
			{ "EndChild",					lua_EndChild				},
			{ "OpenPopup",					lua_OpenPopup				},
			{ "IsPopupOpen",				lua_IsPopupOpen				},
			{ "CloseCurrentPopup",			lua_CloseCurrentPopup		},
			{ "BeginPopup",					lua_BeginPopup				},
			{ "BeginPopupModal",			lua_BeginPopupModal			},
			{ "BeginPopupContextItem",		lua_BeginPopupContextItem	},
			{ "EndPopup",					lua_EndPopup				},
			{ "Separator",					lua_Separator				},
			{ "Text",						lua_Text					},
			{ "TextWrapped",				lua_TextWrapped				},
			{ "InputText",					lua_InputText				},
			{ "DragFloat",					lua_DragFloat				},
			{ "DragFloat2",					lua_DragFloat2				},
			{ "DragInt",					lua_DragInt					},
			{ "Button",						lua_Button					},
			{ "Checkbox",					lua_Checkbox				},
			{ "RadioButton",				lua_RadioButton				},
			{ "RadioButtonInt",				lua_RadioButtonInt			},
			{ "InputFloat",					lua_InputFloat				},
			{ "SliderFloat",				lua_SliderFloat				},
			{ "InputFloat2",				lua_InputFloat2				},
			{ "SliderInt",					lua_SliderInt				},
			{ "SliderAngle",				lua_SliderAngle				},
			{ "InputInt",					lua_InputInt				},
			{ "ColorEdit4",					lua_ColorEdit4				},
			{ "ColorPicker4",				lua_ColorPicker4			},
			{ "ColorButton",				lua_ColorButton				},
			{ "ProgressBar",				lua_ProgressBar				},
			{ "Image",						lua_Image					},
			{ "TreeNode",					lua_TreeNode				},
			{ "TreePop",					lua_TreePop					},
			{ "CollapsingHeader",			lua_CollapsingHeader		},
			{ "BeginCombo",					lua_BeginCombo				},
			{ "Combo",						lua_Combo					},
			{ "EndCombo",					lua_EndCombo				},
			{ "BeginGroup",					lua_BeginGroup				},
			{ "EndGroup",					lua_EndGroup				},
			{ "PushID",						lua_PushID					},
			{ "PopID",						lua_PopID					},
			{ "BeginTooltip",				lua_BeginTooltip			},
			{ "EndTooltip",					lua_EndTooltip				},
			{ "BeginTable",					lua_BeginTable				},
			{ "EndTable",					lua_EndTable				},
			{ "TableNextRow",				lua_TableNextRow			},
			{ "TableNextColumn",			lua_TableNextColumn			},
			{ "TableSetColumnIndex",		lua_TableSetColumnIndex		},
			{ "SameLine",					lua_SameLine				},
			{ "Selectable",					lua_Selectable				},
			{ "NewLine",					lua_NewLine					},
			{ "Spacing",					lua_Spacing					},
			{ "Dummy",						lua_Dummy					},
			{ "Indent",						lua_Indent					},
			{ "Unindent",					lua_Unindent				},
			{ "SetNextItemWidth",			lua_SetNextItemWidth		},
			{ "CalcItemWidth",				lua_CalcItemWidth			},
			{ "GetCursorScreenPos",			lua_GetCursorScreenPos		},
			{ "SetCursorScreenPos",			lua_SetCursorScreenPos		},
			{ "GetCursorPos",				lua_GetCursorPos			},
			{ "SetCursorPos",				lua_SetCursorPos			},
			{ "GetWindowPos",				lua_GetWindowPos			},
			{ "GetWindowSize",				lua_GetWindowSize			},
			{ "SetWindowPos",				lua_SetWindowPos			},
			{ "SetWindowSize",				lua_SetWindowSize			},
			{ "IsItemHovered",				lua_IsItemHovered			},
			{ "IsItemActive",				lua_IsItemActive			},
			{ "IsAnyItemActive",			lua_IsAnyItemActive			},
			{ "IsItemFocused",				lua_IsItemFocused			},
			{ "IsItemClicked",				lua_IsItemClicked			},
			{ "IsItemVisible",				lua_IsItemVisible			},
			{ "IsItemEdited",				lua_IsItemEdited			},
			{ "IsItemActivated",			lua_IsItemActivated			},
			{ "IsItemToggledOpen",			lua_IsItemToggledOpen		},
			{ "GetItemID",					lua_GetItemID				},
			{ "GetItemRectMin",				lua_GetItemRectMin			},
			{ "GetItemRectMax",				lua_GetItemRectMax			},
			{ "GetItemRectSize",			lua_GetItemRectSize			},
			{ "CalcTextSize",				lua_CalcTextSize			},
			{ "GetClipboardText",			lua_GetClipboardText		},
			{ "SetClipboardText",			lua_SetClipboardText		},
			{ "IsMouseClicked",				lua_IsMouseClicked			},
			{ "IsWindowFocused",			lua_IsWindowFocused			},
			{ "SetKeyboardFocusHere",		lua_SetKeyboardFocusHere	},
			{ "SetItemDefaultFocus",		lua_SetItemDefaultFocus		},

			{ NULL,							NULL						}
		};

		luaL_setfuncs(L, methods, 0);

		// Push flags as (imgui.flagName.flagElementKey = flagElementValue)
		for (const auto &flag : ImLua::GetFlags())
		{
			lua_newtable(L);
			for (const auto &value : flag.values)
			{
				lua_pushinteger(L, value.second);
				lua_setfield(L, -2, value.first.c_str());
			}
			lua_setfield(L, -2, flag.flagName.c_str());
		}

		lua_setglobal(L, "imgui");
	
		LuaDoFileCleaned(L, LuaFilePath("Utility/ImLua"));
	}


	bool ImLua::PopBool(lua_State *L, int &index, bool &value)
	{
		if (!lua_isboolean(L, index))
			return false;

		value = lua_toboolean(L, index++);
		return true;
	}
	bool ImLua::PopInt(lua_State *L, int &index, int &value)
	{
		if (!lua_isinteger(L, index))
			return false;

		value = lua_tointeger(L, index++);
		return true;
	}
	bool ImLua::PopFloat(lua_State *L, int &index, float &value)
	{
		if (!lua_isnumber(L, index))
			return false;

		value = (float)lua_tonumber(L, index++);
		return true;
	}
	bool ImLua::PopString(lua_State *L, int &index, std::string &value)
	{
		if (!lua_isstring(L, index))
			return false;

		value = lua_tostring(L, index++);
		return true;
	}
	bool ImLua::PopImVec2(lua_State *L, int &index, ImVec2 &value)
	{
		if (!lua_istable(L, index))
			return false;

		lua_getfield(L, index, "x");
		value.x = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, index, "y");
		value.y = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);

		index++;
		return true;
	}
	bool ImLua::PopImVec4(lua_State *L, int &index, ImVec4 &value)
	{
		if (!lua_istable(L, index))
			return false;

		lua_getfield(L, index, "x");
		value.x = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, index, "y");
		value.y = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, index, "z");
		value.x = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, index, "w");
		value.y = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);

		index++;
		return true;
	}

	void ImLua::PushNil(lua_State *L)
	{
		lua_pushnil(L);
	}
	void ImLua::PushBool(lua_State *L, const bool &value)
	{
		lua_pushboolean(L, value);
	}
	void ImLua::PushInt(lua_State *L, const int &value)
	{
		lua_pushinteger(L, value);
	}
	void ImLua::PushFloat(lua_State *L, const float &value)
	{
		lua_pushnumber(L, value);
	}
	void ImLua::PushString(lua_State *L, const char *value)
	{
		lua_pushstring(L, value);
	}
	void ImLua::PushString(lua_State *L, const std::string &value)
	{
		lua_pushstring(L, value.c_str());
	}
	void ImLua::PushImVec2(lua_State *L, const ImVec2 &value)
	{
		lua_createtable(L, 0, 2);
		lua_pushnumber(L, value.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, value.y);
		lua_setfield(L, -2, "y");
	}
	void ImLua::PushImVec4(lua_State *L, const ImVec4 &value)
	{
		lua_createtable(L, 0, 4);
		lua_pushnumber(L, value.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, value.y);
		lua_setfield(L, -2, "y");
		lua_pushnumber(L, value.z);
		lua_setfield(L, -2, "z");
		lua_pushnumber(L, value.w);
		lua_setfield(L, -2, "w");
	}


	int ImLua::lua_Begin(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		// Optional
		bool value = true;
		ImGuiWindowFlags flags = 0;

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		bool *valuePtr = (bool*)0;
		// Get optional parameters
		do
		{
			if (!PopBool(L, idx, value))
				break;
			valuePtr = &value;

			if (!PopInt(L, idx, flags))
				break;

		} while (false);

		// Do ImGui command
		bool isOpen = ImGui::Begin(label.c_str(), valuePtr, flags);

		// Return result
		PushBool(L, isOpen);

		return 1;
	}

	int ImLua::lua_End(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		ImGui::End();

		return 0;
	}

	int ImLua::lua_BeginChild(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		// Optional
		ImVec2 size = ImVec2(0, 0);
		ImGuiChildFlags child_flags = 0;
		ImGuiWindowFlags window_flags = 0;

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		// Get optional parameters
		do
		{
			if (!PopImVec2(L, idx, size))
				break;

			if (!PopInt(L, idx, child_flags))
				break;

			if (!PopInt(L, idx, window_flags))
				break;

		} while (false);

		// Do ImGui command
		bool isOpen = ImGui::BeginChild(label.c_str(), size, child_flags, window_flags);

		// Return result
		PushBool(L, isOpen);

		return 1;
	}

	int ImLua::lua_EndChild(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		ImGui::EndChild();

		return 0;
	}

	int ImLua::lua_OpenPopup(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		// Optional
		ImGuiPopupFlags popup_flags = 0;

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		// Get optional parameters
		do
		{
			if (!PopInt(L, idx, popup_flags))
				break;

		} while (false);

		// Do ImGui command
		ImGui::OpenPopup(label.c_str(), popup_flags);

		return 0;
	}

	int ImLua::lua_IsPopupOpen(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string str_id;
		// Optional
		ImGuiPopupFlags flags = 0;

		// Get required parameters
		if (!PopString(L, idx, str_id))
			luaL_error(L, "Expected parameter string");

		// Get optional parameters
		do
		{
			if (!PopInt(L, idx, flags))
				break;

		} while (false);

		// Do ImGui command
		bool isOpen = ImGui::IsPopupOpen(str_id.c_str(), flags);

		// Return result
		PushBool(L, isOpen);

		return 1;
	}

	int ImLua::lua_CloseCurrentPopup(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		ImGui::CloseCurrentPopup();

		return 0;
	}

	int ImLua::lua_BeginPopup(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		// Optional
		ImGuiWindowFlags flags = 0;

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		// Get optional parameters
		do
		{
			if (!PopInt(L, idx, flags))
				break;

		} while (false);

		// Do ImGui command
		bool isOpen = ImGui::BeginPopup(label.c_str(), flags);

		// Return result
		PushBool(L, isOpen);

		return 1;
	}

	int ImLua::lua_BeginPopupModal(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string name;
		// Optional
		bool open = true;
		ImGuiWindowFlags flags = 0;

		// Get required parameters
		if (!PopString(L, idx, name))
			luaL_error(L, "Expected parameter string");

		bool *p_open = nullptr;
		// Get optional parameters
		do
		{
			if (!PopBool(L, idx, open))
				break;
			p_open = &open;

			if (!PopInt(L, idx, flags))
				break;

		} while (false);

		// Do ImGui command
		bool isOpen = ImGui::BeginPopupModal(name.c_str(), p_open, flags);

		// Return result
		PushBool(L, isOpen);
		PushBool(L, open);

		return 2;
	}

	int ImLua::lua_BeginPopupContextItem(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Optional
		std::string str_id;
		ImGuiPopupFlags popup_flags = 1;

		// Get optional parameters
		const char *str_id_ptr = nullptr;
		do
		{
			if (!PopString(L, idx, str_id))
				break;
			str_id_ptr = str_id.c_str();

			if (!PopInt(L, idx, popup_flags))
				break;

		} while (false);

		bool isOpen = ImGui::BeginPopupContextItem(str_id_ptr, popup_flags);

		PushBool(L, isOpen);

		return 1;
	}

	int ImLua::lua_EndPopup(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		ImGui::EndPopup();

		return 0;
	}

	int ImLua::lua_Separator(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		if (lua_isstring(L, 1))
			ImGui::SeparatorText(lua_tostring(L, 1));
		else
			ImGui::Separator();

		return 0;
	}

	int ImLua::lua_Text(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		// Do ImGui command
		ImGui::Text(label.c_str());

		return 0;
	}

	int ImLua::lua_TextWrapped(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		// Do ImGui command
		ImGui::TextWrapped(label.c_str());

		return 0;
	}

	int ImLua::lua_InputText(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		std::string input;
		// Optional
		int size = -1;

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		if (!PopString(L, idx, input))
			luaL_error(L, "Expected parameter string");

		// Get optional parameters
		do
		{
			if (!PopInt(L, idx, size))
				break;

		} while (false);

		// Do ImGui command
		TempString value(input, size);
		bool isModified = ImGui::InputText(label.c_str(), value, value.Size);

		// Return result
		PushString(L, value);
		PushBool(L, isModified);

		return 2;
	}

	int ImLua::lua_DragFloat(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		float value = 0.0f;
		// Optional
		float vSpeed = 1.0f;
		float vMin = 0.0f;
		float vMax = 0.0f;
		std::string format = "%.3f";

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		if (!PopFloat(L, idx, value))
			luaL_error(L, "Expected parameter float");
	
		// Get optional parameters
		do
		{
			if (!PopFloat(L, idx, vSpeed))
				break;

			if (!PopFloat(L, idx, vMin))
				break;

			if (!PopFloat(L, idx, vMax))
				break;

			if (!PopString(L, idx, format))
				break;

		} while (false);

		// Do ImGui command
		bool isModified = ImGui::DragFloat(label.c_str(), &value, vSpeed, vMin, vMax, format.c_str());

		// Return result
		PushFloat(L, value);
		PushBool(L, isModified);

		return 2;
	}

	int ImLua::lua_DragFloat2(lua_State* L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		ImVec2 value;
		// Optional
		float vSpeed = 1.0f;
		float vMin = 0.0f;
		float vMax = 0.0f;
		std::string format = "%.3f";

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		if (!PopImVec2(L, idx, value))
			luaL_error(L, "Expected parameter ImVec2");

		// Get optional parameters
		do
		{
			if (!PopFloat(L, idx, vSpeed))
				break;

			if (!PopFloat(L, idx, vMin))
				break;

			if (!PopFloat(L, idx, vMax))
				break;

			if (!PopString(L, idx, format))
				break;

		} while (false);

		// Do ImGui command
		bool isModified = ImGui::DragFloat2(label.c_str(), (float*)&value, vSpeed, vMin, vMax, format.c_str());

		// Return result
		PushImVec2(L, value);
		PushBool(L, isModified);

		return 2;
	}

	int ImLua::lua_DragInt(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		int value = 0;
		// Optional
		float vSpeed = 1.0f;
		int vMin = 0;
		int vMax = 0;
		std::string format = "%d";

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		if (!PopInt(L, idx, value))
			luaL_error(L, "Expected parameter int");

		// Get optional parameters
		do
		{
			if (!PopFloat(L, idx, vSpeed))
				break;

			if (!PopInt(L, idx, vMin))
				break;

			if (!PopInt(L, idx, vMax))
				break;

			if (!PopString(L, idx, format))
				break;

		} while (false);

		// Do ImGui command
		bool isModified = ImGui::DragInt(label.c_str(), &value, vSpeed, vMin, vMax, format.c_str());

		// Return result
		PushInt(L, value);
		PushBool(L, isModified);

		return 2;
	}

	int ImLua::lua_Button(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		// Optional
		ImVec2 size = ImVec2(0, 0);

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		// Get optional parameters
		do
		{
			if (!PopImVec2(L, idx, size))
				break;

		} while (false);
	
		// Do ImGui command
		bool pressed = ImGui::Button(label.c_str(), size);

		// Return result
		PushBool(L, pressed);

		return 1;
	}

	int ImLua::lua_Checkbox(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		bool value = false;

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		if (!PopBool(L, idx, value))
			luaL_error(L, "Expected parameter bool");

		// Do ImGui command
		bool pressed = ImGui::Checkbox(label.c_str(), &value);

		// Return result
		PushBool(L, value);
		PushBool(L, pressed);

		return 2;
	}

	int ImLua::lua_RadioButton(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		bool active = false;

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		if (!PopBool(L, idx, active))
			luaL_error(L, "Expected parameter bool");

		// Do ImGui command
		bool pressed = ImGui::RadioButton(label.c_str(), active);

		// Return result
		PushBool(L, pressed);

		return 1;
	}

	int ImLua::lua_RadioButtonInt(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		int v = 0;
		int v_button = 0;

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		if (!PopInt(L, idx, v))
			luaL_error(L, "Expected parameter int");

		if (!PopInt(L, idx, v_button))
			luaL_error(L, "Expected parameter int");

		// Do ImGui command
		bool pressed = ImGui::RadioButton(label.c_str(), &v, v_button);

		// Return result
		PushInt(L, v);
		PushBool(L, pressed);

		return 2;
	}

	int ImLua::lua_InputFloat(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		float value = 0.0f;
		// Optional
		float step = 0.0f;
		float step_fast = 0.0f;
		std::string format = "%.3f";

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		if (!PopFloat(L, idx, value))
			luaL_error(L, "Expected parameter float");

		// Get optional parameters
		do
		{
			if (!PopFloat(L, idx, step))
				break;

			if (!PopFloat(L, idx, step_fast))
				break;

			if (!PopString(L, idx, format))
				break;

		} while (false);

		// Do ImGui command
		bool isModified = ImGui::InputFloat(label.c_str(), &value, step, step_fast, format.c_str());

		// Return result
		PushFloat(L, value);
		PushBool(L, isModified);

		return 2;
	}

	int ImLua::lua_SliderFloat(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		float value = 0.0f;
		// Optional
		float vMin = 0.0f;
		float vMax = 0.0f;
		std::string format = "%.3f";

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		if (!PopFloat(L, idx, value))
			luaL_error(L, "Expected parameter float");

		// Get optional parameters
		do
		{
			if (!PopFloat(L, idx, vMin))
				break;

			if (!PopFloat(L, idx, vMax))
				break;

			if (!PopString(L, idx, format))
				break;

		} while (false);

		// Do ImGui command
		bool isModified = ImGui::SliderFloat(label.c_str(), &value, vMin, vMax, format.c_str());

		// Return result
		PushFloat(L, value);
		PushBool(L, isModified);

		return 2;
	}

	int ImLua::lua_InputFloat2(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		ImVec2 value;
		// Optional
		std::string format = "%.3f";

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		if (!PopImVec2(L, idx, value))
			luaL_error(L, "Expected parameter ImVec2");

		// Get optional parameters
		do
		{
			if (!PopString(L, idx, format))
				break;

		} while (false);

		// Do ImGui command
		bool isModified = ImGui::InputFloat2(label.c_str(), (float*)&value, format.c_str());

		// Return result
		PushImVec2(L, value);
		PushBool(L, isModified);

		return 2;
	}

	int ImLua::lua_SliderInt(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		int value = 0;
		// Optional
		int vMin = 0;
		int vMax = 0;
		std::string format = "%d";

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		if (!PopInt(L, idx, value))
			luaL_error(L, "Expected parameter int");

		// Get optional parameters
		do
		{
			if (!PopInt(L, idx, vMin))
				break;

			if (!PopInt(L, idx, vMax))
				break;

			if (!PopString(L, idx, format))
				break;

		} while (false);

		// Do ImGui command
		bool isModified = ImGui::SliderInt(label.c_str(), &value, vMin, vMax, format.c_str());

		// Return result
		PushInt(L, value);
		PushBool(L, isModified);

		return 2;
	}

	int ImLua::lua_InputInt(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		int value = 0;
		// Optional
		int step = 1;
		int step_fast = 100;

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		if (!PopInt(L, idx, value))
			luaL_error(L, "Expected parameter int");

		// Get optional parameters
		do
		{
			if (!PopInt(L, idx, step))
				break;

			if (!PopInt(L, idx, step_fast))
				break;

		} while (false);

		// Do ImGui command
		bool isModified = ImGui::InputInt(label.c_str(), &value, step, step_fast);

		// Return result
		PushInt(L, value);
		PushBool(L, isModified);

		return 2;
	}

	int ImLua::lua_SameLine(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Optional
		float offsetFromStartX = 0.0f;
		float spacing = -1.0f;

		// Get optional parameters
		do
		{
			if (!PopFloat(L, idx, offsetFromStartX))
				break;

			if (!PopFloat(L, idx, spacing))
				break;

		} while (false);

		// Do ImGui command
		ImGui::SameLine(offsetFromStartX, spacing);

		return 0;
	}

	int ImLua::lua_BeginCombo(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		std::string previewValue;

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		if (!PopString(L, idx, previewValue))
			luaL_error(L, "Expected parameter string");

		// Do ImGui command
		bool isOpen = ImGui::BeginCombo(label.c_str(), previewValue.c_str());

		// Return result
		PushBool(L, isOpen);

		return 1;
	}

	int ImLua::lua_Combo(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		int currentItem;
		std::string itemsSeparatedByNewlines;
		// Optional
		int popupMaxHeightInItems = -1;

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		if (!PopInt(L, idx, currentItem))
			luaL_error(L, "Expected parameter int");

		if (!PopString(L, idx, itemsSeparatedByNewlines))
			luaL_error(L, "Expected parameter string");

		// Get optional parameters
		do
		{
			if (!PopInt(L, idx, popupMaxHeightInItems))
				break;

		} while (false);

		// Replace \n with \0
		for (size_t i = 0; i < itemsSeparatedByNewlines.length(); i++)
		{
			if (itemsSeparatedByNewlines[i] == '\n')
				itemsSeparatedByNewlines[i] = '\0';
		}

		// Do ImGui command
		bool pressed = ImGui::Combo(label.c_str(), &currentItem, itemsSeparatedByNewlines.c_str(), popupMaxHeightInItems);

		// Return result
		PushBool(L, pressed);
		PushInt(L, currentItem);

		return 2;
	}

	int ImLua::lua_EndCombo(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		ImGui::EndCombo();
		return 0;
	}

	int ImLua::lua_Selectable(lua_State* L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		bool selected = false;
		// Optional
		ImGuiSelectableFlags flags = 0;
		ImVec2 size(0, 0);

		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		if (!PopBool(L, idx, selected))
			luaL_error(L, "Expected parameter int");

		// Get optional parameters
		do
		{
			if (!PopInt(L, idx, flags))
				break;

			if (!PopImVec2(L, idx, size))
				break;

		} while (false);

		// Do ImGui command
		bool pressed = ImGui::Selectable(label.c_str(), selected, flags, size);

		// Return result
		PushBool(L, pressed);

		return 1;
	}

	int ImLua::lua_IsMouseClicked(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		ImGuiMouseButton button = 0;
		// Optional
		bool repeat = false;

		// Get required parameters
		if (!PopInt(L, idx, button))
			luaL_error(L, "Expected parameter ImGuiMouseButton");

		// Get optional parameters
		do
		{
			if (!PopBool(L, idx, repeat))
				break;

		} while (false);

		// Do ImGui command
		bool ret = ImGui::IsMouseClicked(button, repeat);

		// Return result
		PushBool(L, ret);

		return 1;
	}

	int ImLua::lua_IsWindowFocused(lua_State* L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Optional
		ImGuiFocusedFlags flags = 0;

		// Get optional parameters
		do
		{
			if (!PopInt(L, idx, flags))
				break;

		} while (false);

		// Do ImGui command
		bool ret = ImGui::IsWindowFocused(flags);

		// Return result
		PushBool(L, ret);

		return 1;
	}

	int ImLua::lua_SetKeyboardFocusHere(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Optional
		int offset = 0;

		// Get optional parameters
		do
		{
			if (!PopInt(L, idx, offset))
				break;

		} while (false);

		// Do ImGui command
		ImGui::SetKeyboardFocusHere(offset);

		return 0;
	}

	int ImLua::lua_SetItemDefaultFocus(lua_State* L)
	{
		ZoneScopedC(RandomUniqueColor());

		ImGui::SetItemDefaultFocus();
		return 0;
	}

	int ImLua::lua_CollapsingHeader(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Required
		std::string label;
		// Optional
		bool visible = true;
		ImGuiTreeNodeFlags flags = 0;

		// Get required parameters
		if (!PopString(L, idx, label))
			luaL_error(L, "Expected parameter string");

		// Get optional parameters
		bool *p_visible = nullptr;
		do
		{
			if (!PopInt(L, idx, flags))
				break;

			if (!PopBool(L, idx, visible))
				break;
			p_visible = &visible;

		} while (false);

		// Do ImGui command
		bool open = false;
		if (p_visible)
			open = ImGui::CollapsingHeader(label.c_str(), p_visible, flags);
		else
			open = ImGui::CollapsingHeader(label.c_str(), flags);

		// Return result
		int returns = 1;
		PushBool(L, open);

		if (p_visible)
		{
			PushBool(L, visible);
			returns++;
		}

		return returns;
	}

	int ImLua::lua_Indent(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Optional
		float indent_w = 0.0f;

		// Get optional parameters
		do
		{
			if (!PopFloat(L, idx, indent_w))
				break;

		} while (false);

		// Do ImGui command
		ImGui::Indent(indent_w);

		return 0;
	}

	int ImLua::lua_Unindent(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());
		int idx = 1;

		// Optional
		float indent_w = 0.0f;

		// Get optional parameters
		do
		{
			if (!PopFloat(L, idx, indent_w))
				break;

		} while (false);

		// Do ImGui command
		ImGui::Unindent(indent_w);

		return 0;
	}

	int ImLua::lua_IsAnyItemActive(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// Do ImGui command
		bool ret = ImGui::IsAnyItemActive();

		// Return result
		PushBool(L, ret);

		return 1;
	}










	int ImLua::lua_SliderAngle(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_ColorEdit4(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_ColorPicker4(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_ColorButton(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_ProgressBar(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_Image(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_TreeNode(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_TreePop(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_BeginGroup(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_EndGroup(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_PushID(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_PopID(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_BeginTooltip(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_EndTooltip(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_BeginTable(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_EndTable(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_TableNextRow(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_TableNextColumn(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_TableSetColumnIndex(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_NewLine(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_Spacing(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_Dummy(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_SetNextItemWidth(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_CalcItemWidth(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_GetCursorScreenPos(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_SetCursorScreenPos(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_GetCursorPos(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_SetCursorPos(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_GetWindowPos(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_GetWindowSize(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_SetWindowPos(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_SetWindowSize(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_IsItemHovered(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_IsItemActive(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_IsItemFocused(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_IsItemClicked(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_IsItemVisible(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_IsItemEdited(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_IsItemActivated(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_IsItemToggledOpen(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_GetItemID(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_GetItemRectMin(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_GetItemRectMax(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_GetItemRectSize(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_CalcTextSize(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_GetClipboardText(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}

	int ImLua::lua_SetClipboardText(lua_State *L)
	{
		ZoneScopedC(RandomUniqueColor());

		// TODO: Implement this function
		return 0;
	}



	const std::vector<ImLua::ImGuiFlag> &ImLua::GetFlags()
	{
		static const std::vector<ImGuiFlag> flags = {

			ImGuiFlag("ImGuiMouseButton", {
				std::make_pair<std::string, int>("Left", ImGuiMouseButton_Left),
				std::make_pair<std::string, int>("Right", ImGuiMouseButton_Right),
				std::make_pair<std::string, int>("Middle", ImGuiMouseButton_Middle),
				std::make_pair<std::string, int>("COUNT", ImGuiMouseButton_COUNT)
			}),
				
			ImGuiFlag("ImGuiWindowFlags", {
				std::make_pair<std::string, int>("None", ImGuiWindowFlags_None),
				std::make_pair<std::string, int>("NoTitleBar", ImGuiWindowFlags_NoTitleBar),
				std::make_pair<std::string, int>("NoResize", ImGuiWindowFlags_NoResize),
				std::make_pair<std::string, int>("NoMove", ImGuiWindowFlags_NoMove),
				std::make_pair<std::string, int>("NoScrollbar", ImGuiWindowFlags_NoScrollbar),
				std::make_pair<std::string, int>("NoScrollWithMouse", ImGuiWindowFlags_NoScrollWithMouse),
				std::make_pair<std::string, int>("NoCollapse", ImGuiWindowFlags_NoCollapse),
				std::make_pair<std::string, int>("AlwaysAutoResize", ImGuiWindowFlags_AlwaysAutoResize),
				std::make_pair<std::string, int>("NoBackground", ImGuiWindowFlags_NoBackground),
				std::make_pair<std::string, int>("NoSavedSettings", ImGuiWindowFlags_NoSavedSettings),
				std::make_pair<std::string, int>("NoMouseInputs", ImGuiWindowFlags_NoMouseInputs),
				std::make_pair<std::string, int>("MenuBar", ImGuiWindowFlags_MenuBar),
				std::make_pair<std::string, int>("HorizontalScrollbar", ImGuiWindowFlags_HorizontalScrollbar),
				std::make_pair<std::string, int>("NoFocusOnAppearing", ImGuiWindowFlags_NoFocusOnAppearing),
				std::make_pair<std::string, int>("NoBringToFrontOnFocus", ImGuiWindowFlags_NoBringToFrontOnFocus),
				std::make_pair<std::string, int>("AlwaysVerticalScrollbar", ImGuiWindowFlags_AlwaysVerticalScrollbar),
				std::make_pair<std::string, int>("AlwaysHorizontalScrollbar", ImGuiWindowFlags_AlwaysHorizontalScrollbar),
				std::make_pair<std::string, int>("NoNavInputs", ImGuiWindowFlags_NoNavInputs),
				std::make_pair<std::string, int>("NoNavFocus", ImGuiWindowFlags_NoNavFocus),
				std::make_pair<std::string, int>("UnsavedDocument", ImGuiWindowFlags_UnsavedDocument),
				std::make_pair<std::string, int>("NoDocking", ImGuiWindowFlags_NoDocking),
				std::make_pair<std::string, int>("NoNav", ImGuiWindowFlags_NoNav),
				std::make_pair<std::string, int>("NoDecoration", ImGuiWindowFlags_NoDecoration),
				std::make_pair<std::string, int>("NoInputs", ImGuiWindowFlags_NoInputs)
			}),

			ImGuiFlag("ImGuiChildFlags", {
				std::make_pair<std::string, int>("None", ImGuiChildFlags_None),
				std::make_pair<std::string, int>("Borders", ImGuiChildFlags_Borders),
				std::make_pair<std::string, int>("AlwaysUseWindowPadding", ImGuiChildFlags_AlwaysUseWindowPadding),
				std::make_pair<std::string, int>("ResizeX", ImGuiChildFlags_ResizeX),
				std::make_pair<std::string, int>("ResizeY", ImGuiChildFlags_ResizeY),
				std::make_pair<std::string, int>("AutoResizeX", ImGuiChildFlags_AutoResizeX),
				std::make_pair<std::string, int>("AutoResizeY", ImGuiChildFlags_AutoResizeY),
				std::make_pair<std::string, int>("AlwaysAutoResize", ImGuiChildFlags_AlwaysAutoResize),
				std::make_pair<std::string, int>("FrameStyle", ImGuiChildFlags_FrameStyle),
				std::make_pair<std::string, int>("NavFlattened", ImGuiChildFlags_NavFlattened),
			}),

			ImGuiFlag("ImGuiPopupFlags", {
				std::make_pair<std::string, int>("None", ImGuiPopupFlags_None),
				std::make_pair<std::string, int>("MouseButtonLeft", ImGuiPopupFlags_MouseButtonLeft),
				std::make_pair<std::string, int>("MouseButtonRight", ImGuiPopupFlags_MouseButtonRight),
				std::make_pair<std::string, int>("MouseButtonMiddle", ImGuiPopupFlags_MouseButtonMiddle),
				std::make_pair<std::string, int>("MouseButtonMask_", ImGuiPopupFlags_MouseButtonMask_),
				std::make_pair<std::string, int>("MouseButtonDefault_", ImGuiPopupFlags_MouseButtonDefault_),
				std::make_pair<std::string, int>("NoReopen", ImGuiPopupFlags_NoReopen),
				std::make_pair<std::string, int>("NoOpenOverExistingPopup", ImGuiPopupFlags_NoOpenOverExistingPopup),
				std::make_pair<std::string, int>("NoOpenOverItems", ImGuiPopupFlags_NoOpenOverItems),
				std::make_pair<std::string, int>("AnyPopupId", ImGuiPopupFlags_AnyPopupId),
				std::make_pair<std::string, int>("AnyPopupLevel", ImGuiPopupFlags_AnyPopupLevel),
				std::make_pair<std::string, int>("AnyPopup", ImGuiPopupFlags_AnyPopup),
			}),

			ImGuiFlag("ImGuiSelectableFlags", {
				std::make_pair<std::string, int>("None", ImGuiSelectableFlags_None),
				std::make_pair<std::string, int>("NoAutoClosePopups", ImGuiSelectableFlags_NoAutoClosePopups),
				std::make_pair<std::string, int>("SpanAllColumns", ImGuiSelectableFlags_SpanAllColumns),
				std::make_pair<std::string, int>("AllowDoubleClick", ImGuiSelectableFlags_AllowDoubleClick),
				std::make_pair<std::string, int>("Disabled", ImGuiSelectableFlags_Disabled),
				std::make_pair<std::string, int>("AllowOverlap", ImGuiSelectableFlags_AllowOverlap),
				std::make_pair<std::string, int>("Highlight", ImGuiSelectableFlags_Highlight),
			}),

			ImGuiFlag("ImGuiFocusedFlags", {
				std::make_pair<std::string, int>("None", ImGuiFocusedFlags_None),
				std::make_pair<std::string, int>("ChildWindows", ImGuiFocusedFlags_ChildWindows),
				std::make_pair<std::string, int>("RootWindow", ImGuiFocusedFlags_RootWindow),
				std::make_pair<std::string, int>("AnyWindow", ImGuiFocusedFlags_AnyWindow),
				std::make_pair<std::string, int>("NoPopupHierarchy", ImGuiFocusedFlags_NoPopupHierarchy),
				std::make_pair<std::string, int>("DockHierarchy", ImGuiFocusedFlags_DockHierarchy),
				std::make_pair<std::string, int>("RootAndChildWindows", ImGuiFocusedFlags_RootAndChildWindows)
			}),

			ImGuiFlag("ImGuiTreeNodeFlags", {
				std::make_pair<std::string, int>("None", ImGuiTreeNodeFlags_None),
				std::make_pair<std::string, int>("Selected", ImGuiTreeNodeFlags_Selected),
				std::make_pair<std::string, int>("Framed", ImGuiTreeNodeFlags_Framed),
				std::make_pair<std::string, int>("AllowOverlap", ImGuiTreeNodeFlags_AllowOverlap),
				std::make_pair<std::string, int>("NoTreePushOnOpen", ImGuiTreeNodeFlags_NoTreePushOnOpen),
				std::make_pair<std::string, int>("NoAutoOpenOnLog", ImGuiTreeNodeFlags_NoAutoOpenOnLog),
				std::make_pair<std::string, int>("DefaultOpen", ImGuiTreeNodeFlags_DefaultOpen),
				std::make_pair<std::string, int>("OpenOnDoubleClick", ImGuiTreeNodeFlags_OpenOnDoubleClick),
				std::make_pair<std::string, int>("OpenOnArrow", ImGuiTreeNodeFlags_OpenOnArrow),
				std::make_pair<std::string, int>("Leaf", ImGuiTreeNodeFlags_Leaf),
				std::make_pair<std::string, int>("Bullet", ImGuiTreeNodeFlags_Bullet),
				std::make_pair<std::string, int>("FramePadding", ImGuiTreeNodeFlags_FramePadding),
				std::make_pair<std::string, int>("SpanAvailWidth", ImGuiTreeNodeFlags_SpanAvailWidth),
				std::make_pair<std::string, int>("SpanFullWidth", ImGuiTreeNodeFlags_SpanFullWidth),
				std::make_pair<std::string, int>("SpanLabelWidth", ImGuiTreeNodeFlags_SpanLabelWidth),
				std::make_pair<std::string, int>("SpanAllColumns", ImGuiTreeNodeFlags_SpanAllColumns),
				std::make_pair<std::string, int>("LabelSpanAllColumns", ImGuiTreeNodeFlags_LabelSpanAllColumns),
				std::make_pair<std::string, int>("NavLeftJumpsBackHere", ImGuiTreeNodeFlags_NavLeftJumpsBackHere),
				std::make_pair<std::string, int>("CollapsingHeader", ImGuiTreeNodeFlags_CollapsingHeader)
			}),
		};

		return flags;
	}
}
