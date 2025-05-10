#pragma once

namespace ImLua
{
	struct ImLua
	{
	public:
		ImLua() = delete;
		~ImLua() = delete;

		static void lua_openimgui(lua_State *L);

	private:
		struct TempString
		{
			char *Value;
			size_t Size;

			TempString(const char *input, size_t size = -1)
			{
				size_t inputSize = strlen(input) + 1;
				Size = (size > inputSize) ? size : inputSize * 2;
				Value = new char[Size];

				strcpy_s(Value, Size, input);
			}
			TempString(const std::string &input, size_t size = -1)
			{
				size_t inputSize = input.size();
				Size = (size > inputSize) ? size : inputSize * 2;
				Value = new char[Size];

				strcpy_s(Value, Size, input.c_str());
			}
			~TempString()
			{
				delete[] Value;
			}

			operator const char *() const
			{
				return Value;
			}
			operator char *() const
			{
				return Value;
			}
		};


		static bool PopBool(lua_State *L, int &index, bool &value);
		static bool PopInt(lua_State *L, int &index, int &value);
		static bool PopFloat(lua_State *L, int &index, float &value);
		static bool PopString(lua_State *L, int &index, std::string &value);
		static bool PopImVec2(lua_State *L, int &index, ImVec2 &value);
		static bool PopImVec4(lua_State *L, int &index, ImVec4 &value);

		static void PushNil(lua_State *L);
		static void PushBool(lua_State *L, const bool &value);
		static void PushInt(lua_State *L, const int &value);
		static void PushFloat(lua_State *L, const float &value);
		static void PushString(lua_State *L, const char *value);
		static void PushString(lua_State *L, const std::string &value);
		static void PushImVec2(lua_State *L, const ImVec2 &value);
		static void PushImVec4(lua_State *L, const ImVec4 &value);


		// Arguments: string label = nil
		// Returns: none
		static int lua_Separator(lua_State *L);

		// Arguments: string label
		// Returns: none
		static int lua_Text(lua_State *L);

		// Arguments: string label, string buf, int buf_size = -1
		// Returns: string buf, bool isModified
		static int lua_InputText(lua_State *L);

		// Arguments: string label, float v, float v_speed = (1.0F), float v_min = (0.0F), float v_max = (0.0F), string format = "#.3f"
		// Returns: float v, bool isModified
		static int lua_DragFloat(lua_State *L);

		// Arguments: string label, ImVec2 size = ImVec2(0, 0)
		// Returns: bool pressed
		static int lua_Button(lua_State *L);

		// Arguments: string label, bool v
		// Returns: bool v, bool pressed
		static int lua_Checkbox(lua_State *L);

		// Arguments: string label, bool active
		// Returns: bool pressed
		static int lua_RadioButton(lua_State *L);

		// Arguments: string label, int v, int v_button
		// Returns: int v, bool isPressed
		static int lua_RadioButtonInt(lua_State *L);

		// Arguments: string label, float v, float v_min, float v_max, string format = "%.3f"
		// Returns: float v, bool isModified
		static int lua_SliderFloat(lua_State *L);



		// TODO: Implement these in order of relevance

		// Arguments: string label, float v_rad, float v_degrees_min = -360.0f, float v_degrees_max = +360.0f, string format = "%.0f deg"
		// Returns: float v_rad, bool isModified
		static int lua_SliderAngle(lua_State *L);

		// Arguments: string label, int v, int v_min, int v_max, string format = "%d"
		// Returns: int v, bool isModified
		static int lua_SliderInt(lua_State *L);

		// Arguments: string label, int v, int step = 1, int step_fast = 100
		// Returns: int v, bool isModified
		static int lua_InputInt(lua_State *L);

		// Arguments: string label, ImVec4 col
		// Returns: ImVec4 col, bool isModified
		static int lua_ColorEdit4(lua_State *L);

		// Arguments: string label, ImVec4 col, ImVec4 ref_col = NULL
		// Returns: ImVec4 col, bool isModified
		static int lua_ColorPicker4(lua_State *L);


		// TODO: Complete comments

		// Arguments: string desc_id, ImVec4 col, ImVec2 size = ImVec2(0, 0)
		// Returns: 
		static int lua_ColorButton(lua_State *L);

		// Arguments: float fraction, ImVec2 size_arg = ImVec2(-FLT_MIN, 0), string overlay = NULL
		// Returns: 
		static int lua_ProgressBar(lua_State *L);

		// Arguments: ImTextureID user_texture_id, ImVec2 image_size, ImVec2 uv0 = ImVec2(0, 0), ImVec2 uv1 = ImVec2(1, 1)
		// Returns: 
		static int lua_Image(lua_State *L);

