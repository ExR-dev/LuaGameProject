tracy.ZoneBeginN("Lua SandboxUI.lua")

local vec2 = require("Vec2")
local transform = require("Transform2")

local dungeonCreatorUI = {
	prefabCollection = {
		prefabsCount = 0,
		roomPrefabs = {}
	},
	roomCollection = {
		roomCount = 0,
		selectedRoom = "",
		rooms = {}
	}
}


function dungeonCreatorUI:PrefabCollection()
	tracy.ZoneBeginN("Lua dungeonCreatorUI:PrefabCollection")

	imgui.Begin("Prefab Collection")

	for name, _ in pairs(data.prefabs) do
		if imgui.Button(name) then
			if game.SpawnPrefab(name) then
				self.prefabCollection.prefabsCount = self.prefabCollection.prefabsCount + 1
				self.prefabCollection.roomPrefabs[self.prefabCollection.prefabsCount] = name
			else
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
	imgui.Begin("Room Selection (new)")

	-- Room Creator
	if (imgui.BeginPopupContextItem("RoomPopup")) then

		-- TODO: Add flag for enter option
		buffer, done = imgui.InputText("Enter name", buffer, 16)
		if (imgui.Button("Done") and buffer ~= "") then
			self.roomCollection.roomCount = self.roomCollection.roomCount + 1
			self.roomCollection.rooms[self.roomCollection.roomCount] = buffer

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

	for i, name in ipairs(self.roomCollection.rooms) do
		if imgui.Selectable(name, self.roomCollection.selectedRoom == name) then
			self.roomCollection.selectedRoom = name
			-- TODO: Save current room
			-- TODO: Clear scene
			-- TODO: Load new room
		end
	end

	imgui.End()
end

tracy.ZoneEnd()
return dungeonCreatorUI
