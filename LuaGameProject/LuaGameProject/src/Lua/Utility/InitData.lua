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

function data.modding.loadLuaTableSave(path)
	-- path is the filepath of a lua table save, created using table.save
	-- local luaTableSave = {
	-- 	   dataPath		(string)	-- Location of this table in the data table, ex: "weapons", "ammo.caliber"
	-- 	   elementName	(string)	-- Name of the element in the dataPath, ex: "AK47", "45acp"
	-- 	   contents		(table)		-- The table stored at data.dataPath[elementName]
	-- }

	local luaTableSave = table.load(path)

	if luaTableSave == nil then
		print("Error loading LTS file: "..path)
		return
	end

	-- split the dataPath into steps, separated by "."
	local dataPath = luaTableSave.dataPath
	local dataPathSteps = { }
	for step in string.gmatch(dataPath, "[^%.]+") do
		table.insert(dataPathSteps, step)
	end

	-- Insert the contents into the data table at the dataPath
	local current = data
	for _, step in ipairs(dataPathSteps) do
		if current[step] == nil then
			-- Create a new table if the step doesn't exist
			current[step] = {}
			current = current[step]
		elseif type(current[step]) == "table" then
			-- Proceed to the next step if it's a table
			current = current[step]
		else
			-- Error if the step is not a table and can't be traversed
			print("Error: Cannot traverse dataPath '" .. dataPath .. "', step '" .. step .. "' is not a table.")
			return
		end
	end

	local elementName = luaTableSave.elementName
	if current[elementName] then
		print("Overwriting data."..dataPath.."."..elementName.."...")
	end
	current[elementName] = luaTableSave.contents
end

tracy.ZoneEnd()
