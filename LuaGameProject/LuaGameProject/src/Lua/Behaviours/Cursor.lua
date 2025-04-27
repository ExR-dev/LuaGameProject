tracy.ZoneBeginN("Lua Cursor.lua")
local cursor = {}

local vec2 = require("Vec2")
local transform = require("Transform2")
local gameMath = require("Utility/GameMath")

-- Global cursor getter
function GetCursor()
	return cursor
end

function cursor:OnCreate()
	tracy.ZoneBeginN("Lua cursor:OnCreate")
	
	-- TODO: Caching the transform means that any changes on the C++-side will be overwritten.
	-- To be able to both cache transforms and modify them in C++, we would have to store them as lua references.
    self.trans = transform(scene.GetComponent(self.ID, "Transform"))
	self.baseSize = 48.0 -- Multiple of the cursor texture dimensions

	tracy.ZoneEnd()
end

function cursor:OnRender(delta)
	tracy.ZoneBeginN("Lua cursor:OnRender")

	if GetPlayerCamera == nil then
		tracy.ZoneEnd()
		return
	end

	local playerCam = GetPlayerCamera()

	if playerCam == nil then
		tracy.ZoneEnd()
		return
	end

	local camData = scene.GetComponent(playerCam.ID, "CameraData")
	local invZoom = 1.0 / camData.zoom

	local cursorPos = vec2(Input.GetMouseInfo().Position) - vec2(Window.width * 0.5, Window.height * 0.5)
	cursorPos = cursorPos * invZoom

	local newSize = self.baseSize * invZoom
	self.trans.scale = vec2(newSize, newSize)

    self.trans.position = playerCam.camT.position + cursorPos;
	scene.SetComponent(self.ID, "Transform", self.trans)

	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return cursor