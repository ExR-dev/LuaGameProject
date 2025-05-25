tracy.ZoneBeginN("Lua InitEditorPresetCreatorScene.lua")

local gameMath = require("Utility/GameMath")
local vec2 = require("Vec2")
local transform = require("Transform2")
local color = require("Color")
local sprite = require("Components/Sprite")
local cameraData = require("Components/CameraData")


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

tracy.ZoneEnd()