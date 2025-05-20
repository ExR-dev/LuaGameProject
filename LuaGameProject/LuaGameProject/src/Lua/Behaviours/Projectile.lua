tracy.ZoneBeginN("Lua Projectile.lua")
local projectile = {}

local vec2 = require("Vec2")
local transform = require("Transform2")
local color = require("Color")
local gameMath = require("Utility/GameMath")
local sprite = require("Components/Sprite")
local collider = require("Components/Collider")

function projectile:OnCreate()
	tracy.ZoneBeginN("Lua projectile:OnCreate")

	-- Create projectile sprite
	local s = sprite(nil, color(0.9, 0.65, 0.25, 1.0), 10)
	scene.SetComponent(self.ID, "Sprite", s)

	self.trans = transform(scene.GetComponent(self.ID, "Transform"))

	local speedVariation = 1.0 + (0.15 * gameMath.randomND())
	self.speed = 1337.0 * 4.0 * speedVariation

	-- Make collider longer to account for high speed
	local c = collider("Projectile", false, vec2(0, 0), vec2(2.0, 0.5), 0, function(other)
		tracy.ZoneBeginN("Lua Lambda projectile:Collide")

		local doCollide = false
		local doDamage = false
		local createDamageNumber = false
		local runOnHit = false
		local remove = false

		local otherCollider = scene.GetComponent(other, "Collider")
		if (otherCollider.tag == "Enemy") then
			doCollide = true
			doDamage = true
			runOnHit = true
			createDamageNumber = true
		elseif (otherCollider.tag == "Solid") then
			doCollide = true
			doDamage = true
			runOnHit = true
			remove = true
		end

		if doCollide then
			local absorption = 1.0

			-- First apply penetration penalty if the collided entity has a hardness component
			if scene.HasComponent(other, "Hardness") then
				local h = scene.GetComponent(other, "Hardness")
				self.stats.damage = self.stats.damage * self.stats.penetration^(h.hardness)
				absorption = absorption + h.hardness
			end

			if createDamageNumber then
				local dNumberEnt = scene.CreateEntity()
				scene.SetComponent(dNumberEnt, "Behaviour", "Behaviours/DamageNumber")
				local dNumber = scene.GetComponent(dNumberEnt, "Behaviour")

				local otherT = transform(scene.GetComponent(other, "Transform"))
				local midpoint = (self.trans.position + otherT.position) * 0.5

				dNumber:Initialize(midpoint, self.stats.damage)
			end

			-- If the other entity has a health component, apply damage and spawn a damage number
			if doDamage then
				if scene.HasComponent(other, "Health") then
					local h = scene.GetComponent(other, "Health")
					h.current = h.current - self.stats.damage
					scene.SetComponent(other, "Health", h)
			
					createDamageNumber = true
					runOnHit = true

					-- Very large damage reduction after doing damage
					local damageReduction = (self.stats.penetration^(absorption)) * 0.95
					damageReduction = damageReduction * damageReduction
					self.stats.damage = self.stats.damage * damageReduction
				end
			end

			if runOnHit then
				if scene.HasComponent(other, "Behaviour") then
					local otherBeh = scene.GetComponent(other, "Behaviour")

					if otherBeh.OnHit ~= nil then
						otherBeh:OnHit()
					end
				end
			end

			if remove then
				self:OnRemove()
			end
		end

		tracy.ZoneEnd()
	end)

	scene.SetComponent(self.ID, "Collider", c)

	self.expiration = 0.5 -- seconds
	self.decayThreshold = nil
	self.stats = nil

	tracy.ZoneEnd()
end

function projectile:Initialize(weaponStats, ammoStats)
	tracy.ZoneBeginN("Lua projectile:Initialize")	

	self.stats = {
		damage = weaponStats.damage * ammoStats.damageMult,
		falloff = ammoStats.falloff,
		penetration = ammoStats.penetration,
	}

	self.decayThreshold = self.stats.damage * 0.25

	tracy.ZoneEnd()
end

function projectile:OnUpdate(delta)
	tracy.ZoneBeginN("Lua projectile:OnUpdate")

	-- Expire projectile if too old
	if self.expiration <= 0.0 then
		self:OnRemove()
		tracy.ZoneEnd()
		return
	end
	self.expiration = self.expiration - delta

	-- Random chance to decay the projectile if it's lost too much damage
	if self.stats.damage < self.decayThreshold then
		if self.stats.damage < (math.random() * self.decayThreshold) then
			self:OnRemove()
			tracy.ZoneEnd()
			return
		end
	end

	self.trans = transform(scene.GetComponent(self.ID, "Transform"))

	if self.travelDir == nil then
		self.travelDir = self.trans:getForward()
	end

	local stepLength = self.speed * delta

	self.trans.position = self.trans.position + (self.travelDir * stepLength)
	scene.SetComponent(self.ID, "Transform", self.trans)

	-- Modify stats over distance travelled
	local stepInMeters = stepLength * gameMath.pixelsToMeters
	self.stats.damage = self.stats.damage * self.stats.falloff^(stepInMeters)

	tracy.ZoneEnd()
end

function projectile:OnRemove()
	tracy.ZoneBeginN("Lua projectile:OnRemove")
	scene.RemoveEntity(self.ID)
	tracy.ZoneEnd()
end

function projectile:OnDamage(target)
	tracy.ZoneBeginN("Lua projectile:OnDamage")


	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return projectile
