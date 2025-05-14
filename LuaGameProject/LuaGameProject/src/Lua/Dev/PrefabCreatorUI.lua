tracy.ZoneBeginN("Lua PrefabCreatorUI.lua")

local prefabCreatorUI = {
	createPrefab = {
		selectedComp = -1,

		entityID = -1,

		compList = {
			--[[
				["componentName"] = {
					["propertyName"] = value
				}
			--]]
		},
	}
}


function prefabCreatorUI:CreatePrefab()
	tracy.ZoneBeginN("Lua prefabCreatorUI:CreatePrefab")

	-- Open the editor window
	imgui.Begin("Prefab Creator")

	local ctx = self.createPrefab

	if ctx.entityID == -1 then
		ctx.entityID = scene.CreateEntity()
	end

	do
		-- List all properties of components in the entity
		-- Add a checkmark before each property signifying if it exists in the component list
		-- 

		for componentName, componentTable in pairs(ctx.compList) do
			local componentData = scene.GetComponent(ctx.entityID, componentName)
			
			if componentData then
				imgui.Separator(componentName)

				for propertyName, propertyValue in pairs(componentData) do
					imgui.Text(propertyName)

					--[[
						local propertyType = type(propertyValue)
						local propertyNameString = propertyName.." ("..propertyType..")"
						local propertyValueString = tostring(propertyValue)

						--if imgui.Checkbox(propertyNameString, ctx.compList[componentName][propertyName]) then
						--	ctx.compList[componentName][propertyName] = not ctx.compList[componentName][propertyName]
						--end

						if ctx.compList[componentName][propertyName] then
							--imgui.SameLine()
							imgui.Text(propertyValueString)
						end
					--]]
				end

				imgui.Separator()
			else
				print("Component "..componentName.." not found in entity "..ctx.entityID)
				ctx.compList[componentName] = nil
			end
		end
	end
	
	-- Dropdown to add a new component
	do
		local compList = {"Transform","Behaviour","Collider","Sprite","Health","Hardness","CameraData","Active","Remove"}
		local compListString = table.concat(compList, "\n").."\n\n"
		local pressed = false

		pressed, ctx.selectedComp = imgui.Combo("Add Component", ctx.selectedComp, compListString)
		if pressed and ctx.selectedComp >= 0 then
			local selectedCompName = compList[ctx.selectedComp + 1]
			
			if selectedCompName == "Behaviour" then
				-- TODO: Requires special handling
			elseif selectedCompName == "Collider" then
				-- TODO: Requires special handling
			else
				scene.SetComponent(ctx.entityID, selectedCompName, { })
				ctx.compList[selectedCompName] = { }
			end

			ctx.selectedComp = -1
		end
	end

	--[[
		if imgui.BeginCombo("Add Component", "Select Component") then


			for _, component in ipairs(scene.GetComponentList()) do
				if imgui.Selectable(component) then
					-- Add the component to the entity
					scene.SetComponent(ctx.entityID, component, { })
					-- Add the component to the list
					ctx.compList[component] = { }
				end
			end


			imgui.EndCombo()
		end
	--]]

	imgui.Separator()
	
	if imgui.Button("Submit") then
		-- Save the component list as a prefab using table.save

		-- Delete the entity
		scene.RemoveEntity(ctx.entityID)
		ctx.entityID = -1
	end
	
	if imgui.Button("Reset") then
		ctx.compList = { }

		if ctx.entityID ~= -1 then
			scene.RemoveEntity(ctx.entityID)
			ctx.entityID = -1
		end
	end
	
	imgui.End()
	tracy.ZoneEnd()
end


tracy.ZoneEnd()
return prefabCreatorUI
