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
}


if data == nil then
	data = { }
end

if data.ammo == nil then
	data.ammo = { }
end

if data.ammo.calibers == nil then
	data.ammo.calibers = { }
end


if data.ammo.getStats == nil then

	-- Get the stats for a specific caliber and ammo type
	local function getStats(caliber, type)
		local caliberTable = data.ammo.calibers[caliber]
		if caliberTable ~= nil then
			return caliberTable[type]
		end
		return nil
	end

	data.ammo.getStats = getStats
end

if data.ammo.getCaliberAmmoTypes == nil then

	-- Get a list of all ammo type of a specific caliber
	local function getCaliberAmmoTypes(caliber)
		local ammoTypes = { }
		local caliberTable = data.ammo.calibers[caliber]

		if caliberTable ~= nil then
			for typeKey, typeValue in pairs(caliberTable) do
				if typeKey ~= "default" then
					table.insert(ammoTypes, typeKey)
				end
			end
		end

		return ammoTypes
	end

	data.ammo.getCaliberAmmoTypes = getCaliberAmmoTypes
end

if data.ammo.getCaliberDefaultType == nil then

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

	data.ammo.getCaliberDefaultType = getCaliberDefaultType
end


tracy.ZoneEnd()