
local vector = require("Vector")
local transform = require("Transform")

print("Initiating dev scene from lua...")

for _ = 1, 25 do
	local entity = scene.CreateEntity()

	local t = transform(
		vector(math.random() + math.random(0, 200), math.random() + math.random(0, 100), 0.0), 
		vector(0.0, 0.0, 0.0),
		vector(math.random() + math.random(0, 20), math.random() + math.random(0, 20), 1.0)
	)
	local s = "Maxwell.png"

	scene.SetComponent(entity, "transform", t)
	scene.SetComponent(entity, "sprite", s)
	print("Spawned entity with trasform: "..tostring(t))
end

print("Dev scene initialized")