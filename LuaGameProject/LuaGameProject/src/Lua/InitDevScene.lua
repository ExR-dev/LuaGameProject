tracy.ZoneBeginN("Lua InitDevScene.lua")

local vec2 = require("Vec2")
local transform = require("Transform2")
local color = require("Color")
local sprite = require("Components/Sprite")

tracy.ZoneBeginN("Lua InitDevScene Create Walls")
-- Create walls
for _ = 1, 1000 do
	local entity = scene.CreateEntity()

	local t = transform(
		vec2(math.random() + math.random(500, 1200), math.random() + math.random(-150, 550)), 
		0.0,
		vec2(math.random() + math.random(1, 100), math.random() + math.random(1, 100))
	)

	local col = color(math.random(), math.random(), math.random(), math.random())
	local s = sprite("Maxwell.png", col)

	scene.SetComponent(entity, "Transform", t)
	scene.SetComponent(entity, "Sprite", s)
	-- print("  Spawned entity with")
	-- print("    transform: "..tostring(t))
	-- print("    sprite: "..tostring(s))
end
tracy.ZoneEnd()

tracy.ZoneBeginN("Lua InitDevScene Create Enemies")
-- Create enemies
for _ = 1, 100 do
	local entity = scene.CreateEntity()

	local t = transform(
		vec2(math.random() + math.random(500, 1200), math.random() + math.random(-150, 550)), 
		0.0,
		vec2(math.random() + math.random(15, 30), math.random() + math.random(15, 30))
	)

	local col = color(math.random(), math.random() * 0.25, math.random() * 0.25, 1.0)
	local s = sprite("Maxwell.png", col)

	scene.SetComponent(entity, "Transform", t)
	scene.SetComponent(entity, "Sprite", s)
	scene.SetComponent(entity, "Behaviour", "Behaviours/Enemy")
	-- print("  Spawned entity with")
	-- print("    transform: "..tostring(t))
	-- print("    sprite: "..tostring(s))
	-- print("    Behaviour: Behaviours/Enemy")
end
tracy.ZoneEnd()

tracy.ZoneEnd()