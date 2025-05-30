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
}


if data == nil then
	data = { }
end

if data.weapons == nil then
	data.weapons = { }
end


tracy.ZoneEnd()