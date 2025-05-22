tracy.ZoneBeginN("Lua PresetCreatorUI.lua")

local presetCreatorUI = {

	presetGroups = { "Weapon", "Ammo" },
	selectedPresetGroup = nil,

	weaponEditorUI = {
		editingWeapon = nil,		-- string
		newWeaponName = nil,		-- string
		editedWeaponTable = nil		-- table
	},

	ammoEditorUI = {
		selectedAmmoType = 0,
		newAmmoTypeName = "",
		editingCaliber = nil,		-- string
		newCaliberName = nil,		-- string
		editedCaliberTable = nil	-- table
	}
}

function presetCreatorUI:PresetEditorUI()
	tracy.ZoneBeginN("Lua presetCreatorUI:PresetEditorUI")

	imgui.Begin("Preset Group Selector")
	for i, groupName in ipairs(self.presetGroups) do
		if imgui.Button(groupName) then
			self.selectedPresetGroup = i
		end

		if self.selectedPresetGroup == i then
			imgui.SameLine()
			imgui.Text("(Selected)")
		end
	end
	imgui.End()

	if self.selectedPresetGroup ~= nil then
		imgui.Begin("Preset Selector")
		
		if self.selectedPresetGroup == 1 then
			self:WeaponEditorUI()
		elseif self.selectedPresetGroup == 2 then
			self:AmmoEditorUI()
		end

		imgui.End()
	end

	tracy.ZoneEnd()
end

