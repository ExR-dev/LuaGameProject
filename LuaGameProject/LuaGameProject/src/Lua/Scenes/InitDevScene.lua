tracy.ZoneBeginN("Lua InitDevScene.lua")

local gameMath = require("Utility/GameMath")
local vec2 = require("Vec2")
local transform = require("Transform2")
local color = require("Color")
local sprite = require("Components/Sprite")
local cameraData = require("Components/CameraData")
local playerCamera = require("Behaviours/PlayerCamera")
local collider = require("Components/Collider")


-- Don't spawn player if already exists
if game.GetPlayer == nil then
	-- Create Player ---------------
	tracy.ZoneBeginN("Lua Create Player")
	local playerEnt = scene.CreateEntity()

	local playerT = transform(
		vec2(600, 300), 
		0.0,
		vec2(100, 100)
	)

	local playerS = sprite("Transparent2.png")
	playerS.priority = 20

	scene.SetComponent(playerEnt, "Transform", playerT)
	scene.SetComponent(playerEnt, "Sprite", playerS)
	scene.SetComponent(playerEnt, "Behaviour", "Behaviours/Player")
	tracy.ZoneEnd()
	--------------------------------


	-- Create Camera ---------------
	tracy.ZoneBeginN("Lua Create Camera")
	local camEnt = scene.CreateEntity()

	local camT = playerT
	local camD = cameraData()

	scene.SetComponent(camEnt, "Transform", camT)
	scene.SetComponent(camEnt, "CameraData", camD)
	scene.SetComponent(camEnt, "Behaviour", "Behaviours/PlayerCamera")

	-- Get the camera and set the tracked entity
	game.GetPlayerCamera():SetTrackedEntity(playerEnt)

	tracy.ZoneEnd()
	--------------------------------


	-- Create Cursor ---------------
	tracy.ZoneBeginN("Lua Create Cursor")
	local cursorEnt = scene.CreateEntity()

	local cursorS = sprite("Cursor.png")
	cursorS.priority = 1000

	-- local cursorT = transform(vec2(0, 0), 0, vec2(30, 30))
	local cursorT = transform()
	scene.SetComponent(cursorEnt, "Transform", cursorT)
	scene.SetComponent(cursorEnt, "Sprite", cursorS)
	scene.SetComponent(cursorEnt, "Behaviour", "Behaviours/Cursor")

	tracy.ZoneEnd()
	--------------------------------
	
	local function MakeWeaponFunc(name, pos)
		-- Create Weapon ---------------
		tracy.ZoneBeginN("Lua Create Weapon")
		local weaponEnt = scene.CreateEntity()

		local weaponT = transform()
		weaponT.position = pos

		scene.SetComponent(weaponEnt, "Transform", weaponT)
		scene.SetComponent(weaponEnt, "Behaviour", "Behaviours/Weapon")

		-- Get the weapon behaviour to set the stats
		local weaponBehaviour = scene.GetComponent(weaponEnt, "Behaviour")
		weaponBehaviour:LoadType(name)

		tracy.ZoneEnd()
		--------------------------------
	end

	MakeWeaponFunc("Glock", vec2(0, 512))
	MakeWeaponFunc("Glock", vec2(0, 640))

	MakeWeaponFunc("Spas-12", vec2(128, 512))
	MakeWeaponFunc("Spas-12", vec2(128, 640))

	MakeWeaponFunc("AR-15", vec2(256, 512))
	MakeWeaponFunc("AR-15", vec2(256, 640))

	MakeWeaponFunc("M700", vec2(384, 512))
	MakeWeaponFunc("M700", vec2(384, 640))

	MakeWeaponFunc("God Gun", vec2(512, 512))
	MakeWeaponFunc("God Gun", vec2(512, 640))
	
end


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


-- Create Walls ----------------
tracy.ZoneBeginN("Lua Create Walls")
for _ = 1, 10 do
	local entity = game.SpawnPrefab("Wall")

	local t = transform(
		vec2(math.random() + math.random(500, 1200), math.random() + math.random(-150, 550)), 
		0.0,
		vec2(math.random() + math.random(10, 150), math.random() + math.random(10, 150))
	)

	scene.SetComponent(entity, "Transform", t)
end
tracy.ZoneEnd()
--------------------------------


-- Create Enemy Prefab ---------
tracy.ZoneBeginN("Lua Create Enemies")
for _ = 1, 15 do
	local prefabEntity = game.SpawnPrefab("Enemy")
	if prefabEntity then
		local prefabT = scene.GetComponent(prefabEntity, "Transform")
		prefabT.position = vec2(math.random() + math.random(500, 1200), math.random() + math.random(-150, 550))
		scene.SetComponent(prefabEntity, "Transform", prefabT)
	end
end
tracy.ZoneEnd()
--------------------------------

-- Spawn Dungeon ---------------
tracy.ZoneBeginN("Lua Create Dungeon")

game.SpawnGroup("dungeon", transform(vec2(0, 0), 0, vec2(1, 1)), "dungeons")

tracy.ZoneEnd()
--------------------------------

tracy.ZoneEnd()