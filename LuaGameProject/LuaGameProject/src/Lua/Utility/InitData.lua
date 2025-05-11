tracy.ZoneBeginN("Lua InitData.lua")

-- Ensure all prerequisite data is initialized & empty before loading data
data = { }
data.modding = { }
data.weapons = { }
data.ammo = { }
data.ammo.calibers = { }


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