function presetCreatorUI:WeaponEditorUI()
	tracy.ZoneBeginN("Lua presetCreatorUI:WeaponEditorUI")

	local ctx = self.weaponEditorUI

	-- List all weapons in data.weapons as buttons
	for weaponName, _ in pairs(data.weapons) do
		if imgui.Button(weaponName) then
			self.weaponEditorUI.editingWeapon = weaponName
			self.weaponEditorUI.editedWeaponTable = nil
		end

		if self.weaponEditorUI.editingWeapon == weaponName then
			imgui.SameLine()
			imgui.Text("(Selected)")
		end
	end

	-- Add a button for creating a new weapon
	if imgui.Button("New Weapon##NewWeaponButton") then
		self.weaponEditorUI.newWeaponName = ""
		imgui.OpenPopup("New Weapon Name##NameWeaponPopup")
	end

	-- Open a popup to name the weapon
	if imgui.BeginPopup("New Weapon Name##NameWeaponPopup") then

		imgui.Text("Enter a name for the new weapon:")
		self.weaponEditorUI.newWeaponName = imgui.InputText("##NewWeaponNameInput", self.weaponEditorUI.newWeaponName, 64)
			
		if imgui.Button("Confirm") or Input.KeyPressed(Input.Key.KEY_ENTER) then -- Submit if Confirm button or Enter key is pressed
			local newWeapon = {
				[self.weaponEditorUI.newWeaponName] = {

					sprite = nil,
					width = 12,
					length = 28,

					stats = {
						handCount = 1,
						caliber = "9mm",
						fireMode = "Semi",
						capacity = 1,
						damage = 0.0,
						fireRate = 0.0,
						reloadTime = 0.0,
						spread = 0.0,
						recoil = 0.0,
						recovery = 6
					}
				},
			}

			-- Insert the weapon
			data.modding.loadWeaponMod(newWeapon)

			-- Set the new weapon as the editing weapon
			self.weaponEditorUI.editingWeapon = self.weaponEditorUI.newWeaponName
			self.weaponEditorUI.editedWeaponTable = nil
			self.weaponEditorUI.newWeaponName = nil
			imgui.CloseCurrentPopup()
		end

		imgui.EndPopup()
	end


	if self.weaponEditorUI.editingWeapon then
		-- Before editing, copy the original weapon table to perform the edits on
		if self.weaponEditorUI.editedWeaponTable == nil then
			self.weaponEditorUI.editedWeaponTable = table.deepCopy(
				data.weapons[self.weaponEditorUI.editingWeapon]
			)
		end

		local editorOpen = true
		local resetState = false

		-- Open the editor window
		editorOpen = imgui.Begin("Weapon Editor", editorOpen)
		if not editorOpen then -- if the window is closed, reset the state
			resetState = true
		end

		-- Display the weapon name
		imgui.Text("Editing:         "..self.weaponEditorUI.editingWeapon)
		imgui.Separator()

		-- Display the weapon stats
		if imgui.BeginChild("Weapon Stats", imgui.imVec2(0.0, 550.0)) then
			local modified = false
			local value = nil

			-- Sprite
			do
				value = self.weaponEditorUI.editedWeaponTable.sprite or ""

				imgui.Separator("Sprite")
				imgui.Text("Desc:       ")
				imgui.SameLine()
				imgui.TextWrapped("Name of an image located in the texture folder, including extension.")
				imgui.Text("Equation:   ")
				imgui.SameLine()
				imgui.TextWrapped("_")
				imgui.Text("Range:      ")
				imgui.SameLine()
				imgui.TextWrapped("_")
				imgui.Text("Average:    ")
				imgui.SameLine()
				imgui.TextWrapped("_")
				imgui.Text("Value:      ")
				imgui.SameLine()
				value, modified = imgui.InputText("##WeaponSpriteInput", value, 64)
				imgui.Text("")

				if modified then
					if value == "" then
						self.weaponEditorUI.editedWeaponTable.sprite = nil
					else
						self.weaponEditorUI.editedWeaponTable.sprite = value
					end
				end
			end
		
			-- Size
			do
				value = imgui.imVec2(
					self.weaponEditorUI.editedWeaponTable.width, 
					self.weaponEditorUI.editedWeaponTable.length
				)

				imgui.Separator("Size")
				imgui.Text("Desc:       ")
				imgui.SameLine()
				imgui.TextWrapped("Size of the weapon in pixesl. Width, then length.")
				imgui.Text("Equation:   ")
				imgui.SameLine()
				imgui.TextWrapped("_")
				imgui.Text("Range:      ")
				imgui.SameLine()
				imgui.TextWrapped("[(1, 1), (inf, inf)]")
				imgui.Text("Average:    ")
				imgui.SameLine()
				imgui.TextWrapped("(12, 28)")
				imgui.Text("Value:      ")
				imgui.SameLine()
				value, modified = imgui.InputFloat2("##WeaponSizeInput", value)
				imgui.Text("")

				if modified then
					self.weaponEditorUI.editedWeaponTable.width = value.x
					self.weaponEditorUI.editedWeaponTable.length = value.y
				end
			end

			-- Hand count
			do
				value = self.weaponEditorUI.editedWeaponTable.stats.handCount

				imgui.Separator("Hand Count")
				imgui.Text("Desc:       ")
				imgui.SameLine()
				imgui.TextWrapped("Whether the weapon is one-handed or two-handed.")
				imgui.Text("Equation:   ")
				imgui.SameLine()
				imgui.TextWrapped("_")
				imgui.Text("Range:      ")
				imgui.SameLine()
				imgui.TextWrapped("[1, 2]")
				imgui.Text("Average:    ")
				imgui.SameLine()
				imgui.TextWrapped("2")
				imgui.Text("Value:      ")
				imgui.SameLine()
				value, modified = imgui.SliderInt("##WeaponHandCountInput", value, 1, 2)
				imgui.Text("")

				if modified then
					self.weaponEditorUI.editedWeaponTable.stats.handCount = value
				end
			end

			-- Caliber
			do
				-- TODO: dropdown
				value = self.weaponEditorUI.editedWeaponTable.stats.caliber

				imgui.Separator("Caliber")
				imgui.Text("Desc:       ")
				imgui.SameLine()
				imgui.TextWrapped("Name of the weapons caliber. Defines what ammo it uses.")
				imgui.Text("Equation:   ")
				imgui.SameLine()
				imgui.TextWrapped("_")
				imgui.Text("Range:      ")
				imgui.SameLine()
				imgui.TextWrapped("_")
				imgui.Text("Average:    ")
				imgui.SameLine()
				imgui.TextWrapped("9mm")
				imgui.Text("Value:      ")
				imgui.SameLine()
				value, modified = imgui.InputText("##WeaponCaliberInput", value, 64)
				imgui.Text("")

				if modified then
					self.weaponEditorUI.editedWeaponTable.stats.caliber = value
				end
			end

			-- Fire mode
			do
				-- TODO: dropdown
				value = self.weaponEditorUI.editedWeaponTable.stats.fireMode

				imgui.Separator("Fire Mode")
				imgui.Text("Desc:       ")
				imgui.SameLine()
				imgui.TextWrapped("Whether the weapon is semi-automatic or automatic.")
				imgui.Text("Equation:   ")
				imgui.SameLine()
				imgui.TextWrapped("_")
				imgui.Text("Range:      ")
				imgui.SameLine()
				imgui.TextWrapped("[Semi, Auto]")
				imgui.Text("Average:    ")
				imgui.SameLine()
				imgui.TextWrapped("Semi")
				imgui.Text("Value:      ")
				imgui.SameLine()
				value, modified = imgui.InputText("##WeaponFireModeInput", value, 64)
				imgui.Text("")

				if modified then
					self.weaponEditorUI.editedWeaponTable.stats.fireMode = value
				end
			end

			-- Capacity
			do
				local value = self.weaponEditorUI.editedWeaponTable.stats.capacity

				imgui.Separator("Capacity")
				imgui.Text("Desc:       ")
				imgui.SameLine()
				imgui.TextWrapped("Magazine size of the weapon.")
				imgui.Text("Equation:   ")
				imgui.SameLine()
				imgui.TextWrapped("_")
				imgui.Text("Range:      ")
				imgui.SameLine()
				imgui.TextWrapped("[0, inf)")
				imgui.Text("Average:    ")
				imgui.SameLine()
				imgui.TextWrapped("15")
				imgui.Text("Value:      ")
				imgui.SameLine()
				value, modified = imgui.DragInt("##WeaponCapacityInput", value, 0.05, 1)
				imgui.Text("")

				if modified then
					self.weaponEditorUI.editedWeaponTable.stats.capacity = value
				end
			end

			-- Damage
			do
				local value = self.weaponEditorUI.editedWeaponTable.stats.damage

				imgui.Separator("Damage")
				imgui.Text("Desc:       ")
				imgui.SameLine()
				imgui.TextWrapped("Base damage at the moment of firing, before accounting for ammo type.")
				imgui.Text("Equation:   ")
				imgui.SameLine()
				imgui.TextWrapped("damage = damage * ammoDamageMult")
				imgui.Text("Range:      ")
				imgui.SameLine()
				imgui.TextWrapped("[0, inf)")
				imgui.Text("Average:    ")
				imgui.SameLine()
				imgui.TextWrapped("15.0")
				imgui.Text("Value:      ")
				imgui.SameLine()
				value, modified = imgui.DragFloat("##WeaponDamageInput", value, 0.02)
				imgui.Text("")

				if modified then
					self.weaponEditorUI.editedWeaponTable.stats.damage = value
				end
			end

			-- Fire rate
			do
				local value = self.weaponEditorUI.editedWeaponTable.stats.fireRate

				imgui.Separator("Fire Rate")
				imgui.Text("Desc:       ")
				imgui.SameLine()
				imgui.TextWrapped("Rate of fire in bullets per second.")
				imgui.Text("Equation:   ")
				imgui.SameLine()
				imgui.TextWrapped("cooldown = 1.0 / fireRate")
				imgui.Text("Range:      ")
				imgui.SameLine()
				imgui.TextWrapped("[0, inf)")
				imgui.Text("Average:    ")
				imgui.SameLine()
				imgui.TextWrapped("3.0")
				imgui.Text("Value:      ")
				imgui.SameLine()
				value, modified = imgui.DragFloat("##WeaponFireRateInput", value, 0.02, 0.00001)
				imgui.Text("")

				if modified then
					self.weaponEditorUI.editedWeaponTable.stats.fireRate = value
				end
			end

			-- Reload time
			do
				local value = self.weaponEditorUI.editedWeaponTable.stats.reloadTime

				imgui.Separator("Reload Time")
				imgui.Text("Desc:       ")
				imgui.SameLine()
				imgui.TextWrapped("Time needed to reload the weapon.")
				imgui.Text("Equation:   ")
				imgui.SameLine()
				imgui.TextWrapped("_")
				imgui.Text("Range:      ")
				imgui.SameLine()
				imgui.TextWrapped("[0, inf)")
				imgui.Text("Average:    ")
				imgui.SameLine()
				imgui.TextWrapped("2.0")
				imgui.Text("Value:      ")
				imgui.SameLine()
				value, modified = imgui.DragFloat("##WeaponReloadTimeInput", value, 0.02, 0.00001)
				imgui.Text("")

				if modified then
					self.weaponEditorUI.editedWeaponTable.stats.reloadTime = value
				end
			end

			-- Spread
			do
				local value = self.weaponEditorUI.editedWeaponTable.stats.spread

				imgui.Separator("Spread")
				imgui.Text("Desc:       ")
				imgui.SameLine()
				imgui.TextWrapped("The base accuracy of the weapon in degrees, before accounting for ammo type.")
				imgui.Text("Equation:   ")
				imgui.SameLine()
				imgui.TextWrapped("totalSpread = math.max(0.0, spread + ammoSpread)")
				imgui.Text("Range:      ")
				imgui.SameLine()
				imgui.TextWrapped("(-inf, inf)")
				imgui.Text("Average:    ")
				imgui.SameLine()
				imgui.TextWrapped("0.0")
				imgui.Text("Value:      ")
				imgui.SameLine()
				value, modified = imgui.DragFloat("##WeaponSpreadInput", value, 0.02)
				imgui.Text("")

				if modified then
					self.weaponEditorUI.editedWeaponTable.stats.spread = value
				end
			end

			-- Recoil
			do
				local value = self.weaponEditorUI.editedWeaponTable.stats.recoil

				imgui.Separator("Recoil")
				imgui.Text("Desc:       ")
				imgui.SameLine()
				imgui.TextWrapped("Cumulative drift to the aiming direction after firing, decaying over time.")
				imgui.Text("Equation:   ")
				imgui.SameLine()
				imgui.TextWrapped("currentRecoil = math.max(0.0, currentRecoil + recoil + ammoRecoil)")
				imgui.Text("Range:      ")
				imgui.SameLine()
				imgui.TextWrapped("(-inf, inf)")
				imgui.Text("Average:    ")
				imgui.SameLine()
				imgui.TextWrapped("0.0")
				imgui.Text("Value:      ")
				imgui.SameLine()
				value, modified = imgui.DragFloat("##WeaponRecoilInput", value, 0.02)
				imgui.Text("")

				if modified then
					self.weaponEditorUI.editedWeaponTable.stats.recoil = value
				end
			end

			-- Recovery
			do
				local value = self.weaponEditorUI.editedWeaponTable.stats.recovery
			
				imgui.Separator("Recovery")
				imgui.Text("Desc:       ")
				imgui.SameLine()
				imgui.TextWrapped("How fast the recoil decays, higher values means faster recovery.")
				imgui.Text("Equation:   ")
				imgui.SameLine()
				imgui.TextWrapped("See gameMath.expDecay(), recovery is passed as the d parameter")
				imgui.Text("Range:      ")
				imgui.SameLine()
				imgui.TextWrapped("[1, inf)")
				imgui.Text("Average:    ")
				imgui.SameLine()
				imgui.TextWrapped("5")
				imgui.Text("Value:      ")
				imgui.SameLine()
				value, modified = imgui.SliderInt("##WeaponRecoveryInput", value, 0, 24)
				imgui.Text("")

				if modified then
					self.weaponEditorUI.editedWeaponTable.stats.recovery = value
				end
			end
		end
		imgui.EndChild()
		imgui.Separator()

		-- Confirm button
		if imgui.Button("Confirm##ConfirmWeaponButton") then
			resetState = true
			data.weapons[self.weaponEditorUI.editingWeapon] = self.weaponEditorUI.editedWeaponTable

			local err = data.modding.createLuaTableSave(
				"src/Mods/Weapons/", 
				"weapons",
				self.weaponEditorUI.editingWeapon,
				self.weaponEditorUI.editedWeaponTable
			)

			if err then
				print("Error saving weapon: "..err)
			else
				print("Weapon saved successfully.")
			end
		end

		-- Cancel button
		if imgui.Button("Cancel##CancelWeaponButton") then
			resetState = true
		end

		-- Delete button
		if imgui.Button("Delete##DeleteWeaponButton") then
			resetState = true
			data.weapons[self.weaponEditorUI.editingWeapon] = nil
		end

		if resetState then
			self.weaponEditorUI.editedWeaponTable = nil
			self.weaponEditorUI.editingWeapon = nil
		end

		imgui.End()
	end
	tracy.ZoneEnd()
