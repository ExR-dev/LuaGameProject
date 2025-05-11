-- TODO: Incomplete. Could have a table of callback functions, 
-- but it would need to be storable in a C++ struct in order 
-- to not get reset after calling LuaPush()

local hitboxCircle = {}
hitboxCircle.__index = hitboxCircle

local vec2 = require("Vec2")

local function new(o, r)
	assert(vec2.isvec2(o) and type(r) == "number", "hitboxCircle new - expected args: (vec2, number)")
	
	local h = {
		origin = o,
		radius = r
	}
	return setmetatable(h, hitboxCircle)
end

local function ishitboxCircle(o)
	return getmetatable(o) == hitboxCircle
end

-- Meta events
function hitboxCircle:__newindex(k)
	print("hitboxCircle - not possible to assign new fields")
end

function hitboxCircle:__tostring()
	return "hitboxCircle("..self.origin:__tostring()..", "..tostring(self.radius)..")"
end

sprite.new = new
sprite.issprite = issprite
return setmetatable(sprite, {
	__call = function(_, ...) 
	return sprite.new(...) end
})