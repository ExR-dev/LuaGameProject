
local vec2 = require("Vec2")
local transform = require("Transform2")
local color = require("Color")
local sprite = require("Components/Sprite")

print("Initiating dev scene from lua...")

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
	print("  Spawned entity with")
	print("    transform: "..tostring(t))
	print("    sprite: "..tostring(s))
end

print("Dev scene initialized.")