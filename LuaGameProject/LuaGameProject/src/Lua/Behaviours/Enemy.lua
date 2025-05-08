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

	if math.random(0, 100000) == 0 then
		local listenerPos = game.GetPlayerCamera().camT.position
		local soundPos = t.position
		local soundDistSqr = ((listenerPos - soundPos) * gameMath.pixelsToMeters):lengthSqr()

		local baseVolume = 0.5
		local falloff = 1.5
		local distScaler = 1.0 / (falloff * soundDistSqr + 1.0)

		game.PlaySound("Maxwell Short.wav", baseVolume * distScaler)
	end

	scene.SetComponent(self.ID, "Transform", t)
	tracy.ZoneEnd()
end


-- Called during ImGui rendering if the entity is selected
function enemy:OnGUI()
	tracy.ZoneBeginN("Lua enemy:OnGUI")

	-- Do ImGui stuff here...
	-- Ex:

	imgui.Text("Hello! \nThis text is created from Lua.")
	imgui.Text("See ImLua.h for supported functions.")

	if enemy.dragFloat == nil then 
		enemy.dragFloat = 1.0 
	end

	local newVal = nil
	newVal = imgui.DragFloat("Lua DragFloat", enemy.dragFloat, 0.01, -1.0, 1.0)

	if newVal ~= nil then
		enemy.dragFloat = newVal
	end

	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return enemy