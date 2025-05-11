tracy.ZoneBeginN("Lua PlayerCamera.lua")
local playerCamera = {}

local vec2 = require("Vec2")
local transform = require("Transform2")
local gameMath = require("Utility/GameMath")

-- Global player camera getter
local function GetPlayerCamera()
	return playerCamera
end
game.GetPlayerCamera = GetPlayerCamera

function playerCamera:OnCreate()
	tracy.ZoneBeginN("Lua playerCamera:OnCreate")
	
	self.trackedEntity = -1
	self.trackingStrength = 10 -- How fast the camera corrects its position
	self.trackingOffset = vec2(0.0, 0.0) -- For aiming, camera shake, etc.
	self.camT = transform(scene.GetComponent(self.ID, "Transform"))

	tracy.ZoneEnd()
end

function playerCamera:OnUpdate(delta)
	tracy.ZoneBeginN("Lua playerCamera:OnUpdate")

	if self.trackedEntity < 0 then
		-- No entity to track
		tracy.ZoneEnd()
		return
	end

	local entT = transform(scene.GetComponent(self.trackedEntity, "Transform"))

	if entT == nil then
		-- Tracked entity is not valid
		tracy.ZoneEnd()
		return
	end

	self.camT = transform(scene.GetComponent(self.ID, "Transform"))
	
	-- Interpolate camera position towards tracked entity + offset
	self.camT.position = gameMath.expDecay(
		self.camT.position,
		(entT.position + self.trackingOffset),
		self.trackingStrength,
		delta
	)

	-- Update zoom if is scrolling
	local mouseInfo = Input.GetMouseInfo()
	if not gameMath.approx(mouseInfo.Scroll, 0.0) then
		local camData = scene.GetComponent(self.ID, "CameraData")

		if mouseInfo.Scroll > 0.0 then
			camData.zoom = camData.zoom * (1.0 + 0.1 * mouseInfo.Scroll)
		else
			camData.zoom = camData.zoom / (1.0 - 0.1 * mouseInfo.Scroll)
		end

		scene.SetComponent(self.ID, "CameraData", camData)
	end

	scene.SetComponent(self.ID, "Transform", self.camT)
	tracy.ZoneEnd()
end

function playerCamera:SetTrackedEntity(ent)
	self.trackedEntity = ent
end

tracy.ZoneEnd()
return playerCamera