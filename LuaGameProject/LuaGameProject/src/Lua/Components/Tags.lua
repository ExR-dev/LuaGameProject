-- Tags Component, used to group entities by strings. E.g. "enemy", "wall", "chest", etc.
-- Can take an indeterminate amount of tags. To determine if a tag exists, all indexed pairs must be compared.
-- Should ideally not be used overzealously. Consider if the group could be determined by existing information.

local tags = {}
tags.__index = tags

local function new(s)
	local t = { }

	if type(s) == "string" then
		t.insert(s)
	end

	return setmetatable(t, tags)
end

local function istags(t)
	return getmetatable(t) == tags
end

-- Meta events
function tags:__newindex(k)
	assert(type(s) == "string", "tags new - expected args: (string)")
	print("tags - not possible to assign new fields")
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