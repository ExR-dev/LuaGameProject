tracy.ZoneBeginN("Lua Player.lua")
local player = {}

local vec = require("Vec2")
local transform = require("Transform2")

function player:OnCreate()
	tracy.ZoneBeginN("Lua player:OnCreate")
	
	self.speed = 250.0

	tracy.ZoneEnd()
end

function player:OnUpdate(delta)
	tracy.ZoneBeginN("Lua player:OnUpdate")
	local t = transform(scene.GetComponent(self.ID, "Transform"))
	
	local move = vec(0.0, 0.0)

    if (Input.KeyHeld(Input.Key.KEY_W) == true) then
		move.y = move.y - 1.0
    end

    if (Input.KeyHeld(Input.Key.KEY_S) == true) then
		move.y = move.y + 1.0
    end

    if (Input.KeyHeld(Input.Key.KEY_D) == true) then
		move.x = move.x + 1.0
    end

    if (Input.KeyHeld(Input.Key.KEY_A) == true) then
		move.x = move.x - 1.0
    end

	move:normalize()
	t.position = t.position + (move * (self.speed * delta))

	scene.SetComponent(self.ID, "Transform", t)
	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return player