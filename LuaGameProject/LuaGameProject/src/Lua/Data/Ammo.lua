tracy.ZoneBeginN("Lua Ammo.lua")

local calibers = {

	-- Summary
	--[[
	["caliber"] = {
		-- Caliber describes the bullet dimensions, and defines what weapons can fire it

		default = "FMJ", -- The ammo type that weapons will spawn with by default

		["ammo type"] = {
			-- Ammo type describes the bullets use case, and defines its stats
			-- Common types are FMJ, HP, AP, whose use cases are described as
			--     FMJ: general purpose
			--     HP:	hits harder and reduces spread, but penetrates less and falls off faster
			--     AP:	less damage and accuracy, but penetrates more

			-- Name:		damageMult
			-- Desc:		Multiplier of the weapon's damage at the moment of firing
			-- Equation:	damage = weaponDamage * damageMult
			-- Type:		float
			-- Range:		[0, inf)
			-- Average:		1.0
			
			-- Name:		falloff
			-- Desc:		Portion of the damage that remains after travelling one meter
			-- Equation:	damage = damage * falloff^(meters)
			-- Type:		float
			-- Range:		(0, 1]
			-- Average:		0.95
			
			-- Name:		penetration
			-- Desc:		Portion of the damage that remains after penetrating a surface, per hardness level
			-- Equation:	damage = damage * penetration^(hardness)
			-- Type:		float
			-- Range:		(0, 1]
			-- Average:		0.5
			
			-- Name:		spread
			-- Desc:		Amount of spread added to the bullet's total spread, measured in degrees
			-- Equation:	totalSpread = math.max(0.0, weaponSpread + ammoSpread + currentRecoil)
			-- Type:		float
			-- Range:		[0, inf)
			-- Average:		3.0
			
			-- Name:		recoil
			-- Desc:		Amount of recoil added to the total current recoil after firing, decaying over time
			-- Equation:	currentRecoil = math.max(0.0, currentRecoil + weaponRecoil + ammoRecoil)
			-- Type:		float
			-- Range:		(-inf, inf)
			-- Average:		5.0
			
			-- Name:		burstSize
			-- Desc:		How many projectiles spawn from a single shot
			-- Equation:	_
			-- Type:		int
			-- Range:		[1, inf)
			-- Average:		1


			-- Special ammo types could add additional members (including functions) to override base functionality
			-- Such members would need to be manually checked for and used if present
		},
	},
	--]]

	["9mm"] = { -- Small arms
		default = "FMJ",

		["FMJ"] = { -- Default
			damageMult = 1.0,
			falloff = 0.95,
			penetration = 0.5,
			spread = 3.0,
			recoil = 2.0,
			burstSize = 1
		},

		["HP"] = {
			damageMult = 1.5,
			falloff = 0.94,
			penetration = 0.4,
			spread = 2.0,
			recoil = 3.5,
			burstSize = 1
		},

		["AP"] = {
			damageMult = 0.75,
			falloff = 0.96,
			penetration = 0.675,
			spread = 5.0,
			recoil = 4.0,
			burstSize = 1
		}
	},

	["12ga"] = { -- Shotguns
		default = "Buck",

		["Buck"] = { -- Default
			damageMult = 0.25,
			falloff = 0.93,
			penetration = 0.4,
			spread = 7.0,
			recoil = 6.0,
			burstSize = 6
		},

		["Slug"] = { -- Much better damage, falloff & spread, but much worse recoil, 
			damageMult = 1.9,
			falloff = 0.96,
			penetration = 0.6,
			spread = 3.5,
			recoil = 9.0,
			burstSize = 1
		},

		["Dart"] = { -- 
			damageMult = 1.0,
			falloff = 0.965,
			penetration = 0.725,
			spread = 1.5,
			recoil = 7.0,
			burstSize = 1
		}
	},

	["5.56"] = { -- Automatic rifles
		default = "FMJ",

		["FMJ"] = { -- Default
			damageMult = 1.0,
			falloff = 0.975,
			penetration = 0.7,
			spread = 2.0,
			recoil = 4.0,
			burstSize = 1
		}
	},

	["308"] = { -- Bolt-action rifles
		default = "FMJ",

		["FMJ"] = { -- Default
			damageMult = 1.0,
			falloff = 0.99,
			penetration = 0.85,
			spread = 0.5,
			recoil = 12.0,
			burstSize = 1
		}
	},
}

-- Get the stats for a specific caliber and ammo type
local function getStats(caliber, type)
	local caliberTable = data.ammo.calibers[caliber]
	if caliberTable ~= nil then
		return caliberTable[type]
	end
	return nil
end

-- Get the default ammo type of a specific caliber
local function getCaliberDefaultType(caliber)
	local caliberTable = data.ammo.calibers[caliber]

	if caliberTable.default ~= nil then
		return caliberTable.default
	end

	if caliberTable ~= nil then
		for typeKey, _ in pairs(caliberTable) do
			return typeKey
		end
	end

	return nil
end


-- Ensure all prerequisites are met before inserting the ammo data
if data == nil then
	data = { }
end

if data.ammo == nil then
	data.ammo = { }
end

if data.ammo.getStats == nil then
	data.ammo.getStats = getStats
end

if data.ammo.getCaliberDefaultType == nil then
	data.ammo.getCaliberDefaultType = getCaliberDefaultType
end

if data.ammo.calibers == nil then
	data.ammo.calibers = { }
end


for caliberKey, caliberValue in pairs(calibers) do 

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

tracy.ZoneEnd()