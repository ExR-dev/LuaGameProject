local cameraData = {}
cameraData.__index = cameraData

local vec2 = require("Vec2")

local function new(s, z)
	assert((s == nil or vec2.isvec2(s)) and (z == nil or type(z) == "number"), "cameraData new - expected args: (vec2 or nil, number or nil)")
	
	local c = {
		size = s or vec2(1280, 720),
		zoom = z or 1.0
	}
	return setmetatable(c, cameraData)
end

local function iscamera(c)
	return getmetatable(c) == cameraData
end

-- Meta events
function cameraData:__newindex(k)
	print("cameraData - not possible to assign new fields")
end

function cameraData:__tostring()
	return "cameraData(size: "..self.size..")"
end

cameraData.new = new
cameraData.iscamera = iscamera
return setmetatable(cameraData, {
	__call = function(_, ...) 
	return cameraData.new(...) end
})