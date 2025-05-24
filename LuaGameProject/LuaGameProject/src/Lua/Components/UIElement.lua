local uiElement = {}
uiElement.__index = uiElement

local function new()
	local u = { 
		_ = 0
	}

	return setmetatable(u, uiElement)
end

local function isUIElement(u)
	return getmetatable(u) == uiElement
end

-- Meta events
function uiElement:__newindex(k)
	print("uiElement - not possible to assign new fields")
end

function uiElement:__tostring()
	return "uiElement()"
end

uiElement.new = new
uiElement.isUIElement = isUIElement
return setmetatable(uiElement, {
	__call = function(_, ...) 
		return uiElement.new(...) 
	end
})