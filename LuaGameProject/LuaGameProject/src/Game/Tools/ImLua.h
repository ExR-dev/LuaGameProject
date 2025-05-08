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

		// Arguments: string label
		// Returns: none
		static int lua_Text(lua_State *L);

		// Arguments: string label, string buf, int buf_size = -1
		// Returns: string buf (nil if unchanged)
		static int lua_InputText(lua_State *L);

		// Arguments: string label, float v, float v_speed = (1.0F), float v_min = (0.0F), float v_max = (0.0F), string format = "#.3f"
		// Returns: float v (nil if unchanged)
		static int lua_DragFloat(lua_State *L);
	};
}