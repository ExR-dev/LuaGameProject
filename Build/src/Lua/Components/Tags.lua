-- Tags Component, used to group entities by strings. E.g. "enemy", "wall", "chest", etc.
-- Can take an indeterminate amount of tags. To determine if a tag exists, all indexed pairs must be compared.
-- Should ideally not be used overzealously. Consider if the group could be determined by existing information.

local tags = {}
tags.__index = tags

local function new(s)
	local t = { }

	if type(s) == "string" then
		t.insert(s)
	elseif type(s) == "table" then
		for _, v in ipairs(s) do
			assert(type(v) == "string", "tags new - expected args: (string | table of strings)")
			t.insert(v)
		end
	end

	return setmetatable(t, tags)
end

local function istags(t)
	return getmetatable(t) == tags
end

-- Meta events
function tags:__newindex(s)
	assert(type(s) == "string", "tags new - expected args: (string)")
	self.insert(s)
end

function tags:__tostring()
	local tagStr = ""
	for k, v in pairs(self) do
		if type(k) == "string" then
			tagStr = tagStr .. k .. ", "
		end
	end
	return "tags("..tagStr..")"
end

function tags:has(s)
	for k, v in pairs(self) do
		if v == s then
			return true
		end
	end
	return false
end

tags.new = new
tags.istags = istags
return setmetatable(active, {
	__call = function(_, ...) 
		return active.new(...) 
	end
})