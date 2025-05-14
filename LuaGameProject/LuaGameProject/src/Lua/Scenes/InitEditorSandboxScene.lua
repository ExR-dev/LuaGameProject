tracy.ZoneBeginN("Lua InitEditorSandboxCreatorScene.lua")

local gameMath = require("Utility/GameMath")
local vec2 = require("Vec2")
local transform = require("Transform2")
local color = require("Color")
local sprite = require("Components/Sprite")
local cameraData = require("Components/CameraData")


local unitEnt1 = scene.CreateEntity()

local unitT1 = transform(
	vec2(0, 0), 
	0.0,
	vec2(gameMath.metersToPixels, gameMath.metersToPixels)
)

local unitS1 = sprite("", color(0.4, 0, 0, 1), 1)

scene.SetComponent(unitEnt1, "Transform", unitT1)
scene.SetComponent(unitEnt1, "Sprite", unitS1)

-- Create Unit Cube ------------
tracy.ZoneBeginN("Lua Unit Cube")
local unitEnt = scene.CreateEntity()

local unitT = transform(
	vec2(0, 0), 
	0.0,
	vec2(gameMath.metersToPixels, gameMath.metersToPixels)
)

local unitS = sprite("", color(0, 0, 0, 1))

scene.SetComponent(unitEnt, "Transform", unitT)
scene.SetComponent(unitEnt, "Sprite", unitS)
tracy.ZoneEnd()
--------------------------------



local unitEnt2 = scene.CreateEntity()

local unitT2 = transform(
	vec2(0, 0), 
	0.0,
	vec2(gameMath.metersToPixels, gameMath.metersToPixels)
)

local unitS2 = sprite("", color(0.0, 0, 0.4, 1), -1)

scene.SetComponent(unitEnt2, "Transform", unitT2)
scene.SetComponent(unitEnt2, "Sprite", unitS2)


tracy.ZoneEnd()