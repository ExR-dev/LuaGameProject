
tracy.ZoneBeginN("Lua Cursor.lua")
local cursor = {}

local vec2 = require("Vec2")
local transform = require("Transform2")
local gameMath = require("Utility/GameMath")

function cursor:OnCreate()
	tracy.ZoneBeginN("Lua cursor:OnCreate")

    self.transform = transform(scene.GetComponent(self.ID, "Transform"))
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
	self.transform.scale = vec2(newSize, newSize)

    self.transform.position = playerCam.camT.position + cursorPos;
	scene.SetComponent(self.ID, "Transform", self.transform)

	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return cursor