end

function presetCreatorUI:AmmoEditorUI()
	tracy.ZoneBeginN("Lua presetCreatorUI:AmmoEditorUI")
	
	
	local ctx = self.ammoEditorUI

	-- List all calibers in data.ammo.calibers as buttons
	for caliberName, _ in pairs(data.ammo.calibers) do
		if imgui.Button(caliberName) then
			self.ammoEditorUI.editingCaliber = caliberName
			self.ammoEditorUI.editedCaliberTable = nil
		end

		if self.ammoEditorUI.editingCaliber == caliberName then
			imgui.SameLine()
			imgui.Text("(Selected)")
		end
	end

	-- Add a button for creating a new caliber
	if imgui.Button("New Caliber##NewCaliberButton") then
		ctx.newCaliberName = ""
		imgui.OpenPopup("New Caliber Name##NameCaliberPopup")
	end
	
	-- Open a popup to name the caliber
	if imgui.BeginPopup("New Caliber Name##NameCaliberPopup") then

		imgui.Text("Enter a name for the new caliber:")
		ctx.newCaliberName = imgui.InputText("##NewCaliberNameInput", ctx.newCaliberName, 64)
			
		if imgui.Button("Confirm") or Input.KeyPressed(Input.Key.KEY_ENTER) then -- Submit if Confirm button or Enter key is pressed
			local newCaliber = { 
				[ctx.newCaliberName] = {
					default = ""
				}
			}

			-- Insert the caliber
			data.modding.loadAmmoMod(newCaliber)

			-- Set the new caliber as the editing caliber
			ctx.editingCaliber = ctx.newCaliberName
			ctx.editedCaliberTable = nil
			ctx.newCaliberName = nil
			imgui.CloseCurrentPopup()
		end

		imgui.EndPopup()
	end


	if ctx.editingCaliber ~= nil then
		-- Before editing, copy the original caliber table to perform the edits on
		if ctx.editedCaliberTable == nil then
			ctx.editedCaliberTable = table.deepCopy(
				data.ammo.calibers[ctx.editingCaliber]
			)
		end

		local editorOpen = true
		local resetState = false

		-- Open the editor window
		editorOpen = imgui.Begin("Caliber Editor", editorOpen)
		if not editorOpen then -- if the window is closed, reset the state
			resetState = true
		end

		-- Display the caliber name
		imgui.Text("Editing:         "..ctx.editingCaliber)
		imgui.Separator()



		-- TODO: List all ammo types as collapsing headers, containing the ammo type stats
		for ammoTypeName, ammoTypeData in pairs(ctx.editedCaliberTable) do
			if type(ammoTypeData) == "table" then
				if imgui.CollapsingHeader(ammoTypeName.."##EditAmmoType"..ammoTypeName.."Header") then
					
					imgui.Text(table.toString(ammoTypeData))
					
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
				end
			end
		end


		-- Button to add a new ammo type
		if imgui.Button("Add Type##AddNewAmmoTypeButton") then
			imgui.OpenPopup("Enter Name##AddNewAmmoTypePopup")
		end

		-- Input the new ammo type's name
		if imgui.BeginPopup("Enter Name##AddNewAmmoTypePopup") then
			imgui.TextWrapped("Enter a name for the new ammo type:")

			ctx.newAmmoTypeName = imgui.InputText("##NewAmmoTypeNameInput", ctx.newAmmoTypeName, 64)
			
			local validAmmoTypeName = true

			for existingAmmoTypeName, _ in pairs(ctx.editedCaliberTable) do
				if existingAmmoTypeName == ctx.newAmmoTypeName then
					validAmmoTypeName = false
					break
				end
			end

			if validAmmoTypeName then
				if imgui.Button("Confirm") or Input.KeyPressed(Input.Key.KEY_ENTER) then -- Submit if Confirm button or Enter key is pressed
					local newAmmoType = { 
						damageMult = 1.0,
						falloff = 0.95,
						penetration = 0.5,
						spread = 3.0,
						recoil = 5.0,
						burstSize = 1
					}

					-- Insert the ammo type
					ctx.editedCaliberTable[ctx.newAmmoTypeName] = newAmmoType

					ctx.newAmmoTypeName = ""
					imgui.CloseCurrentPopup()
				end
			end

			imgui.EndPopup()
		end

		imgui.Separator()

		
		-- Add a button for selecting the calibers default ammo type
		if imgui.Button("Select Default Type##SelectDefaultAmmoTypeButton") then
			imgui.OpenPopup("Select Default Type##SelectDefaultAmmoTypePopup")
		end

		-- Select the default ammo type before submitting
		if imgui.BeginPopup("Select Default Type##SelectDefaultAmmoTypePopup") then
			imgui.TextWrapped("Select the default ammo type for this caliber:")

			local iter = 0
			local ammoTypeList = {}
			for ammoTypeName, ammoTypeData in pairs(ctx.editedCaliberTable) do
				if ammoTypeName ~= "default" then -- Avoid the "default" string that's also in the table
					table.insert(ammoTypeList, ammoTypeName)

					-- Use this opportunity to find the index of the default type if one is already selected
					if ammoTypeName == ctx.editedCaliberTable.default then
						ctx.selectedAmmoType = iter
					end
					iter = iter + 1
				end
			end

			local ammoTypeListString = table.concat(ammoTypeList, "\n").."\n\n"
			local pressed = false
			pressed, ctx.selectedAmmoType = imgui.Combo("Default##PrefabEditorDefaultType", ctx.selectedAmmoType, ammoTypeListString)

			if pressed then
				ctx.editedCaliberTable.default = ammoTypeList[ctx.selectedAmmoType + 1]
				imgui.CloseCurrentPopup()
			end
			imgui.EndPopup()
		end
		
		
		if table.len(ctx.editedCaliberTable) > 1 then -- Must be greater than one because of "default" element
			-- Confirm button
			if imgui.Button("Confirm##ConfirmCaliberButton") then
				if ctx.editedCaliberTable[ctx.editedCaliberTable.default] == nil then
					imgui.OpenPopup("Select Default Type##SelectDefaultAmmoTypePopup")
				end

				ctx.saveQueued = true
			end

			-- Save the caliber if prerequisites are met
			if ctx.saveQueued == true and ctx.editedCaliberTable[ctx.editedCaliberTable.default] ~= nil then
				resetState = true
				data.ammo.calibers[ctx.editingCaliber] = ctx.editedCaliberTable

				local err = data.modding.createLuaTableSave(
					"src/Mods/Ammo/", 
					"ammo.calibers",
					ctx.editingCaliber,
					ctx.editedCaliberTable
				)

				if err then
					print("Error saving caliber: "..err)
				else
					print("Caliber saved successfully.")
				end
			end
		else
			imgui.TextWrapped("Caliber preset cannot be saved: Caliber must have at least one ammo type!")
		end

		-- Cancel button
		if imgui.Button("Cancel##CancelCaliberButton") then
			resetState = true
		end

		-- Delete button
		if imgui.Button("Delete##DeleteCaliberButton") then
			resetState = true
			data.ammo.calibers[ctx.editingCaliber] = nil
		end


		if resetState then
			ctx.editedCaliberTable = nil
			ctx.editingCaliber = nil
			ctx.saveQueued = false
		end

		imgui.End()
	end

	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return presetCreatorUI
