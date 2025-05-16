tracy.ZoneBeginN("Lua PrefabCreatorUI.lua")

local prefabCreatorUI = {
	entityID = -1,

	editEntity = {

	},
	
	createPrefab = {
		stage = -1,
		prefabName = "",
		savedCompList = { },
		popupOpen = false,
		openMessage = "",
	},
	
	-- HACK: This causes coupling, as it needs to be updated when new components are added
	existingComponentsList = {
		"Transform",	"Behaviour",	"Collider",
		"Sprite",		"Health",		"Hardness",
		"CameraData",	"Active",		"Remove"
	}
}


function prefabCreatorUI:EditEntity()
	tracy.ZoneBeginN("Lua prefabCreatorUI:EditEntity")

	-- 'context', abbreviated for convenience
	local ctx = self.editEntity

	-- An entity is requied. If we don't have one, create it
	if not scene.IsEntity(self.entityID) then
		self.entityID = scene.CreateEntity()
	end

	do
		-- TODO
	end

	tracy.ZoneEnd()
end

function prefabCreatorUI:CreatePrefab()
	tracy.ZoneBeginN("Lua prefabCreatorUI:CreatePrefab")

	-- 'context', abbreviated for convenience
	local ctx = self.createPrefab

	-- An entity is requied. If we don't have one, create it
	if not scene.IsEntity(self.entityID) then
		self.entityID = scene.CreateEntity()
	end

	imgui.Begin("Prefab Creator##PrefabCreatorWindow")
	do
		if ctx.stage == -1 then		-- Initial Stage
			-- Description
			imgui.TextWrapped(
				"Use the Entity Editor window to create the entity. "..
				"When you are done, use this window to save the entity as a prefab."
			)
			imgui.Separator()

			
			-- Button to load an existing prefab
			if imgui.Button("Load Existing Prefab##LoadPrefabButton") then
				-- TODO
			end
			imgui.SameLine(0.0, 32.0)

			-- Button to reset the entity
			if imgui.Button("Reset Entity##ResetEntityButton") then
				self:ResetStage(true) -- Reset stage and entity
			end
			imgui.Separator()


			-- Button for saving entity as a prefab
			if imgui.Button("Save as Prefab##SaveAsPrefabButton") then
				ctx.stage = ctx.stage + 1
			end
		elseif ctx.stage == 0 then	-- Naming Stage
			-- Description
			imgui.TextWrapped("Set the prefabs identifying name.")
			imgui.Separator();


			imgui.Text("Name: ##PrefabNameText")
			imgui.SameLine()
			ctx.prefabName = imgui.InputText("##PrefabNameInput", ctx.prefabName)
			imgui.Separator()


			-- Proceed to next stage
			if imgui.Button("Next##SavePrefabStage0") or (Input.KeyPressed(Input.Key.KEY_ENTER) and not ctx.popupOpen) then
				local failedNaming = false

				-- Check if the name is valid
				if ctx.prefabName == "" then
					failedNaming = true
					ctx.popupOpen = true

					ctx.openMessage = "Prefab name cannot be empty."
					imgui.OpenPopup("Message##MessageBoxPopup")
				end

				-- Check if the name is already taken
				if table.hasKey(data.prefabs, ctx.prefabName) then
					failedNaming = true
					ctx.popupOpen = true

					imgui.OpenPopup("Override Existing Prefab?##OverridePrefabPopup")
				end

				if not failedNaming then
					ctx.stage = ctx.stage + 1
				end
			end
			imgui.SameLine(0.0, 32.0)

			-- Return to previous stage
			if imgui.Button("Back##SavePrefabStage0") then
				ctx.stage = ctx.stage - 1
			end
			imgui.SameLine(0.0, 32.0)

			-- Cancel saving entity as prefab
			if imgui.Button("Cancel##SavePrefabStage0") then
				self:ResetStage(false)
			end


			-- Popup to confirm whether to override existing prefab
			if imgui.BeginPopupModal("Override Existing Prefab?##OverridePrefabPopup") then

				imgui.TextWrapped("A prefab with the name "..ctx.prefabName.." already exists.\nDo you want to override it?")
				imgui.Separator()

				if imgui.Button("Yes##ConfirmOverridePrefab") or Input.KeyPressed(Input.Key.KEY_ENTER) then
					ctx.stage = 1
					ctx.popupOpen = false
				end

				imgui.SameLine(0.0, 32.0)

				if imgui.Button("No##DenyOverridePrefab") then
					ctx.popupOpen = false
				end

				if not ctx.popupOpen then
					imgui.CloseCurrentPopup()
				end
				imgui.EndPopup()
			end
		elseif ctx.stage == 1 then	-- Component & Property Stage
			-- Description
			imgui.TextWrapped("Set what data is included in the prefab.")
			imgui.Separator();

		
			-- List all components on the entity using scene.HasComponent()
			for i, compName in ipairs(self.existingComponentsList) do
				if scene.HasComponent(self.entityID, compName) then -- Draw component and all of it's properties
					local doSaveComp = (ctx.savedCompList[compName] ~= nil)
					local pressedSave = false

					-- Add a checkbox signifying if this component should be saved
					doSaveComp, pressedSave = imgui.Checkbox("##"..compName.."CompSaveCheckbox", doSaveComp)
					imgui.SameLine()
					imgui.Text(compName.."##"..compName.."CompSaveText")

					if pressedSave then -- Checkbox was pressed, invert component save state
						if doSaveComp then -- Add component to save list
							ctx.savedCompList[compName] = { }
						else -- Remove component from save list
							ctx.savedCompList[compName] = nil
						end
					end

					local compData = scene.GetComponent(self.entityID, compName)

					if compData ~= nil then
						-- Indent, then list all properties of the current component
						-- Add a checkbox before each property signifying if it should be saved
						-- Checking a property will also mark the component as saved if it isn't already
						imgui.Indent(48.0)
						if compName == "Behaviour" then
							-- LuaRef requires special handling
							-- TODO
						elseif compName == "Collider" then
							-- LuaRef requires special handling
							-- TODO
						else
							for propName, propData in pairs(compData) do
								local doSaveProp = false
								if doSaveComp then -- Property is always unsaved if the component is
									doSaveProp = (ctx.savedCompList[compName][propName] ~= nil)
								end

								doSaveProp, pressedSave = imgui.Checkbox("##"..compName.."Comp"..propName.."PropSaveCheckbox", doSaveProp)
								imgui.SameLine()
								imgui.Text(propName.." ("..type(propData)..")##"..compName.."Comp"..propName.."PropSaveText")
							
								if pressedSave then -- Checkbox was pressed, invert property save state
									if doSaveProp then -- Add property to component in save list
										if not doSaveComp then -- Mark component as saved
											ctx.savedCompList[compName] = { }
										end

										ctx.savedCompList[compName][propName] = { }
									else -- Remove property from component in save list
										ctx.savedCompList[compName][propName] = nil
									end
								end
							end
						end
						imgui.Unindent(48.0)
					end
				end
			end
			imgui.Separator();


			if imgui.Button("Next##SavePrefabStage1") then
				ctx.stage = ctx.stage + 1
			end
			imgui.SameLine(0.0, 32.0)

			if imgui.Button("Back##SavePrefabStage1") then
				ctx.stage = ctx.stage - 1
			end
			imgui.SameLine(0.0, 32.0)

			if imgui.Button("Cancel##SavePrefabStage1") then
				self:ResetStage(false)
			end
		elseif ctx.stage == 2 then	-- Finalization Stage
			-- Description
			imgui.TextWrapped("Before creating the prefab, confirm that the generated table is correct.")
			imgui.Separator();


			-- Go through all conponents + properties in the save list
			-- Find their respective values in the entity and
			
			local prefabTable = { 
				components = {
				}
			}
			--[[
				prefabTable = {

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

			for compName, props in pairs(ctx.savedCompList) do
				if scene.HasComponent(self.entityID, compName) then
					local compData = scene.GetComponent(self.entityID, compName)

					if compName == "Behaviour" then
						-- TODO: Implement
						prefabTable.behaviour = { }
					else
						prefabTable.components[compName] = { }

						if compName == "Collider" then
							-- TODO: Does this need to be separate?
						else
							for propName, i in pairs(props) do
								if compData[propName] ~= nil then
									prefabTable.components[compName][propName] = compData[propName]
								else
									-- TODO: Handle this better
									print("ERROR: Failed to find property "..propName.." in component "..compName.."!")
								end
							end
						end
					end
				else
					-- TODO: Handle this better
					print("ERROR: Failed to find component "..compName.."!")
				end
			end
			imgui.Separator();


			if imgui.Button("Accept##SavePrefabStage2") then
				-- Save the prefab to a file
				local err = data.modding.createLuaTableSave("src/Mods/Prefabs/", "prefabs", ctx.prefabName, prefabTable)

				if err then
					print("Error saving prefab: "..err)
				else
					print("Prefab saved successfully.")
					data.modding.loadLuaTableSave("src/Mods/Prefabs/"..ctx.prefabName..".lts")
				end

				ctx.stage = -1
			end
			imgui.SameLine(0.0, 32.0)

			if imgui.Button("Back##SavePrefabStage2") then
				ctx.stage = ctx.stage - 1
			end
			imgui.SameLine(0.0, 32.0)

			if imgui.Button("Cancel##SavePrefabStage2") then
				self:ResetStage(false)
			end
		end	


		-- Message box popup
		if imgui.BeginPopupModal("Message##MessageBoxPopup") then

			imgui.Text(ctx.openMessage)
			imgui.Separator()

			if imgui.Button("Ok") or Input.KeyPressed(Input.Key.KEY_ENTER) then
				ctx.popupOpen = false
			end

			imgui.SameLine(0.0, 32.0)

			if imgui.Button("Cancel") then
				self:ResetStage(false)
				ctx.popupOpen = false
			end

			if not ctx.popupOpen then
				imgui.CloseCurrentPopup()
			end
			imgui.EndPopup()
		end
	end
	imgui.End()

	tracy.ZoneEnd()
end

function prefabCreatorUI:ResetStage(resetEntity)
	local ctx = self.createPrefab
	
	if resetEntity then
		if scene.IsEntity(self.entityID) then
			scene.RemoveEntity(self.entityID)
		end
		self.entityID = -1
	end

	ctx.stage = -1
	ctx.prefabName = ""
	ctx.savedCompList = { }
end


tracy.ZoneEnd()
return prefabCreatorUI
