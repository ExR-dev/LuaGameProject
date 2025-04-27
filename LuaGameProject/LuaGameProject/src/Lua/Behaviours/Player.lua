tracy.ZoneBeginN("Lua Player.lua")
local player = {}

local vec2 = require("Vec2")
local transform = require("Transform2")
local gameMath = require("Utility/GameMath")

-- Global player getter
function GetPlayer()
	return player
end

function player:OnCreate()
	tracy.ZoneBeginN("Lua player:OnCreate")
	
	self.trans = transform(scene.GetComponent(self.ID, "Transform"))
	self.speed = 200.0

	tracy.ZoneEnd()
end

function player:OnUpdate(delta)
	tracy.ZoneBeginN("Lua player:OnUpdate")
	
	self.trans = transform(scene.GetComponent(self.ID, "Transform"))
	local move = vec2(0.0, 0.0)

	if Input.KeyHeld(Input.Key.KEY_W) then
		move.y = move.y - 1.0
	end

	if Input.KeyHeld(Input.Key.KEY_S) then
		move.y = move.y + 1.0
	end

	if Input.KeyHeld(Input.Key.KEY_D) then
		move.x = move.x + 1.0
	end

	if Input.KeyHeld(Input.Key.KEY_A) then
		move.x = move.x - 1.0
	end

	if not gameMath.approx(move:lengthSqr(), 0.0) then
		move:normalize()
		self.trans.position = self.trans.position + (move * (self.speed * delta))

		scene.SetComponent(self.ID, "Transform", self.trans)
	end

	if Input.KeyPressed(Input.Key.KEY_SPACE) then
		self:OnShoot()
	end

	tracy.ZoneEnd()
end

-- TODO: Temporary, this should be done by a separate weapon behaviour
function player:OnShoot()
	tracy.ZoneBeginN("Lua player:OnShoot")

	local origin = self.trans.position
	local dir = GetCursor().trans.position - origin

	local projEnt = scene.CreateEntity()
	local projT = transform(origin, dir:angle(), vec2(24, 6))

	scene.SetComponent(projEnt, "Transform", projT)
	scene.SetComponent(projEnt, "Behaviour", "Behaviours/Projectile")

	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return player