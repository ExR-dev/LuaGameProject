-- This file will be executed from C, returning this
-- "behaviour" table to the stack. The C environment
-- will then populate the table with the following
-- fields:
--		self.ID		- the entity ID
--		self.path	- the path of the this file

tracy.ZoneBeginN("Lua Enemy.lua")
local enemy = {}

local vec2 = require("Vec2")
local transform = require("Transform2")
local color = require("Color")
local gameMath = require("Utility/GameMath")
local collider = require("Components/Collider")

-- Guaranteed to be called before OnUpdate from the C environment.
-- Can set up more fields in self.
function enemy:OnCreate()
	tracy.ZoneBeginN("Lua enemy:OnCreate")
	self.isInitialized = false
	self.trans = transform(scene.GetComponent(self.ID, "Transform"))

	self.speed = math.random(15, 35)

	local c = collider("Enemy", true, vec2(0, 0), vec2(1, 1))
	scene.SetComponent(self.ID, "Collider", c)

	self.hurtAnim = nil

	tracy.ZoneEnd()
end

function enemy:OnInitialize()
	tracy.ZoneBeginN("Lua enemy:OnInitialize")

	self.healthBar = scene.CreateEntity()
	scene.SetComponent(self.healthBar, "Behaviour", "Behaviours/HealthBar")
	local healthBarBeh = scene.GetComponent(self.healthBar, "Behaviour")
	healthBarBeh:Initialize(self.ID, 60.0)

	self.isInitialized = true
	tracy.ZoneEnd()
end

-- Called once per tick from the C environment.
function enemy:OnUpdate(delta)
	tracy.ZoneBeginN("Lua enemy:OnUpdate")
	self.trans = transform(scene.GetComponent(self.ID, "Transform"))

	if not self.isInitialized then
		self:OnInitialize()
	end

	if self.wanderPoint == nil then
		self.wanderPoint = self.trans.position + vec2(math.random(-75, 75), math.random(-75, 75));
	end

	-- I have no idea why vec2 is needed here!
	local goal = vec2(self.wanderPoint)
	local toGoal = goal - self.trans.position


	local lengthSqr = toGoal:lengthSqr()
	toGoal:normalize()

	-- If the distance to the goal is small, pick a new random point, otherwise move towards it
	if lengthSqr < 2.0 then
		self.wanderPoint = self.trans.position + vec2(math.random(-75, 75), math.random(-75, 75));
	else
		self.trans.position = self.trans.position + toGoal * (self.speed * delta)
	end

	-- Set the rotation of the transform to face the goal
	self.trans.rotation = toGoal:angle()

	-- Negate the Y scale if facing left
	if self.trans.rotation > 90.0 and self.trans.rotation < 270.0 then
		self.trans.scale.y = -math.abs(self.trans.scale.y)
	else
		self.trans.scale.y = math.abs(self.trans.scale.y)
	end

	if math.random(0, 100000) == 0 then
		local listenerPos = game.GetPlayerCamera().camT.position
		local soundPos = self.trans.position
		local soundDistSqr = ((listenerPos - soundPos) * gameMath.pixelsToMeters):lengthSqr()

		local baseVolume = 0.5
		local falloff = 1.5
		local distScaler = 1.0 / (falloff * soundDistSqr + 1.0)

		game.PlaySound("Maxwell Short.wav", baseVolume * distScaler)
	end

	-- Update the hurt animation coroutine
	if self.hurtAnim then
		if self.hurtAnim.currTime > 0.0 then
			self.hurtAnim.currTime = self.hurtAnim.currTime - delta
			self.hurtAnim.s.color = gameMath.lerp(self.hurtAnim.defaultCol, self.hurtAnim.hurtCol, self.hurtAnim.currTime / self.hurtAnim.startTime)
			scene.SetComponent(self.ID, "Sprite", self.hurtAnim.s)
		else
			self.hurtAnim.s.color = self.hurtAnim.defaultCol
			scene.SetComponent(self.ID, "Sprite", self.hurtAnim.s)
			self.hurtAnim = nil
		end
	end

	scene.SetComponent(self.ID, "Transform", self.trans)

	tracy.ZoneEnd()
end


-- Called during ImGui rendering if the entity is selected
function enemy:OnGUI()
	tracy.ZoneBeginN("Lua enemy:OnGUI")

	-- Do ImGui stuff here...
	-- Ex:

	imgui.Text("Hello! \nThis text is created from Lua.")
	imgui.Text("See ImLua.h for supported functions.")

	self.speed, _ = imgui.DragFloat("Speed", self.speed, 0.2, 0.0, 1000.0)

	imgui.Separator("Wow, a Lua separator!")

	tracy.ZoneEnd()
end


function enemy:OnHit()
	tracy.ZoneBeginN("Lua enemy:OnHit")

	if not scene.HasComponent(self.ID, "Health") then
		tracy.ZoneEnd()
		return
	end

	local h = scene.GetComponent(self.ID, "Health")

	if h.current <= 0.0 then
		-- TODO: play death animation instead
		scene.RemoveEntity(self.ID)
	end
	
	if not self.hurtAnim then
		self.hurtAnim = {}
		self.hurtAnim.s = scene.GetComponent(self.ID, "Sprite")
		self.hurtAnim.defaultCol = self.hurtAnim.s.color
		self.hurtAnim.hurtCol = color(1, 0, 0, 1)
		self.hurtAnim.s.color = self.hurtAnim.hurtCol
	end
	self.hurtAnim.startTime = 0.2
	self.hurtAnim.currTime = self.hurtAnim.startTime

	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return enemy




























--[[
local enemy = {}

local vec2 = require("Vec2")
local transform = require("Transform2")
local gameMath = require("Utility/GameMath")

function enemy:OnUpdate(delta)
	if self.hurtAnim then
		print("Try")
		if coroutine.status(self.hurtAnim) == "dead" then
			print("Dead")
			self.hurtAnim = nil
		else
			print("Run")
			coroutine.resume(self.hurtAnim, delta)
		end
	end
end

function enemy:OnHit()
	self.hurtAnim = coroutine.create(function (dT)
		print("Start")
		local s = scene.GetComponent(self.ID, "Sprite")

		local startTime = 0.3
		local currTime = startTime
		local defaultCol = s.color
		local hurtCol = color(1, 0, 0, 1)
		s.color = hurtCol

		while (currTime > 0.0) do
			scene.SetComponent(self.ID, "Sprite", s)
			print("Loop")

			dT = coroutine.yield()

			print("Cont. "..tostring(dT))

			currTime = currTime - dT
			s.color = gameMath.lerp(hurtCol, defaultCol, currTime / startTime)
		end
		
		print("End")
		s.color = defaultCol
		scene.SetComponent(self.ID, "Sprite", s)
    end)
end

return enemy
--]]