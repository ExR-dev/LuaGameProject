-- Sprite component
--[[
	Causes an entity to be drawn as a sprite. Placement & size is determined by the Transform component.
	It contains the sprite name, color, and priority.
	spriteName - the name of the image file to be used as the sprite, relative to the texture folder. Drawn as rectangle if left undefined.
	color - the tint applied to the sprite.
	priority - used to determine the order in which sprites are drawn. Higher is drawn on top. >1000 is reserved for UI.
]]

local sprite = {}
sprite.__index = sprite

local Color = require("Color")

local function new(n, c, p)
	assert(
		(n == nil or type(n) == "string") and 
		(c == nil or Color.iscolor(c)) and 
		(p == nil or type(p) == "number"), 
		"sprite new - expected args: (string or nil, color or nil, number or nil)"
	)
	
	local s = {
		spriteName = n or "",
		color = c or Color(),
		priority = p or 0,
	}
	return setmetatable(s, sprite)
end

local function issprite(s)
	return getmetatable(s) == sprite
end

-- Meta events
function sprite:__newindex(k)
	print("sprite - not possible to assign new fields")
end

function sprite:__tostring()
	return "sprite("..self.spriteName..", "..self.color:__tostring()..", "..tostring(self.priority)..")"
end

sprite.new = new
sprite.issprite = issprite
return setmetatable(sprite, {
	__call = function(_, ...) 
	return sprite.new(...) end
})