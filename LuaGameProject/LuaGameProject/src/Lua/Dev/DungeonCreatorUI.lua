tracy.ZoneBeginN("Lua SandboxUI.lua")

local vec2 = require("Vec2")
local transform = require("Transform2")

local dungeonCreatorUI = {
	prefabCollection = {
		prefabsCount = 0,
		roomPrefabs = {}
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

	if (imgui.BeginPopupContextItem("RoomPopup")) then

		buffer, done = imgui.InputText("Enter name", buffer, 16)
		
		if (imgui.Button("Done")) then
			print(buffer)
			buffer = ""
			imgui.CloseCurrentPopup()
		end

		imgui.EndPopup()
	end

	if imgui.Button("Create new room") then
		imgui.OpenPopup("RoomPopup")
	end

	imgui.End()
end

tracy.ZoneEnd()
return dungeonCreatorUI
