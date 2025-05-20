tracy.ZoneBeginN("Lua SandboxUI.lua")

local gameMath = require("Utility/GameMath")
local vec2 = require("Vec2")
local transform = require("Transform2")
local color = require("Color")
local sprite = require("Components/Sprite")

local dungeonCreatorUI = {
	roomCollection = {
		selectedRoom = nil,
		rooms = {
		--	[groupName] = {
		--		size = vec2(500, 500),
		--	}
		}
	}
}

dungeonCreatorUI.roomCollection.rooms = data.rooms or {}

function dungeonCreatorUI:PrefabCollection()
	tracy.ZoneBeginN("Lua dungeonCreatorUI:PrefabCollection")

	imgui.Begin("Prefab Collection")

	for name, _ in pairs(data.prefabs) do
		if imgui.Button(name) and self.roomCollection.selectedRoom ~= nil then
			if ~game.SpawnPrefab(name) then
				print("Could not instanciate prefab")
			end
		end
	end

	imgui.End()

	tracy.ZoneEnd()
end

local function SaveRoom(name, room)
	if name == nil then
		return
	end

	local err = data.modding.createLuaTableSave("src/Mods/Rooms/", "rooms", name, room)
	if err then
		print("Error saving room: "..err)
	end

	-- Saving group
	err = data.modding.createLuaTableSave("src/Mods/Groups/", "groups", name, data.groups[name])
	if err then
		print("Error saving group: "..err)
	end

end

local buffer = ""

function dungeonCreatorUI:RoomSelection()
	imgui.Begin("Room Selection")

	if imgui.Button("Save All Rooms") then
		for name, room in pairs(self.roomCollection.rooms) do
			SaveRoom(name, room)
		end
	end

	imgui.SameLine()

	if imgui.Button("Refresh") then
		for name, room in pairs(data.rooms) do
			self.roomCollection.rooms[name] = room
		end
	end

	imgui.Separator("Current room settings")

	-- Set Bounds
	if self.roomCollection.selectedRoom ~= nil then
		if imgui.Button("Save") then
			local selected = self.roomCollection.selectedRoom

			-- Saving room
			game.CreateGroupFromScene(selected)

			SaveRoom(selected, self.roomCollection.rooms[selected])
		end

		if scene.IsEntity(RoomBounds) == false then
			RoomBounds = scene.CreateEntity()

			local boundsS = sprite("", color(237/255, 232/255, 150/255, 1), -100)
			local unitT = transform(
				vec2(0, 0), 
				0.0,
				vec2(gameMath.metersToPixels, gameMath.metersToPixels)
			)
			scene.SetComponent(RoomBounds, "Transform", unitT)
			scene.SetComponent(RoomBounds, "Sprite", boundsS)
			scene.SetComponent(RoomBounds, "Debug")
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

    imgui.Separator("Rooms");

	-- Room Creator
	if imgui.BeginPopupContextItem("RoomPopup") then

		-- TODO: Add flag for enter option
		buffer, done = imgui.InputText("Enter name", buffer, 16)
		if imgui.Button("Done") and buffer ~= "" then
			self.roomCollection.rooms[buffer] = {
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
	for name, room in pairs(self.roomCollection.rooms) do
		if imgui.Selectable(name, self.roomCollection.selectedRoom == name) and self.roomCollection.selectedRoom ~= name then
			local selected = self.roomCollection.selectedRoom
			if selected == name then
				return
			end

			-- Save current scene
			if selected ~= nil then
				game.CreateGroupFromScene(selected, true)
			end

			-- Reset scene
			scene.Clear()

			-- Load new room
			game.SpawnGroup(name)
			self.roomCollection.selectedRoom = name
		end
	end

	imgui.End()
end

local radius = 100
local selectedRooms = {}
function dungeonCreatorUI:GenerateDungeon()
	imgui.Text("Dungeon Generation")
	if imgui.Button("Generate") then
		scene.Clear()
		self.roomCollection.selectedRoom = nil

		dungeonGenerator.Reset()
		dungeonGenerator.Initialize(vec2(100, 0))
		for roomName, selected in pairs(selectedRooms) do
			if selected then
				for i=1, 10 do 
					dungeonGenerator.AddRoom({
						size = self.roomCollection.rooms[roomName].size,
						name = roomName
					})
				end
			end
		end
		dungeonGenerator.Generate(radius)
		dungeonGenerator.SeparateRooms()
	end

	imgui.SameLine()
	if imgui.Button("Save") then
		-- TODO: Save dungeon to file	
	end

	imgui.Separator("Settings")

	radius, _ = imgui.DragFloat("Radius", radius, 0.1, 0.01, 10000.0)

	imgui.Separator("Room Selection")

	-- Display rooms
	for name, room in pairs(self.roomCollection.rooms) do
		if imgui.Selectable(name, selectedRooms[name] == true, imgui.ImGuiSelectableFlags.NoAutoClosePopups) then
			selectedRooms[name] = not selectedRooms[name]
		end
	end

	imgui.Separator("Debug Options")

	if imgui.Button("Spawn Rooms") then
		scene.Clear()

		dungeonGenerator.Reset()
		dungeonGenerator.Initialize(vec2(100, 100))
		for roomName, selected in pairs(selectedRooms) do
			if selected then
				for i=1, 10 do 
					dungeonGenerator.AddRoom({
						size = self.roomCollection.rooms[roomName].size,
						name = roomName
					})
				end
			end
		end
		dungeonGenerator.Generate(radius)
	end

	if imgui.Button("Separate Rooms") then
		dungeonGenerator.SeparateRooms()
	end

	if imgui.Button("Complete") then
		for i, room in pairs(dungeonGenerator.GetRooms()) do
			local t = transform(vec2(room.position), 0, vec2(1, 1))
			game.SpawnGroup(room.name, t)
		end
		dungeonGenerator.Reset();
	end

	imgui.Separator()

	if imgui.Button("Close") then
		imgui.CloseCurrentPopup()
	end

	imgui.SameLine()
	if imgui.Button("Reset") then
		dungeonGenerator.Reset()
	end
end

tracy.ZoneEnd()
return dungeonCreatorUI
