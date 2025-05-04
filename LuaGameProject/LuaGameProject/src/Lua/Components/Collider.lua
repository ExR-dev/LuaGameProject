-- Collider component
--[[
	Associates a collider with an entity. Placement & size is determined by the Transform component.
	It contains tag, debug and callback.
	tag - string that can be used to categorize colliders
	debug - if true the collider will be rendered
	callback - a function that will be called when the collider intersects another collider, it's parameter is the entity it collides with.
]]

local collider = {}
collider.__index = collider

local function new(t, d, c)
	assert(
		(t == nil or type(t) == "string") and 
		(d == nil or type(d) == "boolean") and 
		(c == nil or type(c) == "function"),
		"collider new - expected args: (string or nil, bool or nil, function or nil)"
	)
	
	local this = {
		tag = t or "",
		debug = d or false,
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