tracy.ZoneBeginN("Lua DamageNumber.lua")
local damageNumber = {}

local vec2 = require("Vec2")
local transform = require("Transform2")
local color = require("Color")
local textRender = require("Components/TextRender")
local gameMath = require("Utility/GameMath")


function damageNumber:OnCreate()
	tracy.ZoneBeginN("Lua damageNumber:OnCreate")

	self.lifetime = 1.0
	self.velocity = vec2(gameMath.randomND() * 80.0, -80.0)
	self.fadeoutTime = 0.3
	self.bgAlpha = 0.2

	tracy.ZoneEnd()
end

function damageNumber:Initialize(pos, damage)
	tracy.ZoneBeginN("Lua damageNumber:Initialize")	
	
	self.trans =  transform()
	self.trans.position = pos

	local textCol = color(0.9, 0.6, 0.1, 1.0)

	self.textRender = textRender(
		tostring(math.floor(damage * 10.0) * 0.1), -- Text
		"", -- Font
		18.0, -- Size
		4.0, -- Spacing
		textCol, -- Text color
		nil,
		nil,
		3.0,
		color(0.0, 0.0, 0.0, self.bgAlpha) -- Background color
	)

	scene.SetComponent(self.ID, "Transform", self.trans)
	scene.SetComponent(self.ID, "TextRender", self.textRender)

	tracy.ZoneEnd()
end

function damageNumber:OnUpdate(delta)
	tracy.ZoneBeginN("Lua damageNumber:OnUpdate")

	if self.trans == nil then
		tracy.ZoneEnd()
		return
	end

	if self.lifetime <= 0.0 then
		scene.RemoveEntity(self.ID)
		tracy.ZoneEnd()
		return
	end
	
	if self.lifetime <= self.fadeoutTime then
		local fade = self.lifetime / self.fadeoutTime
		self.textRender.textColor.a = fade
		self.textRender.bgColor.a = self.bgAlpha * fade

		scene.SetComponent(self.ID, "TextRender", self.textRender)
	end

	self.lifetime = self.lifetime - delta

	self.trans.position = self.trans.position + (self.velocity * delta)

	-- decay velocity
	self.velocity.x = gameMath.expDecay(self.velocity.x, 0.0, 1, delta)
	self.velocity.y = gameMath.expDecay(self.velocity.y, 0.0, 2, delta)

	scene.SetComponent(self.ID, "Transform", self.trans)
	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return damageNumber