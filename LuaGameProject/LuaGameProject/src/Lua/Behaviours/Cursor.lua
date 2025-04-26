
tracy.ZoneBeginN("Lua Cursor.lua")
local cursor = {}

local vec2 = require("Vec2")
local transform = require("Transform2")
local gameMath = require("GameMath")

function cursor:OnCreate()
	tracy.ZoneBeginN("Lua cursor:OnCreate")

    self.transform = transform(scene.GetComponent(self.ID, "Transform"))

	tracy.ZoneEnd()
end

function cursor:OnUpdate(delta)
	tracy.ZoneBeginN("Lua cursor:OnUpdate")

    -- TODO: Offset by half screen size
    self.transform.position = GetPlayerCamera().camT.position + vec2(Input.GetMouseInfo().Position);
	scene.SetComponent(self.ID, "Transform", self.transform)

	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return cursor