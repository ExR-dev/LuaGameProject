tracy.ZoneBeginN("Lua SandboxUI.lua")

local vec2 = require("Vec2")

local dungeonCreatorUI = {
	prefabCollection = {

	}
}


function dungeonCreatorUI:PrefabCollection()
	tracy.ZoneBeginN("Lua sandboxUI:Temp")

	imgui.Begin("Prefab Collection")

	for name, _ in pairs(data.prefabs) do
		if imgui.Button(name) then
			local prefabEntity = game.SpawnPrefab(name)
		end
	end

	imgui.End()

	tracy.ZoneEnd()
end


tracy.ZoneEnd()
return dungeonCreatorUI
