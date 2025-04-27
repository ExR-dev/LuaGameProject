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

	self.shootFunc = self.OnShootDefault

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


	if Input.KeyPressed(Input.Key.KEY_Q) then
		if self.shootFunc == self.OnShootDefault then
			self.shootFunc = self.OnShootBurst
		else
			self.shootFunc = self.OnShootDefault
		end
	end

	if Input.KeyPressed(Input.Key.KEY_SPACE) then
		self.shootFunc(self)
	end

	tracy.ZoneEnd()
end

-- TODO: Temporary, this should be done by a weapon behaviour
function player.OnShootDefault(self)
	tracy.ZoneBeginN("Lua player:OnShootDefault")

	local origin = self.trans.position
	local dir = GetCursor().trans.position - origin

	local offsetAngle = dir:angle() + ((math.random() - 0.5) * 2.0)

	local projEnt = scene.CreateEntity()
	local projT = transform(origin, offsetAngle, vec2(24, 6))

	scene.SetComponent(projEnt, "Transform", projT)
	scene.SetComponent(projEnt, "Behaviour", "Behaviours/Projectile")

	tracy.ZoneEnd()
end

function player.OnShootBurst(self)
	tracy.ZoneBeginN("Lua player:OnShootBurst")

	local origin = self.trans.position
	local dir = GetCursor().trans.position - origin

	dir:normalize()
	local angle = dir:angle()

	local count = 5
	local spread = 10.0

	for i = 1, count do
		local offsetAngle = angle + ((math.random() - 0.5) * spread)

		local projEnt = scene.CreateEntity()
		local projT = transform(origin, offsetAngle, vec2(18, 4))

		scene.SetComponent(projEnt, "Transform", projT)
		scene.SetComponent(projEnt, "Behaviour", "Behaviours/Projectile")
	end
	
	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return player