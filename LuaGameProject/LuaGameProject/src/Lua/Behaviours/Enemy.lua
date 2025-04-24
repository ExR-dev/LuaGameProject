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

	self.wanderPoint = t.position + vec(math.random(-50, 50), math.random(-50, 50));
	self.speed = math.random(10, 35)
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

	if lengthSqr < 1.0 then
		self.wanderPoint = t.position + vec(math.random(-50, 50), math.random(-50, 50));
	else
		t.position = t.position + toGoal * (self.speed * delta)
	end

	scene.SetComponent(self.ID, "Transform", t)
	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return enemy