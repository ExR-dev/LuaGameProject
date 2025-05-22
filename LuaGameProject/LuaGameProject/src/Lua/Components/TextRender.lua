local textRender = {}
textRender.__index = textRender

local vec2 = require("Vec2")
local transform = require("Transform2")
local color = require("Color")
local gameMath = require("Utility/GameMath")

local function new(p_text, p_font, p_fontSize, p_spacing, p_textColor, p_offset, p_rotation, p_bgThickness, p_bgColor)
	assert(
		(p_text == nil or type(p_text) == "string") and 
		(p_font == nil or type(p_font) == "string") and 
		(p_fontSize == nil or type(p_fontSize) == "number") and 
		(p_spacing == nil or type(p_spacing) == "number") and 
		(p_textColor == nil or color.iscolor(p_textColor)) and 
		(p_offset == nil or vec2.isvec2(p_offset)) and 
		(p_rotation == nil or type(p_rotation) == "number") and 
		(p_bgThickness == nil or type(p_bgThickness) == "number") and 
		(p_bgColor == nil or color.iscolor(p_bgColor)),
		"textRender new - expected args: (string or nil, string or nil, number or nil, number or nil, color or nil, vec2 or nil, number or nil, number or nil, color or nil)"
	)
	
	local t = {
		text = p_text or "",
		font = p_font or "arial.ttf",
		fontSize = p_fontSize or 12.0,
		spacing = p_spacing or 1.0,
		textColor = p_textColor or color(),
		offset = p_offset or vec2(),
		rotation = p_rotation or 0.0,
		bgThickness = p_bgThickness or 2.0,
		bgColor = p_bgColor or color(),
	}
	return setmetatable(t, textRender)
end

local function istextRender(t)
	return getmetatable(t) == textRender
end

-- Meta events
function textRender:__newindex(k)
	print("textRender - not possible to assign new fields")
end

function textRender:__tostring()
	return "textRender("..self.text..", "..self.font..")"
end

textRender.new = new
textRender.istextRender = istextRender
return setmetatable(textRender, {
	__call = function(_, ...) 
	return textRender.new(...) end
})