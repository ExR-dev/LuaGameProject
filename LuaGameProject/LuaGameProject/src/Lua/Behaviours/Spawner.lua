

tracy.ZoneBeginN("Lua Spawner.lua")
local spawner = {}

local vec2 = require("Vec2")
local transform = require("Transform2")
local collider = require("Components/Collider")

-- Guaranteed to be called before OnUpdate from the C environment.
-- Can set up more fields in self.
function spawner:OnCreate()
	tracy.ZoneBeginN("Lua enemy:OnCreate")

    self.spawnRate = 10
    self.spawnTimer = 0

    self.capacity = 3
    self.counter = self.capacity

    self.prefab = nil

	tracy.ZoneEnd()
end

-- Called once per tick from the C environment.
function spawner:OnUpdate(delta)
	tracy.ZoneBeginN("Lua enemy:OnUpdate")

    if self.counter > 0 then
        self.spawnTimer = self.spawnTimer + delta
    end

    if self.spawnTimer >= 60/self.spawnRate and self.prefab ~= nil then
        local entityID = game.SpawnPrefab(self.prefab)
		if entityID == nil then
            print("Could not spawn prefab: " .. self.prefab)
        else
            -- Set position to spawner
            if scene.HasComponent(entityID, "Transofrm") then
	            local t = transform(scene.GetComponent(entityID, "Transform"))
                t.position = transform(scene.GetComponent(self.ID, "Transform")).position
	            scene.SetComponent(entityID, "Transform", t)
            end
        end
        self.counter = self.counter - 1
        self.spawnTimer = 0
    end

	tracy.ZoneEnd()
end

-- Reset spawner
function spawner:Reset()
    self.counter = self.capacity
    self.spawnTimer = 0
end


-- Called during ImGui rendering if the entity is selected
function spawner:OnGUI()
	tracy.ZoneBeginN("Lua enemy:OnGUI")

    imgui.Separator("Information")
    imgui.Text(string.format("Time: %.2f/%.2f", self.spawnTimer, 60/self.spawnRate))
    imgui.Text(string.format("Counter: %d/%d", self.counter, self.capacity))

    imgui.Separator("Settings")
    local changed = false

    self.spawnRate, _ = imgui.DragFloat("Spawn Rate (spawn/min)", self.spawnRate, 0.1, 1000, 0.1)
    self.capacity, changed = imgui.InputInt("Capacity", self.capacity)

    if changed then
        self.counter = self.capacity
    end

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

    if imgui.Button("Reset") then
        self.Reset(self)
    end

	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return spawner