		// Arguments: string label
		// Returns: 
		static int lua_TreeNode(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_TreePop(lua_State *L);

		// Arguments: string label, bool p_visible
		// Returns: bool p_visible, bool isModified
		static int lua_CollapsingHeader(lua_State *L);

		// Arguments: string label, string preview_value
		// Returns: 
		static int lua_BeginCombo(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_EndCombo(lua_State *L);

		// Arguments: string str_id, ImVec2 size = ImVec2(0, 0)
		// Returns: 
		static int lua_BeginChild(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_EndChild(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_BeginGroup(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_EndGroup(lua_State *L);

		// Arguments: string str_id
		// Returns: 
		static int lua_PushID(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_PopID(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_BeginTooltip(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_EndTooltip(lua_State *L);

		// Arguments: string str_id
		// Returns: 
		static int lua_BeginPopup(lua_State *L);

		// Arguments: string name, bool p_open
		// Returns: bool p_open, bool isModified
		static int lua_BeginPopupModal(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_EndPopup(lua_State *L);

		// Arguments: string str_id, int columns, ImVec2 outer_size = ImVec2(0.0f, 0.0f), float inner_width = 0.0f
		// Returns: 
		static int lua_BeginTable(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_EndTable(lua_State *L);

		// Arguments: float min_row_height = 0.0f
		// Returns: 
		static int lua_TableNextRow(lua_State *L);

		// Arguments: int column_n
		// Returns: 
		static int lua_TableNextColumn(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_TableSetColumnIndex(lua_State *L);

		// Arguments: float offset_from_start_x = 0.0f, float spacing = -1.0f
		// Returns: 
		static int lua_SameLine(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_NewLine(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_Spacing(lua_State *L);

		// Arguments: ImVec2 size
		// Returns: 
		static int lua_Dummy(lua_State *L);

		// Arguments: float indent_w = 0.0f
		// Returns: 
		static int lua_Indent(lua_State *L);

		// Arguments: float indent_w = 0.0f
		// Returns: 
		static int lua_Unindent(lua_State *L);

		// Arguments: float item_width
		// Returns: 
		static int lua_SetNextItemWidth(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_CalcItemWidth(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_GetCursorScreenPos(lua_State *L);

		// Arguments: ImVec2 pos
		// Returns: 
		static int lua_SetCursorScreenPos(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_GetCursorPos(lua_State *L);

		// Arguments: ImVec2 local_pos
		// Returns: 
		static int lua_SetCursorPos(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_GetWindowPos(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_GetWindowSize(lua_State *L);

		// Arguments: ImVec2 pos
		// Returns: 
		static int lua_SetWindowPos(lua_State *L);

		// Arguments: ImVec2 size
		// Returns: 
		static int lua_SetWindowSize(lua_State *L);

		// Arguments:
		// Returns: 
		static int lua_IsItemHovered(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_IsItemActive(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_IsItemFocused(lua_State *L);

		// Arguments: ImGuiMouseButton mouse_button = 0
		// Returns: 
		static int lua_IsItemClicked(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_IsItemVisible(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_IsItemEdited(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_IsItemActivated(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_IsItemToggledOpen(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_GetItemID(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_GetItemRectMin(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_GetItemRectMax(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_GetItemRectSize(lua_State *L);

		// Arguments: string text, string text_end = NULL, bool hide_text_after_double_hash = false, float wrap_width = -1.0f
		// Returns: 
		static int lua_CalcTextSize(lua_State *L);

		// Arguments: 
		// Returns: 
		static int lua_GetClipboardText(lua_State *L);

		// Arguments: string text
		// Returns: 
		static int lua_SetClipboardText(lua_State *L);






		/*
		lua_SliderAngle
		lua_SliderInt
		lua_InputInt
		lua_ColorEdit4
		lua_ColorPicker4
		lua_ColorButton
		lua_ProgressBar
		lua_Image
		lua_TreeNode
		lua_TreePop
		lua_CollapsingHeader
		lua_BeginCombo
		lua_EndCombo
		lua_BeginChild
		lua_EndChild
		lua_BeginGroup
		lua_EndGroup
		lua_PushID
		lua_PopID
		lua_BeginTooltip
		lua_EndTooltip
		lua_BeginPopup
		lua_BeginPopupModal
		lua_EndPopup
		lua_BeginTable
		lua_EndTable
		lua_TableNextRow
		lua_TableNextColumn
		lua_TableSetColumnIndex
		lua_SameLine
		lua_NewLine
		lua_Spacing
		lua_Dummy
		lua_Indent
		lua_Unindent
		lua_SetNextItemWidth
		lua_CalcItemWidth
		lua_GetCursorScreenPos
		lua_SetCursorScreenPos
		lua_GetCursorPos
		lua_SetCursorPos
		lua_GetWindowPos
		lua_GetWindowSize
		lua_SetWindowPos
		lua_SetWindowSize
		lua_IsItemHovered
		lua_IsItemActive
		lua_IsItemFocused
		lua_IsItemClicked
		lua_IsItemVisible
		lua_IsItemEdited
		lua_IsItemActivated
		lua_IsItemToggledOpen
		lua_GetItemID
		lua_GetItemRectMin
		lua_GetItemRectMax
		lua_GetItemRectSize
		lua_CalcTextSize
		lua_GetClipboardText
		lua_SetClipboardText

		*/
	};
}