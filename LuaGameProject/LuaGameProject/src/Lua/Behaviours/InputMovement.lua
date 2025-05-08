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
local gameMath = require("Utility/GameMath")

-- Guaranteed to be called before OnUpdate from the C environment.
-- Can set up more fields in self.
function enemy:OnCreate()
	tracy.ZoneBeginN("Lua enemy:OnCreate")
	local t = transform(scene.GetComponent(self.ID, "Transform"))

	self.speed = math.random(15, 35)
	tracy.ZoneEnd()
end

-- Called once per tick from the C environment.
function enemy:OnUpdate(delta)
	tracy.ZoneBeginN("Lua enemy:OnUpdate")
	local t = transform(scene.GetComponent(self.ID, "Transform"))


	local goal = t.position;
	if Input.KeyHeld(Input.Key.KEY_W) then
		goal.y = goal.y - 1.0
	end
	if Input.KeyHeld(Input.Key.KEY_S) then
		goal.y = goal.y + 1.0
	end
	if Input.KeyHeld(Input.Key.KEY_D) then
		goal.x = goal.x + 1.0
	end
	if Input.KeyHeld(Input.Key.KEY_A) then
		goal.x = goal.x - 1.0
	end

	local toGoal = goal - t.position

	local lengthSqr = toGoal:lengthSqr()
	toGoal:normalize()

	-- If the distance to the goal is small, pick a new random point, otherwise move towards it
	t.position = t.position + toGoal * (self.speed * delta)

	-- Set the rotation of the transform to face the goal
	t.rotation = toGoal:angle()

	scene.SetComponent(self.ID, "Transform", t)
	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return enemy