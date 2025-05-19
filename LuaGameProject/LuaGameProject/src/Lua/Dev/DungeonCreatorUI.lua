tracy.ZoneBeginN("Lua SandboxUI.lua")

local vec2 = require("Vec2")
local transform = require("Transform2")

local dungeonCreatorUI = {
	roomCollection = {
		roomCount = 0,
		selectedRoom = -1,
		rooms = {
		-- 	0 = {
		--		groupName = "",
		-- 		size = vec2(500, 500),
		-- 	}
		}
	}
}

function dungeonCreatorUI:PrefabCollection()
	tracy.ZoneBeginN("Lua dungeonCreatorUI:PrefabCollection")

	imgui.Begin("Prefab Collection")

	for name, _ in pairs(data.prefabs) do
		if imgui.Button(name) and self.roomCollection.selectedRoom ~= -1 then
			if ~game.SpawnPrefab(name) then
				print("Could not instanciate prefab")
			end
		end
	end

	imgui.End()

	tracy.ZoneEnd()
end

local buffer = ""
local done = false

function dungeonCreatorUI:RoomSelection()
	imgui.Begin("Room Selection")

	imgui.Text("Current room settings")

	if imgui.Button("Save All Rooms") then
		for i, room in ipairs(self.roomCollection.rooms) do
			local err = data.modding.createLuaTableSave("src/Mods/Rooms/", "rooms", room.groupName, room)

			if err then
				print("Error saving prefab: "..err)
			end
		end
	end

	-- Set Bounds
	if self.roomCollection.selectedRoom ~= -1 then
		if imgui.Button("Save") then
			game.CreateGroupFromScene(self.roomCollection.rooms[self.roomCollection.selectedRoom].groupName)
		end

		local trans = transform(scene.GetComponent(RoomBounds, "Transform"))

		local value = imgui.imVec2(self.roomCollection.rooms[self.roomCollection.selectedRoom].size.x,
								   self.roomCollection.rooms[self.roomCollection.selectedRoom].size.y)

		value, _ = imgui.DragFloat2("Room Size", value)

		self.roomCollection.rooms[self.roomCollection.selectedRoom].size.x = value.x
		self.roomCollection.rooms[self.roomCollection.selectedRoom].size.y = value.y

		trans.scale = self.roomCollection.rooms[self.roomCollection.selectedRoom].size
		scene.SetComponent(RoomBounds, "Transform", trans)
	end

    imgui.Separator();

	-- Room Creator
	if imgui.BeginPopupContextItem("RoomPopup") then

		-- TODO: Add flag for enter option
		buffer, done = imgui.InputText("Enter name", buffer, 16)
		if imgui.Button("Done") and buffer ~= "" then
			self.roomCollection.roomCount = self.roomCollection.roomCount + 1
			self.roomCollection.rooms[self.roomCollection.roomCount] = {
				groupName = buffer,
				size = vec2(500, 500),
			}

			buffer = ""
			imgui.CloseCurrentPopup()
		end

		imgui.SameLine()

		if imgui.Button("Cancel") then
			buffer = ""
			imgui.CloseCurrentPopup()
		end

		imgui.EndPopup()
	end

	if imgui.Button("Create new room") then
		imgui.OpenPopup("RoomPopup")
	end

	-- Room Display
	for i, room in ipairs(self.roomCollection.rooms) do
		if imgui.Selectable(room.groupName, self.roomCollection.selectedRoom == i) and self.roomCollection.selectedRoom ~= i then
			-- Save current scene
			if self.roomCollection.selectedRoom ~= -1 then
				game.CreateGroupFromScene(self.roomCollection.rooms[self.roomCollection.selectedRoom].groupName, true)
			end

			-- Reset scene
			self.roomCollection.selectedRoom = i
			scene.Clear()

			-- Load new room
			game.SpawnGroup(self.roomCollection.rooms[self.roomCollection.selectedRoom].groupName)
		end
	end

	imgui.End()
end

tracy.ZoneEnd()
return dungeonCreatorUI
