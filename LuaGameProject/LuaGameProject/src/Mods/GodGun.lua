tracy.ZoneBeginN("Lua GodGun.lua")

-- Define the mod data
local weapons = {

	["God Gun"] = { -- Example mod weapon

		sprite = nil,
		width = 24,
		length = 96,

		stats = {
			handCount = 1,
			caliber = "God Ammo",
			fireMode = "Auto",
			capacity = 99999,
			damage = 9999.0,
			fireRate = 90.0,
			reloadTime = 0.0,
			spread = 0.0,
			recoil = 0.0,
			recovery = 4
		}
	},
}

local calibers = {
	["God Ammo"] = { -- Example mod caliber
		default = "Spray",

		["Spray"] = {
			damageMult = 1.0,
			falloff = 1.0,
			penetration = 1.0,
			spread = 15.0,
			recoil = 3.0,
			burstSize = 9
		},

		["Accurate"] = {
			damageMult = 1.0,
			falloff = 1.0,
			penetration = 1.0,
			spread = 0.0,
			recoil = 10.0,
			burstSize = 1
		}
	},
}

-- Insert the mod data
data.modding.loadWeaponMod(weapons)
data.modding.loadAmmoMod(calibers)

tracy.ZoneEnd()
