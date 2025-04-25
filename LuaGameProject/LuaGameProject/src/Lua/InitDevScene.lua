tracy.ZoneBeginN("Lua InitDevScene.lua")

local vec2 = require("Vec2")
local transform = require("Transform2")
local color = require("Color")
local sprite = require("Components/Sprite")

-- Create walls
tracy.ZoneBeginN("Lua InitDevScene Create Walls")
for _ = 1, 50 do
	local entity = scene.CreateEntity()

	local t = transform(
		vec2(math.random() + math.random(500, 1200), math.random() + math.random(-150, 550)), 
		0.0,
		vec2(math.random() + math.random(1, 100), math.random() + math.random(1, 100))
	)

	local col = color(math.random(), math.random(), math.random(), math.random())
	local s = sprite("", col)

	scene.SetComponent(entity, "Transform", t)
	scene.SetComponent(entity, "Sprite", s)
end
tracy.ZoneEnd()

-- Create enemies
tracy.ZoneBeginN("Lua InitDevScene Create Enemies")
for _ = 1, 10 do
	local entity = scene.CreateEntity()

	local t = transform(
		vec2(math.random() + math.random(500, 1200), math.random() + math.random(-150, 550)), 
		0.0,
		vec2(math.random() + math.random(55, 65), math.random() + math.random(55, 65))
	)

	--local col = color(math.random(), math.random() * 0.5, math.random() * 0.5, 1.0)
	--local s = sprite("Maxwell.png", col)
	local s = sprite("Maxwell.png")

	scene.SetComponent(entity, "Transform", t)
	scene.SetComponent(entity, "Sprite", s)
	scene.SetComponent(entity, "Behaviour", "Behaviours/Enemy")
end
tracy.ZoneEnd()

tracy.ZoneEnd()