tracy.ZoneBeginN("Lua Weapon.lua")
local weapon = {}

local vec2 = require("Vec2")
local transform = require("Transform2")
local color = require("Color")
local gameMath = require("Utility/GameMath")
local sprite = require("Components/Sprite")

function weapon:OnCreate()
	tracy.ZoneBeginN("Lua weapon:OnCreate")
	
	self.trans = transform(scene.GetComponent(self.ID, "Transform"))

	self.stats = nil

	self.loadedAmmoCount = 0
	self.loadedAmmoType = nil

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
				print("Done Reloading.")
			end
		end

		if self.fireCooldown > 0.0 then
			self.fireCooldown = self.fireCooldown - delta
			
			if self.fireCooldown <= 0.0 then
				if self.stats.fireMode == "Auto" then
					if Input.KeyHeld(Input.Key.KEY_SPACE) then
						self:OnShoot()
					end
				else
					self.fireCooldown = 0.0
				end
			end
		end

		self.currRecoil = gameMath.expDecay(self.currRecoil, 0.0, self.stats.recovery, delta)
	else
		-- self.trans = transform(scene.GetComponent(self.ID, "Transform"))

		-- TODO: Idk, spin or something
	end

	tracy.ZoneEnd()
end

function weapon:OnShoot()
	tracy.ZoneBeginN("Lua weapon:OnShoot")

	-- Check if the weapon is on cooldown
	if self.fireCooldown > 0.0 then
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
	local dir = game.GetCursor().trans.position - origin

	local angle = dir:angle()
	angle = angle + (gameMath.randomSigned() * self.currRecoil)

	local totalSpread = self.stats.spread + ammoStats.spread

	for i = 1, ammoStats.burstSize do
		local offsetAngle = angle + (gameMath.randomND() * totalSpread)

		local projEnt = scene.CreateEntity()
		local projT = transform(origin, offsetAngle, vec2(24, 6))

		scene.SetComponent(projEnt, "Transform", projT)
		scene.SetComponent(projEnt, "Behaviour", "Behaviours/Projectile")

		-- Get the projectile behaviour to set its stats
		local projBehaviour = scene.GetComponent(projEnt, "Behaviour")
		projBehaviour.stats = ammoStats
	end

	self.fireCooldown = self.fireCooldown + (1.0 / self.stats.fireRate)
	self.currRecoil = self.currRecoil + self.stats.recoil + ammoStats.recoil
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

	print("Reloading "..self.loadedAmmoType.." ammo: "..ammoTaken.." / "..ammoInReserve)
	print("Reserve now: "..caliberReserve[self.loadedAmmoType])

	return true
end

function weapon:TryPickUp()
	if self.isHeld then
		return false
	end

	local player = game.GetPlayer()

	if self.stats.handCount == 2 then

		if player.rHandEntity ~= nil then
			return false
		end

		if player.lHandEntity ~= nil then
			return false
		end
		
		player.rHandEntity = self.ID
		player.lHandEntity = self.ID

		self.isHeld = true
	else
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

tracy.ZoneEnd()
return weapon