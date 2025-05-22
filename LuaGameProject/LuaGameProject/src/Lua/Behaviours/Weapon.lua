tracy.ZoneBeginN("Lua Weapon.lua")
local weapon = {}

local gameMath = require("Utility/GameMath")
local vec2 = require("Vec2")
local transform = require("Transform2")
local color = require("Color")
local sprite = require("Components/Sprite")
local collider = require("Components/Collider")

function weapon:OnCreate()
	tracy.ZoneBeginN("Lua weapon:OnCreate")
	
	self.trans = transform(scene.GetComponent(self.ID, "Transform"))

	self.stats = nil

	self.loadedAmmoCount = 0
	self.loadedAmmoType = nil

	self.isOnCooldown = false
	self.fireCooldown = 0.0
	self.currRecoil = 0.0

	self.isHeld = false

	self.isReloading = false
	self.reloadTimer = 0.0

	tracy.ZoneEnd()
end

function weapon:LoadType(type)
	tracy.ZoneBeginN("Lua weapon:LoadType")

	-- Get the type from data.weapons
	local weaponData = data.weapons[type]

	if weaponData == nil then
		print("Couldn't find weapon: "..type..". Defaulting to Glock...")
		weaponData = data.weapons["Glock"]
	end
	
	-- Set the sprite
	local weaponSprite = sprite()
	weaponSprite.priority = 21
	weaponSprite.spriteName = weaponData.sprite

	if weaponSprite.spriteName == nil then
		weaponSprite.color = color(0.2, 0.175, 0.1, 1.0)
	end

	scene.SetComponent(self.ID, "Sprite", weaponSprite)

	-- Set the size
	self.trans.scale.x = weaponData.length
	self.trans.scale.y = weaponData.width

	scene.SetComponent(self.ID, "Transform", self.trans)
	
	-- Create weapon collider
	local c = collider("Weapon", false, vec2(0, 0), vec2(1.0, 1.0), 0, true, nil, nil)

	scene.SetComponent(self.ID, "Collider", c)

	-- Set the stats
	self.stats = weaponData.stats
	self.loadedAmmoType = data.ammo.getCaliberDefaultType(self.stats.caliber)

	self.loadedAmmoCount = weaponData.stats.capacity

	tracy.ZoneEnd()
end

function weapon:OnUpdate(delta)
	tracy.ZoneBeginN("Lua weapon:OnUpdate")
	
	if self.isHeld then

		if self.isReloading then
			self.reloadTimer = self.reloadTimer - delta

			if self.reloadTimer <= 0.0 then
				self.reloadTimer = 0.0
				self.isReloading = false
				print("Done Reloading.\n")
			end
		end

		if self.isOnCooldown then
			self.fireCooldown = self.fireCooldown - delta
			
			if self.fireCooldown <= 0.0 then
				self.isOnCooldown = false

				if self.stats.fireMode == "Auto" then
					if Input.MouseHeld(Input.Mouse.MOUSE_LEFT) then
						self:OnShoot()
					end
				else
					self.fireCooldown = 0.0
				end
			end
		end

		self.currRecoil = gameMath.expDecay(self.currRecoil, 0.0, self.stats.recovery, delta)
	else
		self.trans = transform(scene.GetComponent(self.ID, "Transform"))

		-- TODO: Idk, spin or something

		self.trans.rotation = self.trans.rotation + (delta * 20.0)
		scene.SetComponent(self.ID, "Transform", self.trans)
	end

	tracy.ZoneEnd()
end

-- Called during ImGui rendering if the entity is selected
function weapon:OnGUI()
	tracy.ZoneBeginN("Lua weapon:OnGUI")

	local strInput = self.presetInputStr or ""
	local modified = false
	strInput, modified = imgui.InputText("Stat Preset", strInput)

	if modified then
		self.presetInputStr = strInput
	end

	if imgui.Button("Load Preset") then
		self:LoadType(self.presetInputStr)
	end

	tracy.ZoneEnd()
end

