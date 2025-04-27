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
	self.speed = 1337
	self.expiration = 5.0

	tracy.ZoneEnd()
end

function projectile:OnUpdate(delta)
	tracy.ZoneBeginN("Lua projectile:OnUpdate")

	if self.expiration <= 0.0 then
		scene.RemoveEntity(self.ID)
		tracy.ZoneEnd()
		return
	end
	self.expiration = self.expiration - delta

	self.trans = transform(scene.GetComponent(self.ID, "Transform"))
	self.trans:moveRelative(vec2(0, self.speed * delta))
	scene.SetComponent(self.ID, "Transform", self.trans)

	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return projectile