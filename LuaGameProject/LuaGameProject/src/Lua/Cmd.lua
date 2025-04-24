-- Script for executing console commands

local vec2 = require("Vec2")
local transform = require("Transform2")
local sprite = require("Components/Sprite")

local function MakeEnemy(texture, pos, size)
	local entity = scene.CreateEntity()

	local t = transform(
		pos,
		0.0,
		size
	)

	local s = sprite(texture)

	scene.SetComponent(entity, "Transform", t)
	scene.SetComponent(entity, "Sprite", s)
	scene.SetComponent(entity, "Behaviour", "Behaviours/Enemy")
end

--MakeEnemy("Maxwell.png", vec2(300, 35), vec2(200, 200))
MakeEnemy("Transparent2.png", vec2(300, 35), vec2(200, 200))