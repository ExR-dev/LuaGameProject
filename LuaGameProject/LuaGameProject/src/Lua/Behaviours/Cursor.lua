
tracy.ZoneBeginN("Lua Cursor.lua")
local cursor = {}

local vec2 = require("Vec2")
local transform = require("Transform2")
local gameMath = require("Utility/GameMath")

function cursor:OnCreate()
	tracy.ZoneBeginN("Lua cursor:OnCreate")

    self.transform = transform(scene.GetComponent(self.ID, "Transform"))

	tracy.ZoneEnd()
end

function cursor:OnUpdate(delta)
	tracy.ZoneBeginN("Lua cursor:OnUpdate")

    self.transform.position = GetPlayerCamera().camT.position + vec2(Input.GetMouseInfo().Position) - vec2(Window.width/2, Window.height/2);
	scene.SetComponent(self.ID, "Transform", self.transform)

	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return cursor