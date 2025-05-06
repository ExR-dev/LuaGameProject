tracy.ZoneBeginN("Lua Weapons.lua")

local weapons = {

	-- Summary
	--[[
	["weapon"] = {

		-- Render data
		sprite,
		width,
		length,

		stats = {
			-- Name:		handCount
			-- Desc:		TODO
			-- Equation:	_
			-- Type:		int
			-- Range:		[1, 2]
			-- Average:		2

			-- Name:		caliber
			-- Desc:		TODO
			-- Equation:	_
			-- Type:		string
			-- Range:		_
			-- Average:		_

			-- Name:		fireMode
			-- Desc:		TODO
			-- Equation:	_
			-- Type:		string
			-- Range:		_
			-- Average:		_

			-- Name:		capacity
			-- Desc:		TODO
			-- Equation:	_
			-- Type:		int
			-- Range:		[0, inf)
			-- Average:		15

			-- Name:		damage
			-- Desc:		TODO
			-- Equation:	damage = damage * ammoDamageMult
			-- Type:		float
			-- Range:		[0, inf)
			-- Average:		15.0

			-- Name:		fireRate
			-- Desc:		TODO
			-- Equation:	cooldown = 1.0 / fireRate
			-- Type:		float
			-- Range:		[0, inf)
			-- Average:		3.0

			-- Name:		reloadTime
			-- Desc:		TODO
			-- Equation:	_
			-- Type:		float
			-- Range:		[0, inf)
			-- Average:		2.0

			-- Name:		spread
			-- Desc:		TODO
			-- Equation:	totalSpread = math.max(0.0, spread + ammoSpread + currentRecoil)
			-- Type:		float
			-- Range:		(-inf, inf)
			-- Average:		0.0

			-- Name:		recoil
			-- Desc:		Added spread by firing, decaying over time
			-- Equation:	currentRecoil = math.max(0.0, currentRecoil + recoil + ammoRecoil)
			-- Type:		float
			-- Range:		(-inf, inf)
			-- Average:		0.0

			-- Name:		recovery
			-- Desc:		How fast the recoil decays, higher values means faster recovery
			-- Equation:	See gameMath.expDecay(), recovery is passed as the d parameter
			-- Type:		int
			-- Range:		[1, inf)
			-- Average:		5


			-- Special weapons could add additional members (including functions) to override base functionality
			-- Such members would need to be manually checked for and used if present
		}
	},
	--]]

	["Glock"] = {

		sprite = nil,
		width = 10,
		length = 36,

		stats = {
			handCount = 1,
			caliber = "9mm",
			fireMode = "Semi",
			capacity = 18,
			damage = 10.0,
			fireRate = 8.0,
			reloadTime = 0.7,
			spread = 2.0,
			recoil = 2.0,
			recovery = 8
		}
	},

	["Spas-12"] = {

		sprite = nil,
		width = 14,
		length = 64,

		stats = {
			handCount = 2,
			caliber = "12ga",
			fireMode = "Semi",
			capacity = 8,
			damage = 40.0,
			fireRate = 4.0,
			reloadTime = 2.0,
			spread = 2.5,
			recoil = 7.0,
			recovery = 2
		}
	},

	["AR-15"] = {

		sprite = nil,
		width = 12,
		length = 48,

		stats = {
			handCount = 2,
			caliber = "5.56",
			fireMode = "Auto",
			capacity = 30,
			damage = 35.0,
			fireRate = 8.0,
			reloadTime = 1.0,
			spread = 0.8,
			recoil = 1.75,
			recovery = 6
		}
	},

	["M700"] = {

		sprite = nil,
		width = 10,
		length = 64,

		stats = {
			handCount = 2,
			caliber = "308",
			fireMode = "Semi",
			capacity = 5,
			damage = 75.0,
			fireRate = 1.0,
			reloadTime = 3.0,
			spread = 0.3,
			recoil = 13.0,
			recovery = 3
		}
	},

	["God Gun"] = {

		sprite = nil,
		width = 6,
		length = 80,

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
			recovery = 8
		}
	},
}


-- Ensure all prerequisites are met before inserting the weapon data
if data == nil then
	data = { }
end

if data.weapons == nil then
	data.weapons = { }
end


for key, value in pairs(weapons) do

	-- Warn if the weapon already exists
	if data.weapons[key] ~= nil then
		print("Overwriting weapon "..key.."...")
	end

	data.weapons[key] = value
end

tracy.ZoneEnd()