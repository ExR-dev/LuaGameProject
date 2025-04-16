local transform = {}

local vector = require("Vector")

local function new(p, r, s)
	assert(p == nil or vector.isvector(p), "transform new - expected args: vector or nil")

	local t = {
		position = p or vector(),
		rotation = r or vector(),
		scale = s or vector()
	}
	return setmetatable(t, transform)
end