-- Collider component
--[[
	Associates a collider with an entity. Placement & size is determined by the Transform component.
	It contains tag, debug and callback.
	tag - string that can be used to categorize colliders
	debug - if true the collider will be rendered
	offset - offset the collider from the entity position
	extents - size of the collider in relation to the size of the entity (can not be retreived from getComponent)
	rotation - rotation of the collider relative the rotation of the entity
	callback - a function that will be called when the collider intersects another collider, it's parameter is the entity it collides with.
]]

local vec2 = require("Vec2")

local collider = {}
collider.__index = collider

local function new(t, d, o, e, r, enter, exit)

	if type(t) == "table" then
		return setmetatable(t, collider)
	end

	assert(
		(t == nil or type(t) == "string") and 
		(d == nil or type(d) == "boolean") and 
		(o == nil or vec2.isvec2(o)) and
		(e == nil or vec2.isvec2(e)) and
		(r == nil or type(r) == "number"),
		(enter == nil or type(enter) == "function"),
		(exit == nil or type(exit) == "function"),
		"collider new - expected args: (string or nil, bool or nil, vec2 or nil, numver or nil, function or nil, function or nil)"
	)
	
	local this = {
		tag = t or "",
		debug = d or false,
		offset = o or vec2(0, 0),
		extents = e or vec2(1, 1),
		rotation = r or 0,
		onEnterCallback = enter or function (other) end,
		onExitCallback = exit or function (other) end,
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