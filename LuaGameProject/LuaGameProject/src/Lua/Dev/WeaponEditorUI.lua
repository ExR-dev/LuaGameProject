tracy.ZoneBeginN("Lua WeaponEditorUI.lua")

if not data.dev then
	data.dev = {}
	data.dev.editingWeapon = nil -- string
	data.dev.newWeaponName = nil -- string
	data.dev.editedWeaponTable = nil -- table
end

local function WeaponEditorUI()
	if not data.dev.editingWeapon then
		imgui.Begin("Weapon Editor")

		-- List all weapons in data.weapons as buttons
		for weaponName, _ in pairs(data.weapons) do
			if imgui.Button(weaponName) then
				data.dev.editingWeapon = weaponName
			end
		end

		-- Add a button for creating a new weapon
		if imgui.Button("New Weapon##NewWeaponButton") then
			data.dev.newWeaponName = ""
		end

		-- Open a popup to name the weapon
		if data.dev.newWeaponName then
			if imgui.BeginPopup("New Weapon Name##NameWeaponPopup") then

				imgui.Text("Enter a name for the new weapon:")
				data.dev.newWeaponName = imgui.InputText("##NewWeaponNameInput", data.dev.newWeaponName, 64)
			
				if imgui.Button("Confirm Name") then
					local newWeapon = {
						[data.dev.newWeaponName] = {

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
					data.dev.editingWeapon = data.dev.newWeaponName
					data.dev.newWeaponName = nil
				end

				imgui.EndPopup()
			end
		end

		imgui.End()
	else
		-- Before editing, get the original weapon table to perform the edits on
		if not data.dev.editedWeaponTable then
			data.dev.editedWeaponTable = data.weapons[data.dev.editingWeapon]
		end

		local editorOpen = true
		local resetState = false

		-- Open the editor window
		editorOpen = imgui.Begin("Weapon Editor", editorOpen)
		if not editorOpen then -- if the window is closed, reset the state
			resetState = true
		end

		-- Display the weapon name
		imgui.Text("Editing: "..data.dev.editingWeapon)

		imgui.BeginChild("Weapon Stats", imgui.imVec2(0.0, 512.0))

		local modified = false
		local value = nil

		-- Sprite
		value = data.dev.editedWeaponTable.sprite or ""

		imgui.Text("Sprite: ")
		imgui.SameLine()
		value, modified = imgui.InputText("##WeaponSpriteInput", value, 64)

		if modified then
			if value == "" then
				data.dev.editedWeaponTable.sprite = nil
			else
				data.dev.editedWeaponTable.sprite = value
			end
		end
		
		-- Size
		value = imgui.imVec2(
			data.dev.editedWeaponTable.width, 
			data.dev.editedWeaponTable.length
		)

		imgui.Text("Size: ")
		imgui.SameLine()
		value, modified = imgui.InputFloat2("##WeaponSizeInput", value)

		if modified then
			data.dev.editedWeaponTable.width = value.x
			data.dev.editedWeaponTable.length = value.y
		end

		-- Hand count
		value = data.dev.editedWeaponTable.stats.handCount

		imgui.Text("Hand Count: ")
		imgui.SameLine()
		value, modified = imgui.SliderInt("##WeaponHandCountInput", value, 1, 2)

		if modified then
			data.dev.editedWeaponTable.stats.handCount = value
		end

		-- Caliber
		-- TODO: dropdown
		value = data.dev.editedWeaponTable.stats.caliber

		imgui.Text("Caliber: ")
		imgui.SameLine()
		value, modified = imgui.InputText("##WeaponCaliberInput", value, 64)

		if modified then
			data.dev.editedWeaponTable.stats.caliber = value
		end

		-- Fire mode
		-- TODO: dropdown
		value = data.dev.editedWeaponTable.stats.fireMode

		imgui.Text("Fire Mode: ")
		imgui.SameLine()
		value, modified = imgui.InputText("##WeaponFireModeInput", value, 64)

		if modified then
			data.dev.editedWeaponTable.stats.fireMode = value
		end

		-- Capacity
		local value = data.dev.editedWeaponTable.stats.capacity

		imgui.Text("Capacity: ")
		imgui.SameLine()
		value, modified = imgui.DragInt("##WeaponCapacityInput", value, 0.05, 1)

		if modified then
			data.dev.editedWeaponTable.stats.capacity = value
		end

		-- Damage
		local value = data.dev.editedWeaponTable.stats.damage

		imgui.Text("Damage: ")
		imgui.SameLine()
		value, modified = imgui.DragFloat("##WeaponDamageInput", value, 0.02)

		if modified then
			data.dev.editedWeaponTable.stats.damage = value
		end

		-- Fire rate
		local value = data.dev.editedWeaponTable.stats.fireRate

		imgui.Text("Fire Rate: ")
		imgui.SameLine()
		value, modified = imgui.DragFloat("##WeaponFireRateInput", value, 0.02, 0.00001)

		if modified then
			data.dev.editedWeaponTable.stats.fireRate = value
		end

		-- Reload time
		local value = data.dev.editedWeaponTable.stats.reloadTime

		imgui.Text("Reload Time: ")
		imgui.SameLine()
		value, modified = imgui.DragFloat("##WeaponReloadTimeInput", value, 0.02, 0.00001)

		if modified then
			data.dev.editedWeaponTable.stats.reloadTime = value
		end

		-- Spread
		local value = data.dev.editedWeaponTable.stats.spread

		imgui.Text("Spread: ")
		imgui.SameLine()
		value, modified = imgui.DragFloat("##WeaponSpreadInput", value, 0.02)

		if modified then
			data.dev.editedWeaponTable.stats.spread = value
		end

		-- Recoil
		local value = data.dev.editedWeaponTable.stats.recoil

		imgui.Text("Recoil: ")
		imgui.SameLine()
		value, modified = imgui.DragFloat("##WeaponRecoilInput", value, 0.02)

		if modified then
			data.dev.editedWeaponTable.stats.recoil = value
		end

		-- Recovery
		local value = data.dev.editedWeaponTable.stats.recovery

		imgui.Text("Recovery: ")
		imgui.SameLine()
		value, modified = imgui.SliderInt("##WeaponRecoveryInput", value, 0, 24)

		if modified then
			data.dev.editedWeaponTable.stats.recovery = value
		end

		imgui.EndChild()


		-- Confirm button
		if imgui.Button("Confirm##ConfirmWeaponButton") then
			data.weapons[data.dev.editingWeapon] = data.dev.editedWeaponTable
			resetState = true
		end

		-- Cancel button
		if imgui.Button("Cancel##CancelWeaponButton") then
			resetState = true
		end

		-- Delete button
		if imgui.Button("Delete##DeleteWeaponButton") then
			data.weapons[data.dev.editingWeapon] = nil
			resetState = true
		end

		if resetState then
			data.dev.editedWeaponTable = nil
			data.dev.editingWeapon = nil
		end

		imgui.End()
	end
end

WeaponEditorUI()
tracy.ZoneEnd()