function weapon:OnShoot()
	tracy.ZoneBeginN("Lua weapon:OnShoot")

	-- Check if the weapon is on cooldown
	if self.isOnCooldown then
		tracy.ZoneEnd()
		return
	end

	-- Check if the weapon is loaded
	if self.loadedAmmoCount <= 0 then
		tracy.ZoneEnd()
		return
	end

	-- Check if the weapon is reloading
	if self.isReloading then
		tracy.ZoneEnd()
		return
	end

	-- Get the ammo stats
	local ammoStats = data.ammo.getStats(self.stats.caliber, self.loadedAmmoType)

	if ammoStats == nil then
		print("Invalid ammo type: "..self.stats.caliber..", "..self.loadedAmmoType)
		tracy.ZoneEnd()
		return
	end

	local origin = self.trans.position
	local angle = self.trans.rotation

	local totalSpread = self.stats.spread + ammoStats.spread

	for i = 1, ammoStats.burstSize do
		local offsetAngle = angle + (gameMath.randomND() * totalSpread)

		local projEnt = scene.CreateEntity()
		local projT = transform(origin, offsetAngle, vec2(24, 6))

		scene.SetComponent(projEnt, "Transform", projT)
		scene.SetComponent(projEnt, "Behaviour", "Behaviours/Projectile")

		-- Get the projectile behaviour to set its stats
		local projBehaviour = scene.GetComponent(projEnt, "Behaviour")
		projBehaviour:Initialize(self.stats, ammoStats)
	end

	self.isOnCooldown = true
	self.fireCooldown = self.fireCooldown + (1.0 / self.stats.fireRate)

	local recoilStrength = self.stats.recoil + ammoStats.recoil
	local newRecoil = gameMath.randomSigned() * recoilStrength
	newRecoil = newRecoil + (0.1 * gameMath.clamp(self.currRecoil, -recoilStrength, recoilStrength))
	self.currRecoil = self.currRecoil + newRecoil

	self.loadedAmmoCount = self.loadedAmmoCount - 1

	tracy.ZoneEnd()
end

function weapon:OnReload(reserve)
	if self.isReloading then
		return false
	end

	local ammoNeeded = self.stats.capacity - self.loadedAmmoCount

	if ammoNeeded <= 0 then
		return false
	end

	local caliberReserve = reserve[self.stats.caliber]
	if caliberReserve == nil then
		return false
	end

	local ammoInReserve = caliberReserve[self.loadedAmmoType]
	if ammoInReserve == nil then
		return false
	end

	if ammoInReserve <= 0 then
		return false
	end

	local ammoTaken = math.min(ammoInReserve, ammoNeeded)
	self.loadedAmmoCount = self.loadedAmmoCount + ammoTaken
	caliberReserve[self.loadedAmmoType] = ammoInReserve - ammoTaken
	
	self.isReloading = true
	self.reloadTimer = self.stats.reloadTime

	print("Reloading "..self.stats.caliber.." ("..self.loadedAmmoType..")")
	print("Taking: "..ammoTaken.." out of "..ammoInReserve)
	print("Reserve now: "..caliberReserve[self.loadedAmmoType].."\n")

	return true
end

function weapon:Interact()
	if self.isHeld then
		return false
	end

	local player = game.GetPlayer()

	if self.stats.handCount == 2 then

		if player.rHandEntity ~= nil or player.lHandEntity ~= nil then
			player:DropItems(2)
		end
		
		player.rHandEntity = self.ID
		player.lHandEntity = self.ID

		self.isHeld = true
	else
		if player.rHandEntity ~= nil and player.lHandEntity ~= nil then
			player:DropItems(1)
		end

		if player.rHandEntity == nil then
			player.rHandEntity = self.ID
			self.isHeld = true
		elseif player.lHandEntity == nil then
			player.lHandEntity = self.ID
			self.isHeld = true
		end
	end

	return self.isHeld
end

function weapon:Drop()
	if not self.isHeld then
		return
	end

	local player = game.GetPlayer()

	if player.rHandEntity == self.ID then
		player.rHandEntity = nil
	elseif player.lHandEntity == self.ID then
		player.lHandEntity = nil
	end

	self.isHeld = false
end

tracy.ZoneEnd()
return weapon