tracy.ZoneBeginN("Lua Commands.lua")

local gameMath = require("Utility/GameMath")
local vec2 = require("Vec2")
local transform = require("Transform2")

if not dev then
	dev = { }
end

-- Create Weapon at Mouse
function dev.MakeWeap(name)
	tracy.ZoneBeginN("Lua Create Weapon")

	local weaponEnt = scene.CreateEntity()

	local spawnPos = nil
	if game.GetCursor then
		spawnPos = game.GetCursor().trans.position
	else
		local mouseWorldPos = game.GetMouseWorldPos()
		spawnPos = vec2(mouseWorldPos.x, mouseWorldPos.y)
	end

	local weaponT = transform()
	weaponT.position = spawnPos

	scene.SetComponent(weaponEnt, "Transform", weaponT)
	scene.SetComponent(weaponEnt, "Behaviour", "Behaviours/Weapon")

	-- Get the weapon behaviour to set the stats
	local weaponBehaviour = scene.GetComponent(weaponEnt, "Behaviour")
	weaponBehaviour:LoadType(name)

	tracy.ZoneEnd()
end


tracy.ZoneEnd()