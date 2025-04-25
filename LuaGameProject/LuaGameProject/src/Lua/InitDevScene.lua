tracy.ZoneBeginN("Lua InitDevScene.lua")

local vec2 = require("Vec2")
local transform = require("Transform2")
local color = require("Color")
local sprite = require("Components/Sprite")
local cameraData = require("Components/CameraData")
--local playerCamera = require("Behaviours/PlayerCamera")


-- Create Walls ----------------
tracy.ZoneBeginN("Lua Create Walls")
for _ = 1, 50 do
	local entity = scene.CreateEntity()

	local t = transform(
		vec2(math.random() + math.random(500, 1200), math.random() + math.random(-150, 550)), 
		0.0,
		vec2(math.random() + math.random(1, 100), math.random() + math.random(1, 100))
	)

	local col = color(math.random(), math.random(), math.random(), math.random())
	local s = sprite("", col)
	s.priority = math.random(10, 100)

	scene.SetComponent(entity, "Transform", t)
	scene.SetComponent(entity, "Sprite", s)
end
tracy.ZoneEnd()
--------------------------------


-- Create Player ---------------
tracy.ZoneBeginN("Lua Create Player")
local pEnt = scene.CreateEntity()

local pT = transform(
	vec2(600, 300), 
	0.0,
	vec2(100, 100)
)

local pS = sprite("Transparent2.png")
pS.priority = 500

scene.SetComponent(pEnt, "Transform", pT)
scene.SetComponent(pEnt, "Sprite", pS)
scene.SetComponent(pEnt, "Behaviour", "Behaviours/Player")
tracy.ZoneEnd()
--------------------------------


-- Create Camera ---------------
tracy.ZoneBeginN("Lua Create Camera")
local cEnt = scene.CreateEntity()

local cT = pT
local cD = cameraData()

scene.SetComponent(cEnt, "Transform", cT)
scene.SetComponent(cEnt, "CameraData", cD)
scene.SetComponent(cEnt, "Behaviour", "Behaviours/PlayerCamera")

-- Get the camera and set the tracked entity
local cam = scene.GetComponent(cEnt, "Behaviour")
cam:SetTrackedEntity(pEnt)

tracy.ZoneEnd()
--------------------------------


-- Create Enemies --------------
tracy.ZoneBeginN("Lua Create Enemies")
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
--------------------------------


tracy.ZoneEnd()