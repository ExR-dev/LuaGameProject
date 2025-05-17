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
		selectedRoom = -1,
		rooms = {
		-- 	0 = {
		--		name = ""
		--  	prefabsCount = 0,
		--     	roomPrefabs = {}
		-- 	}
		}
	}
}


function dungeonCreatorUI:PrefabCollection()
	tracy.ZoneBeginN("Lua dungeonCreatorUI:PrefabCollection")

	imgui.Begin("Prefab Collection")

	for name, _ in pairs(data.prefabs) do
		if imgui.Button(name) and self.roomCollection.selectedRoom ~= -1 then
			if game.SpawnPrefab(name) then
				local count = self.roomCollection.rooms[self.roomCollection.selectedRoom].prefabsCount + 1
				self.roomCollection.rooms[self.roomCollection.selectedRoom].prefabsCount = count
				self.roomCollection.rooms[self.roomCollection.selectedRoom].roomPrefabs[count] = name
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
	imgui.Begin("Room Selection")

	if imgui.Button("Save") then
		game.CreateGroupFromScene(self.roomCollection.rooms[self.roomCollection.selectedRoom].name)
	end

	-- Room Creator
	if imgui.BeginPopupContextItem("RoomPopup") then

		-- TODO: Add flag for enter option
		buffer, done = imgui.InputText("Enter name", buffer, 16)
		if imgui.Button("Done") and buffer ~= "" then
			self.roomCollection.roomCount = self.roomCollection.roomCount + 1
			self.roomCollection.rooms[self.roomCollection.roomCount] = {
				name = buffer,
				prefabsCount = 0,
				roomPrefabs = {}
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
		if imgui.Selectable(room.name, self.roomCollection.selectedRoom == i) and self.roomCollection.selectedRoom ~= i then
			-- Save current scene
			if self.roomCollection.selectedRoom ~= -1 then
				game.CreateGroupFromScene(self.roomCollection.rooms[self.roomCollection.selectedRoom].name)
			end

			-- Reset scene
			self.roomCollection.selectedRoom = i
			scene.Clear()

			-- Load new room
			game.SpawnGroup(self.roomCollection.rooms[self.roomCollection.selectedRoom].name)
		end
	end

	imgui.End()
end

tracy.ZoneEnd()
return dungeonCreatorUI
