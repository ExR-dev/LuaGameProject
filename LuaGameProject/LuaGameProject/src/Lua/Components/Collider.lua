-- Collider component
--[[
	Associates a collider with an entity. Placement & size is determined by the Transform component.
	It contains tag, debug and callback.
	tag - string that can be used to categorize colliders
	debug - if true the collider will be rendered
	offset - offset the collider from the entity position
	extents - size of the collider in relation to the size of the entity (can not be retreived from getComponent)
	callback - a function that will be called when the collider intersects another collider, it's parameter is the entity it collides with.
]]

local vec2 = require("Vec2")

local collider = {}
collider.__index = collider

local function new(t, d, o, e, c)
	assert(
		(t == nil or type(t) == "string") and 
		(d == nil or type(d) == "boolean") and 
		(o == nil or vec2.isvec2(o)) and
		(e == nil or vec2.isvec2(e)) and
		(c == nil or type(c) == "function"),
		"collider new - expected args: (string or nil, bool or nil, vec2 or  nil, function or nil)"
	)
	
	local this = {
		tag = t or "",
		debug = d or false,
		offset = o or vec2(0, 0),
		extents = e or vec2(1, 1),
		callback = c or function (other) end,
	}
	return setmetatable(this, collider)
end

local function iscollider(c)
	return getmetatable(c) == collider
end

-- Meta events
function collider:__newindex(k)
	print("collider - not possible to assign new fields")
end

function collider:__tostring()
	return "collider("..self.tag..", "..self.debug..")"
end

collider.new = new
collider.iscollider = iscollider
return setmetatable(collider, {
	__call = function(_, ...) 
	return collider.new(...) end
})