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
	self.start = true
	
	-- Triggers on the first update
	-- params: self
	-- returns: nil
	self.onStartCallbacks = {}

	-- Triggers on every update
	-- params: self, delta
	-- returns: nil
	self.onUpdateCallbacks = {} 

	-- Triggers on collision with any entity
	-- params: self, hitEntity
	-- returns: doCollide
	self.onHitCallbacks = {} -- TODO: unimplemented

	-- Triggers on collision with damageable entity
	-- params: self, hitEntity, damage
	-- returns: nil
	self.onDamageCallbacks = {} -- TODO: unimplemented

	-- Triggers on collision with any entity
	-- params: self, hitEntity
	-- returns: doCollide
	self.onHitCallbacks = {} -- TODO: unimplemented

	-- Triggers on expiration
	-- params: self
	-- returns: nil
	self.onExpireCallbacks = {}

	-- Triggers on being removed
	-- params: self
	-- returns: nil
	self.onRemoveCallbacks = {} 

	tracy.ZoneEnd()
end

function projectile:OnUpdate(delta)
	tracy.ZoneBeginN("Lua projectile:OnUpdate")

	if self.start then
		self.start = false

		-- Start callbacks
		for i, func in ipairs(self.onStartCallbacks) do
			func(self)
		end
	end

	if self.expiration <= 0.0 then

		-- Expire callbacks
		for i, func in ipairs(self.onExpireCallbacks) do
			func(self)
		end

		self:OnRemove()
		tracy.ZoneEnd()
		return
	end

	self.expiration = self.expiration - delta
	
	-- Update callbacks
	for i, func in ipairs(self.onUpdateCallbacks) do
		func(self, delta)
	end

	self.trans = transform(scene.GetComponent(self.ID, "Transform"))
	self.trans:moveRelative(vec2(0, self.speed * delta))
	scene.SetComponent(self.ID, "Transform", self.trans)

	tracy.ZoneEnd()
end

function projectile:OnRemove()
	tracy.ZoneBeginN("Lua projectile:OnRemove")
	
	-- Remove callbacks
	for i, func in ipairs(self.onRemoveCallbacks) do
		func(self)
	end

	scene.RemoveEntity(self.ID)
	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return projectile
