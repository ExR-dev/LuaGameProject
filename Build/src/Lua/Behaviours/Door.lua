
tracy.ZoneBeginN("Lua Door.lua")
local door = {}

local vec2 = require("Vec2")
local transform = require("Transform2")
local collider = require("Components/Collider")

-- Guaranteed to be called before OnUpdate from the C environment.
-- Can set up more fields in self.
function door:OnCreate()
	tracy.ZoneBeginN("Lua door:OnCreate")

    self.linkID = nil
    local active = true

	local c = collider("Door", false, vec2(0, 0), vec2(1.0, 1.0), 0, true, 
        -- On Enter function
        function(other)
            local c = collider(scene.GetComponent(other, "Collider"))
            if c.tag ~= "Player" or not active then
                return
            end

            if self.linkID ~= nil then
                if scene.HasComponent(other, "Transform") and scene.HasComponent(self.linkID, "Transform") then
                    local otherT = transform(scene.GetComponent(other, "Transform"))
                    local link = transform(scene.GetComponent(self.linkID, "Transform"))
                    otherT.position = link.position
                    scene.SetComponent(other, "Transform", otherT)
                    active = false
                end 
            end
        end,

        -- On Exit function
        function(other)
            active = true
        end
    )

    scene.SetComponent(self.ID, "Collider", c)

	tracy.ZoneEnd()
end

-- Called once per tick from the C environment.
function door:OnUpdate(delta)
	tracy.ZoneBeginN("Lua door:OnUpdate")

	tracy.ZoneEnd()
end

-- Called during ImGui rendering if the entity is selected
function door:OnGUI()
	tracy.ZoneBeginN("Lua door:OnGUI")


    local title = "Select Link"
    if (self.linkID ~= nil) then
        title = string.format("%d", self.linkID)
    end
    if imgui.BeginCombo("Link", title) then
        for i, entity in ipairs(scene.GetEntities()) do
			if scene.HasComponent(entity, "Collider") and entity ~= self.ID then
				local c = collider(scene.GetComponent(entity, "Collider"))
                if c.tag == "Door" then
                    local isSelected = self.linkID == entity
                    if imgui.Selectable(string.format("%d", entity), isSelected) then
                        self.linkID = entity
                    end

                    if isSelected then
                        imgui.SetItemDefaultFocus()
                    end
                end
            end
        end
        imgui.EndCombo()
    end

	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return door

