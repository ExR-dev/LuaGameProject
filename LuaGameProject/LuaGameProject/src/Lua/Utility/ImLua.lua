tracy.ZoneBeginN("Lua Run PrintEntity.lua")

if imgui == nil then
	imgui = { }
end

imgui.imVec2 = function(x, y)
	return {
		x = x,
		y = y
	}
end

imgui.imVec4 = function(x, y, z, w)
	return {
		x = x,
		y = y,
		z = z,
		w = w
	}
end


tracy.ZoneEnd()