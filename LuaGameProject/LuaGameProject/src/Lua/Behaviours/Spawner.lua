

tracy.ZoneBeginN("Lua Spawner.lua")
local enemy = {}

local vec2 = require("Vec2")
local transform = require("Transform2")
local color = require("Color")
local gameMath = require("Utility/GameMath")
local collider = require("Components/Collider")

-- Guaranteed to be called before OnUpdate from the C environment.
-- Can set up more fields in self.
function enemy:OnCreate()
	tracy.ZoneBeginN("Lua enemy:OnCreate")

    self.spawnRate = 10
    self.spawnTimer = 0
    self.prefab = ""

	tracy.ZoneEnd()
end

-- Called once per tick from the C environment.
function enemy:OnUpdate(delta)
	tracy.ZoneBeginN("Lua enemy:OnUpdate")

    self.spawnTimer = self.spawnTimer + delta
    if self.spawnTimer >= 60/self.spawnRate then
		if ~game.SpawnPrefab(self.prefab) then
            print("Could not spawn prefab: " .. self.prefab)
        end
        self.spawnTimer = 0
    end

	tracy.ZoneEnd()
end


-- Called during ImGui rendering if the entity is selected
function enemy:OnGUI()
	tracy.ZoneBeginN("Lua enemy:OnGUI")

    self.spawnRate, _ = imgui.DragFloat("Spawn Rate (spawn/min)", self.spawnRate, 0.0001, 1000, 0.1)

    if imgui.BeginCombo("Spawn Prefab", self.prefab) then
        for name, _ in pairs(data.prefabs) do
            local isSelected = self.prefab == name
            if imgui.Selectable(name, isSelected) then
                self.prefab = name
            end

            if isSelected then
                imgui.SetItemDefaultFocus()
            end

        end
        imgui.EndCombo()
    end

	tracy.ZoneEnd()
end


function enemy:OnHit()
	tracy.ZoneBeginN("Lua enemy:OnHit")

	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return enemy

