tracy.ZoneBeginN("Lua WeaponEditorUI.lua")

if not dev then
	dev = {}
	dev.editingWeapon = nil -- string
	dev.newWeaponName = nil -- string
	dev.editedWeaponTable = nil -- table
end

local function WeaponEditorUI()
	if not dev.editingWeapon then
		imgui.Begin("Weapon Editor")

		-- List all weapons in data.weapons as buttons
		for weaponName, _ in pairs(data.weapons) do
			if imgui.Button(weaponName) then
				dev.editingWeapon = weaponName
			end
		end

		-- Add a button for creating a new weapon
		if imgui.Button("New Weapon##NewWeaponButton") then
			dev.newWeaponName = ""
			imgui.OpenPopup("New Weapon Name##NameWeaponPopup")
		end

		-- Open a popup to name the weapon
		if imgui.BeginPopup("New Weapon Name##NameWeaponPopup") then

			imgui.Text("Enter a name for the new weapon:")
			dev.newWeaponName = imgui.InputText("##NewWeaponNameInput", dev.newWeaponName, 64)
			
			if imgui.Button("Confirm Name") then
				local newWeapon = {
					[dev.newWeaponName] = {

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
				dev.editingWeapon = dev.newWeaponName
				dev.newWeaponName = nil
				imgui.CloseCurrentPopup()
			end

			imgui.EndPopup()
		end

		imgui.End()
	else
		-- Before editing, copy the original weapon table to perform the edits on
		if not dev.editedWeaponTable then
			dev.editedWeaponTable = table.deepCopy(
				data.weapons[dev.editingWeapon]
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
		imgui.Text("Editing: "..dev.editingWeapon)

		if imgui.BeginChild("Weapon Stats", imgui.imVec2(0.0, 512.0)) then
			local modified = false
			local value = nil

			-- Sprite
			value = dev.editedWeaponTable.sprite or ""

			imgui.Text("Sprite: ")
			imgui.SameLine()
			value, modified = imgui.InputText("##WeaponSpriteInput", value, 64)

			if modified then
				if value == "" then
					dev.editedWeaponTable.sprite = nil
				else
					dev.editedWeaponTable.sprite = value
				end
			end
		
			-- Size
			value = imgui.imVec2(
				dev.editedWeaponTable.width, 
				dev.editedWeaponTable.length
			)

			imgui.Text("Size: ")
			imgui.SameLine()
			value, modified = imgui.InputFloat2("##WeaponSizeInput", value)

			if modified then
				dev.editedWeaponTable.width = value.x
				dev.editedWeaponTable.length = value.y
			end

			-- Hand count
			value = dev.editedWeaponTable.stats.handCount

			imgui.Text("Hand Count: ")
			imgui.SameLine()
			value, modified = imgui.SliderInt("##WeaponHandCountInput", value, 1, 2)

			if modified then
				dev.editedWeaponTable.stats.handCount = value
			end

			-- Caliber
			-- TODO: dropdown
			value = dev.editedWeaponTable.stats.caliber

			imgui.Text("Caliber: ")
			imgui.SameLine()
			value, modified = imgui.InputText("##WeaponCaliberInput", value, 64)

			if modified then
				dev.editedWeaponTable.stats.caliber = value
			end

			-- Fire mode
			-- TODO: dropdown
			value = dev.editedWeaponTable.stats.fireMode

			imgui.Text("Fire Mode: ")
			imgui.SameLine()
			value, modified = imgui.InputText("##WeaponFireModeInput", value, 64)

			if modified then
				dev.editedWeaponTable.stats.fireMode = value
			end

			-- Capacity
			local value = dev.editedWeaponTable.stats.capacity

			imgui.Text("Capacity: ")
			imgui.SameLine()
			value, modified = imgui.DragInt("##WeaponCapacityInput", value, 0.05, 1)

			if modified then
				dev.editedWeaponTable.stats.capacity = value
			end

			-- Damage
			local value = dev.editedWeaponTable.stats.damage

			imgui.Text("Damage: ")
			imgui.SameLine()
			value, modified = imgui.DragFloat("##WeaponDamageInput", value, 0.02)

			if modified then
				dev.editedWeaponTable.stats.damage = value
			end

			-- Fire rate
			local value = dev.editedWeaponTable.stats.fireRate

			imgui.Text("Fire Rate: ")
			imgui.SameLine()
			value, modified = imgui.DragFloat("##WeaponFireRateInput", value, 0.02, 0.00001)

			if modified then
				dev.editedWeaponTable.stats.fireRate = value
			end

			-- Reload time
			local value = dev.editedWeaponTable.stats.reloadTime

			imgui.Text("Reload Time: ")
			imgui.SameLine()
			value, modified = imgui.DragFloat("##WeaponReloadTimeInput", value, 0.02, 0.00001)

			if modified then
				dev.editedWeaponTable.stats.reloadTime = value
			end

			-- Spread
			local value = dev.editedWeaponTable.stats.spread

			imgui.Text("Spread: ")
			imgui.SameLine()
			value, modified = imgui.DragFloat("##WeaponSpreadInput", value, 0.02)

			if modified then
				dev.editedWeaponTable.stats.spread = value
			end

			-- Recoil
			local value = dev.editedWeaponTable.stats.recoil

			imgui.Text("Recoil: ")
			imgui.SameLine()
			value, modified = imgui.DragFloat("##WeaponRecoilInput", value, 0.02)

			if modified then
				dev.editedWeaponTable.stats.recoil = value
			end

			-- Recovery
			local value = dev.editedWeaponTable.stats.recovery

			imgui.Text("Recovery: ")
			imgui.SameLine()
			value, modified = imgui.SliderInt("##WeaponRecoveryInput", value, 0, 24)

			if modified then
				dev.editedWeaponTable.stats.recovery = value
			end
		end
		imgui.EndChild()


		-- Confirm button
		if imgui.Button("Confirm##ConfirmWeaponButton") then
			resetState = true

			if true then
				data.weapons[dev.editingWeapon] = dev.editedWeaponTable

				-- Save the weapon to a file
				local saveTable = {
					dataPath	= "weapons",					-- Location of this table in the data table
					elementName = dev.editingWeapon,		-- Name of the element in the dataPath
					contents	= dev.editedWeaponTable	-- The table stored at data.dataPath[elementName]
				}

				local err = table.save(saveTable, "src/Mods/Weapons/"..saveTable.elementName..".lts") -- lts: Lua Table Save

				if err then
					print("Error saving weapon: "..err)
				else
					print("Weapon saved successfully.")
				end
			end

			-- Save all ammo calibers to files
			for key, value in pairs(data.ammo.calibers) do

				-- Save the ammo to a file
				local saveTable = {
					dataPath	= "ammo.calibers",
					elementName = key,
					contents	= value
				}

				local err = table.save(saveTable, "src/Mods/Ammo/"..saveTable.elementName..".lts")

				if err then
					print("Error saving ammo: "..err)
				else
					print("Ammo saved successfully.")
				end
			end
		end

		-- Cancel button
		if imgui.Button("Cancel##CancelWeaponButton") then
			resetState = true
		end

		-- Delete button
		if imgui.Button("Delete##DeleteWeaponButton") then
			resetState = true
			data.weapons[dev.editingWeapon] = nil
		end

		if resetState then
			dev.editedWeaponTable = nil
			dev.editingWeapon = nil
		end

		imgui.End()
	end
end

WeaponEditorUI()
tracy.ZoneEnd()