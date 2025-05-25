tracy.ZoneBeginN("Lua Commands.lua")

local gameMath = require("Utility/GameMath")
local vec2 = require("Vec2")
local transform = require("Transform2")

if not dev then
	dev = { }
end


-- Create Weapon at Mouse
function dev.MakeWeap(name)
	tracy.ZoneBeginN("Lua Command Create Weapon")

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

function dev.GiveAmmo(caliber, type, count)
	tracy.ZoneBeginN("Lua Command Give Ammo")

	if not game.GetPlayer then
		print("No player found to give ammo to!")
		tracy.ZoneEnd()
		return
	end

	local player = game.GetPlayer()

	if not player then
		print("No player found to give ammo to!")
		tracy.ZoneEnd()
		return
	end

	if not player.ammoReserve[caliber] then
		player.ammoReserve[caliber] = {}
	end

	if player.ammoReserve[caliber][type] then
		player.ammoReserve[caliber][type] = player.ammoReserve[caliber][type] + count
	else
		player.ammoReserve[caliber][type] = count
	end

	print("Gave "..count.." "..caliber.." ("..type..") ammo to player.")
	print("Player now has "..player.ammoReserve[caliber][type]..".")

	tracy.ZoneEnd()
end

-- Give player all resources
function dev.Impulse101()
	tracy.ZoneBeginN("Lua Command Impulse 101")

	if not game.GetPlayer then
		print("No player found to give ammo to!")
		tracy.ZoneEnd()
		return
	end

	local player = game.GetPlayer()

	if not player then
		print("No player found to give ammo to!")
		tracy.ZoneEnd()
		return
	end

	-- loop over all calibers + types in data.ammo and give player 9999 of each type
	for caliber, types in pairs(data.ammo.calibers) do

		if not player.ammoReserve[caliber] then
			player.ammoReserve[caliber] = {}
		end

		for type, stats in pairs(types) do
			if type ~= "default" then
				player.ammoReserve[caliber][type] = 9999
			end
		end
	end

	tracy.ZoneEnd()
end

-- Shake the camera
function dev.Shake(frequency, amplitude, duration)
	tracy.ZoneBeginN("Lua Command Shake")

	if game.GetPlayerCamera == nil then
		print("No get player camera function found!")
		tracy.ZoneEnd()
		return
	end

	local playerCamera = game.GetPlayerCamera()
	if playerCamera == nil then
		print("No player camera found to shake!")
		tracy.ZoneEnd()
		return
	end

	playerCamera:ApplyShake(frequency, amplitude, duration)
	tracy.ZoneEnd()
end

tracy.ZoneEnd()