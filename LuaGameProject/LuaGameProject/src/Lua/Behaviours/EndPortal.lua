
tracy.ZoneBeginN("Lua EndPortal.lua")
local endPortal = {}

local vec2 = require("Vec2")
local collider = require("Components/Collider")

-- Guaranteed to be called before OnUpdate from the C environment.
-- Can set up more fields in self.
function endPortal:OnCreate()
	tracy.ZoneBeginN("Lua endPortal:OnCreate")

	local c = collider("EndPortal", false, vec2(0, 0), vec2(1.0, 1.0), 0, true, function(other)
		local c = collider(scene.GetComponent(other, "Collider"))
		if c.tag ~= "Player" then
			return
		end
        print("End")
    end)

    scene.SetComponent(self.ID, "Collider", c)

	tracy.ZoneEnd()
end

-- Called once per tick from the C environment.
function endPortal:OnUpdate(delta)
	tracy.ZoneBeginN("Lua endPortal:OnUpdate")

	tracy.ZoneEnd()
end

-- Called during ImGui rendering if the entity is selected
function endPortal:OnGUI()
	tracy.ZoneBeginN("Lua endPortal:OnGUI")

	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return endPortal

