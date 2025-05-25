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

	self.shakeCoroutines = { }

	tracy.ZoneEnd()
end

function playerCamera:OnUpdate(delta)
	tracy.ZoneBeginN("Lua playerCamera:OnUpdate")
	
	if self.trackedEntity < 0 then
		if game.GetPlayer then
			self.trackedEntity = game.GetPlayer().ID
		else
			-- No entity to track
			tracy.ZoneEnd()
			return
		end
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

		camData.zoom = gameMath.clamp(camData.zoom, 0.4, 3.0);

		scene.SetComponent(self.ID, "CameraData", camData)
	end

	-- Update camera shake coroutines
	for i, co in pairs(self.shakeCoroutines) do
		local ret, err = pcall(coroutine.resume, co, delta)

		if not ret then
			print("Error: ", err)
			self.shakeCoroutines[i] = nil
		end

		if coroutine.status(co) == "dead" then
			self.shakeCoroutines[i] = nil
		end
	end

	scene.SetComponent(self.ID, "Transform", self.camT)
	tracy.ZoneEnd()
end

function playerCamera:SetTrackedEntity(ent)
	self.trackedEntity = ent
end

function playerCamera:ApplyShake(frequency, amplitude, duration)
	local shakeCo = coroutine.create(function(self, frequency, amplitude, duration)
		local waveLength = 1.0 / frequency
		local timeLeft = duration
		local invDuration = 1.0 / duration
		local nextWaveTimer = 0.0

		local delta = 0.0
		while timeLeft >= 0 do
			local timeLeftNormalized = timeLeft * invDuration

			while nextWaveTimer <= 0 do
				-- Generate & apply random shake offset
				local shakeOffset = vec2(
					gameMath.randomND() * amplitude * timeLeftNormalized,
					gameMath.randomND() * amplitude * timeLeftNormalized
				)

				self.camT.position = self.camT.position + shakeOffset
				nextWaveTimer = nextWaveTimer + waveLength
			end

			nextWaveTimer = nextWaveTimer - delta
			timeLeft = timeLeft - delta
			delta = coroutine.yield()
		end
	end)

	-- Start the coroutine with the given parameters
	coroutine.resume(shakeCo, self, frequency, amplitude, duration)

	table.insert(self.shakeCoroutines, shakeCo)
end

tracy.ZoneEnd()
return playerCamera