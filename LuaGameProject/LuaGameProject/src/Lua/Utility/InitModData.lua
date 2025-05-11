tracy.ZoneBeginN("Lua InitModData.lua")

-- Ensure all prerequisite data is initialized before loading mods
if data == nil then
	data = { }
end

if data.modding == nil then
	data.modding = { }
end

if data.weapons == nil then
	data.weapons = { }
end

if data.ammo == nil then
	data.ammo = { }
end

if data.ammo.calibers == nil then
	data.ammo.calibers = { }
end


-- Define mod loading functions
function data.modding.loadWeaponMod(weaponData)
	for key, value in pairs(weaponData) do
		-- Warn if the weapon already exists
		if data.weapons[key] ~= nil then
			print("Overwriting weapon "..key.."...")
		end

		data.weapons[key] = value
	end
end

function data.modding.loadAmmoMod(ammoData)
	for caliberKey, caliberValue in pairs(ammoData) do 

		if data.ammo.calibers[caliberKey] == nil then
			data.ammo.calibers[caliberKey] = caliberValue
		else
			for ammoTypeKey, ammoTypeValue in pairs(caliberValue) do 

				-- Warn if the ammo type already exists
				if (data.ammo.calibers[caliberKey])[ammoTypeKey] ~= nil then
					print("Overwriting ammo type "..ammoTypeKey.."...")
				end

				(data.ammo.calibers[caliberKey])[ammoTypeKey] = ammoTypeValue
			end
		end
	end
end

tracy.ZoneEnd()
