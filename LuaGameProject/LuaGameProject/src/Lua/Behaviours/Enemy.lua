-- This file will be executed from C, returning this
-- "behaviour" table to the stack. The C environment
-- will then populate the table with the following
-- fields:
--		self.ID		- the entity ID
--		self.path	- the path of the this file

tracy.ZoneBeginN("Lua Enemy.lua")
local enemy = {}

local vec = require("Vec2")
local transform = require("Transform2")

-- Guaranteed to be called before OnUpdate from the C environment.
-- Can set up more fields in self.
function enemy:OnCreate()
	tracy.ZoneBeginN("Lua enemy:OnCreate")
	local t = transform(scene.GetComponent(self.ID, "Transform"))

	self.wanderPoint = t.position + vec(math.random(-75, 75), math.random(-75, 75));
	self.speed = math.random(15, 35)
	tracy.ZoneEnd()
end

-- Called once per tick from the C environment.
function enemy:OnUpdate(delta)
	tracy.ZoneBeginN("Lua enemy:OnUpdate")
	local t = transform(scene.GetComponent(self.ID, "Transform"))

	local goal = self.wanderPoint
	local toGoal = goal - t.position

	local lengthSqr = toGoal:lengthSqr()
	toGoal:normalize()

	-- If the distance to the goal is small, pick a new random point, otherwise move towards it
	if lengthSqr < 2.0 then
		self.wanderPoint = t.position + vec(math.random(-75, 75), math.random(-75, 75));
	else
		t.position = t.position + toGoal * (self.speed * delta)
	end

	-- Set the rotation of the transform to face the goal
	t.rotation = toGoal:angle()

	-- Negate the Y scale if facing left
	if t.rotation > 90.0 and t.rotation < 270.0 then
		t.scale.y = -math.abs(t.scale.y)
	else
		t.scale.y = math.abs(t.scale.y)
	end

	if math.random(0, 50000) == 0 then
		game.PlaySound("Maxwell Short.wav", 0.33)
	end

	scene.SetComponent(self.ID, "Transform", t)
	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return enemy