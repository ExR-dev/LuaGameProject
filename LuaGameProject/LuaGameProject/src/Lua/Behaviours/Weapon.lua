tracy.ZoneBeginN("Lua Weapon.lua")
local weapon = {}

local vec2 = require("Vec2")
local transform = require("Transform2")
local gameMath = require("Utility/GameMath")

function weapon:OnCreate()
	tracy.ZoneBeginN("Lua weapon:OnCreate")
	
	self.trans = transform(scene.GetComponent(self.ID, "Transform"))

	-- Heavily overengineered for now, simplify before implementing
	self.stats = { -- Baseline stats. Could be affected by condition, bullet velocity, ammo type, etc.
		handCount = 1,
		caliber = "9mm", -- Ex: 9mm, 12ga, 5.56, 308,
		fireMode = "Semi", -- Semi, Auto
		damage = 10.0,
		fireRate = 4.0,
		spread = 2.0,
		recoil = 5.0, -- Added spread by firing, decaying over time
		recovery = 8.0, -- Decay of recoil, see gameMath.expDecay().
		capacity = 12
	}

	self.ammoStats = { -- Stats for the currently loaded ammo
		caliber = "9mm", -- For matching ammo to the weapon
		ammoType = "FMJ", -- Ex: FMJ, HP, AP
		damage = 5.0,
		velocity = 0.0,
		range = 0.0,
		penetration = 0.0,
		spread = 3.0,
		recoil = 0.0,
		burstSize = 1 -- For weapons that fire multiple rounds at once
	}

	self.loadedAmmoCount = 0
	self.loadedAmmoType = ""

	self.fireCooldown = 0.0
	self.currRecoil = 0.0

	self.isHeld = false

	tracy.ZoneEnd()
end

function weapon:OnUpdate(delta)
	tracy.ZoneBeginN("Lua weapon:OnUpdate")
	
	if self.fireCooldown > 0.0 then
		self.fireCooldown = self.fireCooldown - delta
	end

	self.currRecoil = gameMath.expDecay(self.currRecoil, 0.0, self.stats.recovery, delta)

	tracy.ZoneEnd()
end

function weapon:OnShoot()
	tracy.ZoneBeginN("Lua weapon:OnShoot")

	if self.fireCooldown > 0.0 then
		tracy.ZoneEnd()
		return
	end

	if self.loadedAmmoCount <= 0 then
		tracy.ZoneEnd()
		return
	end

	local origin = self.trans.position
	local dir = GetCursor().trans.position - origin

	local angle = dir:angle()
	local totalSpread = self.stats.spread + self.ammoStats.spread + self.currRecoil

	for i = 1, self.ammoStats.burstSize do
		local offsetAngle = angle + (gameMath.randomND() * totalSpread)

		local projEnt = scene.CreateEntity()
		local projT = transform(origin, offsetAngle, vec2(24, 6))

		scene.SetComponent(projEnt, "Transform", projT)
		scene.SetComponent(projEnt, "Behaviour", "Behaviours/Projectile")
	end

	self.fireCooldown = self.fireCooldown + (1.0 / self.stats.fireRate)
	self.currRecoil = self.currRecoil + self.stats.recoil + self.ammoStats.recoil
	self.loadedAmmoCount = self.loadedAmmoCount - 1

	tracy.ZoneEnd()
end

-- Returns the amount of ammo used to load the weapon
function weapon:OnReload()
	local ammoLoaded = self.stats.capacity - self.loadedAmmoCount
	self.loadedAmmoCount = self.stats.capacity
	return ammoLoaded
end

tracy.ZoneEnd()
return weapon