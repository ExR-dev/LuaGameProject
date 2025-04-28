local active = {}
active.__index = active

local function new(b)
	assert(b == nil or type(b) == "boolean", "active new - expected args: (boolean or nil)")
	
	local a = {
		isActive = b or true
	}
	return setmetatable(a, active)
end

local function isactive(a)
	return getmetatable(a) == active
end

-- Meta events
function active:__newindex(k)
	print("active - not possible to assign new fields")
end

function active:__tostring()
	return "active("..self.isActive..")"
end

active.new = new
active.isactive = isactive
return setmetatable(active, {
	__call = function(_, ...) 
		return active.new(...) 
	end
})