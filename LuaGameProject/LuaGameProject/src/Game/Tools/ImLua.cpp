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


bool ImLua::ImLua::PopBool(lua_State *L, int &index, bool &value)
{
	if (!lua_isboolean(L, index))
		return false;

	value = lua_toboolean(L, index++);
	return true;
}

bool ImLua::ImLua::PopInt(lua_State *L, int &index, int &value)
{
	if (!lua_isinteger(L, index))
		return false;

	value = lua_tointeger(L, index++);
	return true;
}

bool ImLua::ImLua::PopFloat(lua_State *L, int &index, float &value)
{
	if (!lua_isnumber(L, index))
		return false;

	value = (float)lua_tonumber(L, index++);
	return true;
}

bool ImLua::ImLua::PopString(lua_State *L, int &index, std::string &value)
{
	if (!lua_isstring(L, index))
		return false;

	value = lua_tostring(L, index++);
	return true;
}

bool ImLua::ImLua::PopImVec2(lua_State *L, int &index, ImVec2 &value)
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

bool ImLua::ImLua::PopImVec4(lua_State *L, int &index, ImVec4 &value)
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


void ImLua::ImLua::PushNil(lua_State *L)
{
	lua_pushnil(L);
}

void ImLua::ImLua::PushBool(lua_State *L, const bool &value)
{
	lua_pushboolean(L, value);
}

void ImLua::ImLua::PushInt(lua_State *L, const int &value)
{
	lua_pushinteger(L, value);
}

void ImLua::ImLua::PushFloat(lua_State *L, const float &value)
{
	lua_pushnumber(L, value);
}

void ImLua::ImLua::PushString(lua_State *L, const char *value)
{
	lua_pushstring(L, value);
}

void ImLua::ImLua::PushString(lua_State *L, const std::string &value)
{
	lua_pushstring(L, value.c_str());
}

void ImLua::ImLua::PushImVec2(lua_State *L, const ImVec2 &value)
{
	lua_createtable(L, 0, 2);
	lua_pushnumber(L, value.x);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, value.y);
	lua_setfield(L, -2, "y");
}

void ImLua::ImLua::PushImVec4(lua_State *L, const ImVec4 &value)
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
	std::string label;

	// Get required parameters
	if (!PopString(L, idx, label))
		luaL_error(L, "Expected parameter string");

	// Do ImGui command
	ImGui::Text(label.c_str());

	return 1;
}

int ImLua::ImLua::lua_InputText(lua_State *L)
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
	if (isModified)
		PushString(L, value);
	else
		PushNil(L);

	return 1;
}

int ImLua::ImLua::lua_DragFloat(lua_State *L)
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
	if (isModified)
		PushFloat(L, value);
	else
		lua_pushnil(L);

	return 1;
}

int ImLua::ImLua::lua_Button(lua_State *L)
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

int ImLua::ImLua::lua_Checkbox(lua_State *L)
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
	PushBool(L, pressed);
	PushBool(L, value);

	return 1;
}

int ImLua::ImLua::lua_RadioButton(lua_State *L)
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

int ImLua::ImLua::lua_RadioButtonInt(lua_State *L)
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
	PushBool(L, pressed);
	PushInt(L, v);

	return 1;
}

int ImLua::ImLua::lua_SliderFloat(lua_State *L)
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
	if (isModified)
		PushFloat(L, value);
	else
		lua_pushnil(L);

	return 1;
}
