#include "stdafx.h"
#include "ImLua.h"


void ImLua::ImLua::lua_openimgui(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());

	lua_newtable(L);

	luaL_Reg methods[] = {
		{ "Separator",			lua_Separator		},
		{ "Text",				lua_Text			},
		{ "InputText",			lua_InputText		},
		{ "DragFloat",			lua_DragFloat		},
		{ NULL,					NULL				}
	};

	luaL_setfuncs(L, methods, 0);

	lua_setglobal(L, "imgui");
	
	LuaDoFileCleaned(L, LuaFilePath("Utility/ImLua"));
}


int ImLua::ImLua::lua_Separator(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());

	if (lua_isstring(L, 1))
		ImGui::SeparatorText(lua_tostring(L, 1));
	else
		ImGui::Separator();

	return 1;
}

int ImLua::ImLua::lua_Text(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());
	int idx = 1;

	// Required
	const char *label = nullptr;

	// Get required parameters
	if (!lua_isstring(L, idx))
		luaL_error(L, "Expected parameter string");
	label = lua_tostring(L, idx++);

	// Do ImGui command
	ImGui::Text(label);

	return 1;
}

int ImLua::ImLua::lua_InputText(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());
	int idx = 1;

	// Required
	const char *label = nullptr;
	const char *input = nullptr;
	// Optional
	size_t size = -1;

	// Get required parameters
	if (!lua_isstring(L, idx))
		luaL_error(L, "Expected parameter string");
	label = lua_tostring(L, idx++);

	if (!lua_isstring(L, idx))
		luaL_error(L, "Expected parameter string");
	input = lua_tostring(L, idx++);

	// Get optional parameters
	do
	{
		if (!lua_isinteger(L, idx))
			break;
		size = lua_tointeger(L, idx++);

	} while (false);

	// Do ImGui command
	TempString value(input, size);
	bool wasModified = ImGui::InputText(label, value, value.Size);

	// Return result
	if (wasModified)
		lua_pushstring(L, value);
	else
		lua_pushnil(L);

	return 1;
}

int ImLua::ImLua::lua_DragFloat(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());
	int idx = 1;

	// Required
	const char *label = nullptr;
	float value = 0.0f;
	// Optional
	float vSpeed = 1.0f;
	float vMin = 0.0f;
	float vMax = 0.0f;
	const char *format = "%.3f";

	// Get required parameters
	if (!lua_isstring(L, idx))
		luaL_error(L, "Expected parameter string");
	label = lua_tostring(L, idx++);

	if (!lua_isnumber(L, idx))
		luaL_error(L, "Expected parameter float");
	value = (float)lua_tonumber(L, idx++);
	
	// Get optional parameters
	do
	{
		if (!lua_isnumber(L, idx))
			break;
		vSpeed = lua_tonumber(L, idx++);

		if (!lua_isnumber(L, idx))
			break;
		vMin = lua_tonumber(L, idx++);

		if (!lua_isnumber(L, idx))
			break;
		vMax = lua_tonumber(L, idx++);

		if (!lua_isstring(L, idx))
			break;
		format = lua_tostring(L, idx++);

	} while (false);

	// Do ImGui command
	bool wasModified = ImGui::DragFloat(label, &value, vSpeed, vMin, vMax, format);

	// Return result
	if (wasModified)
		lua_pushnumber(L, value);
	else
		lua_pushnil(L);

	return 1;
}
