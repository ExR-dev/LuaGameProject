tracy.ZoneBeginN("Lua Projectile.lua")
local projectile = {}

local vec2 = require("Vec2")
local transform = require("Transform2")
local color = require("Color")
local gameMath = require("Utility/GameMath")
local sprite = require("Components/Sprite")

function projectile:OnCreate()
	tracy.ZoneBeginN("Lua projectile:OnCreate")

	-- Create projectile sprite
	local s = sprite(nil, color(0.9, 0.65, 0.25, 1.0), 10)
	scene.SetComponent(self.ID, "Sprite", s)

	self.trans = transform(scene.GetComponent(self.ID, "Transform"))
	self.speed = 1337.0 * 1.5

	scene.SetComponent(self.ID, "Collider", "Projectile", function(other) 
		tracy.ZoneBeginN("Lua Lambda collider:Projectile")

		local o = scene.GetComponent(other, "Collider")
		print("Colliding " .. o.tag)
		if (o.tag == "Enemy") then
			scene.RemoveEntity(other)
			scene.RemoveEntity(self.ID)
		end

		tracy.ZoneEnd()
	end)

	self.expiration = 5.0

	self.stats = nil -- Set after spawning projectile using data.ammo.getStats()

	tracy.ZoneEnd()
end

function projectile:OnUpdate(delta)
	tracy.ZoneBeginN("Lua projectile:OnUpdate")
	if self.expiration <= 0.0 then
		self:OnRemove()
		tracy.ZoneEnd()
		return
	end

	self.expiration = self.expiration - delta

	self.trans = transform(scene.GetComponent(self.ID, "Transform"))
	self.trans:moveRelative(vec2(0, self.speed * delta))
	scene.SetComponent(self.ID, "Transform", self.trans)

	tracy.ZoneEnd()
end

function projectile:OnRemove()
	tracy.ZoneBeginN("Lua projectile:OnRemove")
	scene.RemoveEntity(self.ID)
	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return projectile
