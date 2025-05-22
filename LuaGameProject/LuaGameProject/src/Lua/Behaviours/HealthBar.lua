tracy.ZoneBeginN("Lua HealthBar.lua")
local healthBar = {}

local vec2 = require("Vec2")
local transform = require("Transform2")
local color = require("Color")
local textRender = require("Components/TextRender")
local gameMath = require("Utility/GameMath")
local health = require("Components/Health")


function healthBar:OnCreate()
	tracy.ZoneBeginN("Lua healthBar:OnCreate")
	self.speed = 16
	tracy.ZoneEnd()
end

function healthBar:Initialize(target, offset)
	tracy.ZoneBeginN("Lua healthBar:Initialize")
	
	offset = offset or 100

	self.target = target
	local targetT = transform(scene.GetComponent(target, "Transform"))

	self.trans = transform()
	self.trans.position = targetT.position

	self.textRender = textRender(
		"", -- Text
		"", -- Font
		14.0, -- Size
		3.0, -- Spacing
		color(1, 1, 1, 1), -- Text color
		vec2(0, -offset),
		nil,
		2.0,
		color(0.0, 0.0, 0.0, 1.0) -- Background color
	)

	scene.SetComponent(self.ID, "Transform", self.trans)
	scene.SetComponent(self.ID, "TextRender", self.textRender)

	tracy.ZoneEnd()
end

function healthBar:OnUpdate(delta)
	tracy.ZoneBeginN("Lua healthBar:OnUpdate")

	if self.target == nil then
		tracy.ZoneEnd()
		return
	end

	if not scene.IsEntity(self.target) then
		scene.RemoveEntity(self.ID)
		tracy.ZoneEnd()
		return
	end

	local targetT = transform(scene.GetComponent(self.target, "Transform"))
	self.trans.position.x = gameMath.expDecay(self.trans.position.x, targetT.position.x, self.speed, delta)
	self.trans.position.y = gameMath.expDecay(self.trans.position.y, targetT.position.y, self.speed, delta)
	scene.SetComponent(self.ID, "Transform", self.trans)

	local targetH = scene.GetComponent(self.target, "Health")
	if targetH then
		self.textRender.text = tostring(math.floor(targetH.current)).." / "..tostring(math.floor(targetH.max))
		scene.SetComponent(self.ID, "TextRender", self.textRender)
	end

	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return healthBar