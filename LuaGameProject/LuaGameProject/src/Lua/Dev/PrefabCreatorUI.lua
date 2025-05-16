tracy.ZoneBeginN("Lua PrefabCreatorUI.lua")

local prefabCreatorUI = {
	entityID = -1,

	editEntity = {
		selectedComp = -1
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
	imgui.Separator()

	-- Dropdown to add a new component
	do
		local compListString = table.concat(self.existingComponentsList, "\n").."\n\n"
		local pressed = false
		pressed, ctx.selectedComp = imgui.Combo("Add Component##PrefabEditorAddComponent", ctx.selectedComp, compListString)

		if pressed and ctx.selectedComp >= 0 then
			local selectedCompName = self.existingComponentsList[ctx.selectedComp + 1]
			
			if selectedCompName == "Behaviour" then
				local scriptPath = "Behaviours/InputMovement"
				scene.SetComponent(self.entityID, selectedCompName, scriptPath)
			else
				scene.SetComponent(self.entityID, selectedCompName, { })
			end

			ctx.selectedComp = -1
		end
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

	local openMessageBox = false
	local namingCollision = false

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


			imgui.Text("Name: ")
			imgui.SameLine()
			ctx.prefabName = imgui.InputText("##PrefabNameInput", ctx.prefabName)
			imgui.Separator()


			-- Check if name is valid and proceed to next stage
			if imgui.Button("Next##SavePrefabStage0") or (Input.KeyPressed(Input.Key.KEY_ENTER) and not ctx.popupOpen) then
				local failedNaming = false

				-- Check if the name is valid
				if ctx.prefabName == "" then
					failedNaming = true
					ctx.popupOpen = true

					ctx.openMessage = "Prefab name cannot be empty."
					openMessageBox = true
				end

				-- Check if the name is already taken
				if table.hasKey(data.prefabs, ctx.prefabName) then
					failedNaming = true
					ctx.popupOpen = true
					namingCollision = true
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
					imgui.Text(compName)

					if pressedSave then -- Checkbox was pressed, invert component save state
						if doSaveComp then -- Add component to save list
							ctx.savedCompList[compName] = { }
						else -- Remove component from save list
							ctx.savedCompList[compName] = nil
						end
					end

					local compData = scene.GetComponent(self.entityID, compName)

					if compData ~= nil then

						local skippedProps = { }
						local skippedTypes = { "function" }

						if compName == "Behaviour" then
							-- Entity ID and behaviour script path are handelled automatically
							-- Therefore, do not show them
							table.insert(skippedProps, "ID")
							table.insert(skippedProps, "path")
						end

						-- Indent, then list all properties of the current component
						-- Add a checkbox before each property signifying if it should be saved
						-- Checking a property will also mark the component as saved if it isn't already
						imgui.Indent(32.0)
						for propName, propData in pairs(compData) do
							local propType = type(propData)

							-- Skip property if name or type is in the skip list
							local skipProp = (
								table.hasValue(skippedProps, propName) or 
								table.hasValue(skippedTypes, propType)
							)

							if not skipProp then
								local doSaveProp = false
								if doSaveComp then -- Property is always unsaved if the component is
									doSaveProp = (ctx.savedCompList[compName][propName] ~= nil)
								end

								doSaveProp, pressedSave = imgui.Checkbox("##"..compName.."Comp"..propName.."PropSaveCheckbox", doSaveProp)
								imgui.SameLine()
								imgui.Text(propName.." ("..propType..")")
							
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
						imgui.Unindent(32.0)
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

			
			-- Table containing all saved values
			local prefabTable = { 
				components = {
				}
			}

			-- Go through all saved components + properties and fetch their values to the prefab table
			for compName, props in pairs(ctx.savedCompList) do
				if scene.HasComponent(self.entityID, compName) then
					local compData = scene.GetComponent(self.entityID, compName)
					local compTable = nil

					if compName == "Behaviour" then
						prefabTable.behaviour = { 
							path = compData.path,
							properties = { }
						}
						compTable = prefabTable.behaviour.properties
					else
						prefabTable.components[compName] = { }
						compTable = prefabTable.components[compName]
					end

					for propName, i in pairs(props) do
						compTable[propName] = compData[propName]
					end
				else
					-- TODO: Handle this in a better way
					print("ERROR: Failed to find component "..compName.."!")
				end
			end

			imgui.Text(ctx.prefabName.." = "..table.toString(prefabTable))
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
	end
	imgui.End()

	-- Popup to confirm whether to override existing prefab
	if namingCollision then
		imgui.OpenPopup("Override Prefab")
	end

	if imgui.BeginPopupModal("Override Prefab") then
		imgui.TextWrapped(
			"A prefab with the name "..ctx.prefabName.." already exists.\n"..
			"Do you want to override it?"
		)
		imgui.Separator()

		if imgui.Button("Yes##ConfirmOverridePrefabPopup") or Input.KeyPressed(Input.Key.KEY_ENTER) then
			ctx.stage = ctx.stage + 1
			ctx.popupOpen = false
		end
		imgui.SameLine(0.0, 32.0)

		if imgui.Button("No##DenyOverridePrefabPopup") then
			ctx.popupOpen = false
		end

		if not ctx.popupOpen then
			imgui.CloseCurrentPopup()
		end

		imgui.EndPopup()
	end


	-- Message box popup
	if openMessageBox then
		ctx.popupOpen = true
		imgui.OpenPopup("Message Box")
	end

	if imgui.BeginPopupModal("Message Box") then
		imgui.TextWrapped(ctx.openMessage)
		imgui.Separator()

		if imgui.Button("Ok##MessageBoxPopup") or Input.KeyPressed(Input.Key.KEY_ENTER) then
			ctx.popupOpen = false
		end
		imgui.SameLine(0.0, 32.0)

		if imgui.Button("Cancel##MessageBoxPopup") then
			self:ResetStage(false)
			ctx.popupOpen = false
		end

		if not ctx.popupOpen then
			imgui.CloseCurrentPopup()
		end
		imgui.EndPopup()
	end

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
