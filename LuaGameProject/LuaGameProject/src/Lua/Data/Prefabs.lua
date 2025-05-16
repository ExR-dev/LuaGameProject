-- Prefabs are entities with a predefined set of components and properties.

local vec2 = require("Vec2")

local prefabs = {
	-- Summary
	--[[
		["prefab name"] = {

			behaviour = {
				path = "",

				properties = {
					["property"] = value,
				},
			},
			
			components = {
				["component name"] = {
					["property"] = value,
				},
			},
		}
	--]]
}


if data == nil then
	data = { }
end

if data.prefabs == nil then
	data.prefabs = { }
end

if game == nil then
	game = { }
end

if game.SpawnPrefab == nil then
	local function SpawnPrefab(prefab)
		local prefabData = data.prefabs[prefab]

		if not prefabData then
			print("Prefab not found: "..prefab)
			return nil
		end

		local entity = scene.CreateEntity()
		
		-- First add collider if present
		local colliderTable = prefabData.components["Collider"]
		if colliderTable ~= nil then
			scene.SetComponent(entity, "Collider", colliderTable)
		end

		-- Add behaviour
		if prefabData.behaviour then
			scene.SetComponent(entity, "Behaviour", prefabData.behaviour.path)

			local behaviour = scene.GetComponent(entity, "Behaviour")

			if behaviour then
				for propertyName, propertyData in pairs(prefabData.behaviour.properties) do
					behaviour[propertyName] = propertyData
				end
			else
				print("Failed to create behaviour "..prefabData.behaviour.path.."!")
			end
		end

		-- Add other components
		for componentName, componentData in pairs(prefabData.components) do
			if componentName ~= "Collider" then -- Skip collider
				scene.SetComponent(entity, componentName, componentData)
			end
		end

		return entity
	end
	game.SpawnPrefab = SpawnPrefab
end

data.prefabs = prefabs -- HACK