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
				Size = (size > inputSize) ? size * 2 : inputSize;
				Value = new char[Size];

				strcpy_s(Value, Size, input);
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


		// Arguments: string label = nil
		// Returns: none
		static int lua_Separator(lua_State *L);

		// Arguments: string label
		// Returns: none
		static int lua_Text(lua_State *L);

		// Arguments: string label, string buf, int buf_size = -1
		// Returns: string buf (nil if unchanged)
		static int lua_InputText(lua_State *L);

		// Arguments: string label, float v, float v_speed = (1.0F), float v_min = (0.0F), float v_max = (0.0F), string format = "#.3f"
		// Returns: float v (nil if unchanged)
		static int lua_DragFloat(lua_State *L);



		// TODO:
		/*
		* 
		* Button(const char* label, const ImVec2& size = ImVec2(0, 0))
		* Checkbox(const char* label, bool* v)
		* RadioButton(const char* label, bool active)
		* ProgressBar(float fraction, const ImVec2& size_arg = ImVec2(-FLT_MIN, 0), const char* overlay = NULL)
		* 
		* SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
		* SliderAngle(const char* label, float* v_rad, float v_degrees_min = -360.0f, float v_degrees_max = +360.0f, const char* format = "%.0f deg", ImGuiSliderFlags flags = 0)
		* SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0)
		* 
		* InputInt(const char* label, int* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0)
		* 
		* ColorEdit4(const char* label, float col[4], ImGuiColorEditFlags flags = 0)
		* ColorPicker4(const char* label, float col[4], ImGuiColorEditFlags flags = 0, const float* ref_col = NULL)
		* 
		* 
		* ColorButton(const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags = 0, const ImVec2& size = ImVec2(0, 0))
		* 
		* Image(ImTextureID user_texture_id, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1))
		* 
		* 
		* TreeNode(const char* label)
		* TreePop()
		* 
		* CollapsingHeader(const char* label, bool* p_visible, ImGuiTreeNodeFlags flags = 0)
		* 
		* BeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags = 0)
		* EndCombo()
		* 
		* BeginChild(const char* str_id, const ImVec2& size = ImVec2(0, 0), ImGuiChildFlags child_flags = 0, ImGuiWindowFlags window_flags = 0)
		* EndChild()
		* 
		* BeginGroup()
		* EndGroup()
		* 
		* PushID(const char* str_id)
		* PopID()
		* 
		* BeginTooltip()
		* EndTooltip()
		* 
		* BeginPopup(const char* str_id, ImGuiWindowFlags flags = 0)
		* BeginPopupModal(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0)
		* EndPopup()
		* 
		* 
		* BeginTable(const char* str_id, int columns, ImGuiTableFlags flags = 0, const ImVec2& outer_size = ImVec2(0.0f, 0.0f), float inner_width = 0.0f)
		* EndTable()
		* TableNextRow(ImGuiTableRowFlags row_flags = 0, float min_row_height = 0.0f)
		* TableNextColumn()
		* TableSetColumnIndex(int column_n)
		* 
		* 
		* SameLine(float offset_from_start_x=0.0f, float spacing=-1.0f)
		* NewLine()
		* 
		* Spacing()
		* Dummy(const ImVec2& size)
		* 
		* Indent(float indent_w = 0.0f)
		* Unindent(float indent_w = 0.0f)
		* 
		* SetNextItemWidth(float item_width)
		* CalcItemWidth()
		* 
		* 
		* GetCursorScreenPos()
		* SetCursorScreenPos(const ImVec2& pos)
		* 
		* GetCursorPos()
		* SetCursorPos(const ImVec2& local_pos)
		* 
		* 
		* GetWindowPos()
		* GetWindowSize()
		* 
		* SetWindowPos(const ImVec2& pos, ImGuiCond cond = 0)
		* SetWindowSize(const ImVec2& size, ImGuiCond cond = 0)
		* 
		* 
		* IsItemHovered(ImGuiHoveredFlags flags = 0)
		* IsItemActive()
		* IsItemFocused()
		* IsItemClicked(ImGuiMouseButton mouse_button = 0)
		* IsItemVisible()
		* IsItemEdited()
		* IsItemActivated()
		* IsItemToggledOpen()
		* GetItemID()
		* GetItemRectMin()
		* GetItemRectMax()
		* GetItemRectSize()
		* 
		* 
		* CalcTextSize(const char* text, const char* text_end = NULL, bool hide_text_after_double_hash = false, float wrap_width = -1.0f)
		* 
		* 
		* GetClipboardText()
		* SetClipboardText(const char* text)
		* 
		*/

	};
}