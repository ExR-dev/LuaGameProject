tracy.ZoneBeginN("Lua Player.lua")
local player = {}

local vec2 = require("Vec2")
local transform = require("Transform2")
local gameMath = require("Utility/GameMath")

-- Global player getter
local function GetPlayer()
	return player
end
game.GetPlayer = GetPlayer

function player:OnCreate()
	tracy.ZoneBeginN("Lua player:OnCreate")
	
	self.trans = transform(scene.GetComponent(self.ID, "Transform"))
	self.speed = 200.0

	-- For tracking held items like weapons
	-- Idea: two-handed weapons could take up both hands, 
	-- preventing the player from using healing items without first holstering the weapon.
	-- Could act as an incentive to use weaker weapons like pistols, as they'd offer more flexible controls.
	-- Also allows for akimbo, I guess. People seem to like that.
	self.rHandEntity = nil
	self.lHandEntity = nil

	-- Inventory of items that are holstered
	self.holsteredEntities = { }

	self.holdOffset = vec2(-18.0, 24.0) -- Invert x-axis for left hand

	self.ammoReserve = {
		["9mm"] = {
			["FMJ"] = 90,
			["HP"] = 36,
			["AP"] = 18
		},
		["12ga"] = {
			["Buck"] = 24,
			["Slug"] = 16,
			["Dart"] = 8
		},
		["5.56"] = {
			["FMJ"] = 180,
		},
		["308"] = {
			["FMJ"] = 25,
		},
	}

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
	end

	-- Rotate the player to face the cursor
	local dir = game.GetCursor().trans.position - self.trans.position
	self.trans.rotation = dir:angle()

	scene.SetComponent(self.ID, "Transform", self.trans)

	if self.rHandEntity ~= nil then
		-- Update the transform of the entity in the players right hand
		self:UpdateHeldItem(self.rHandEntity, self.holdOffset)
	end

	if (self.lHandEntity ~= nil) and (self.lHandEntity ~= self.rHandEntity) then
		-- Update the transform of the entity in the players left hand
		self:UpdateHeldItem(self.lHandEntity, (self.holdOffset * vec2(-1.0, 1.0)))
	end

	tracy.ZoneEnd()
end

function player:UpdateHeldItem(entID, localOffset)
	tracy.ZoneBeginN("Lua player:UpdateHeldItem")
	
	local itemBehaviour = scene.GetComponent(entID, "Behaviour")
	local itemTrans = transform(scene.GetComponent(entID, "Transform"))

	itemTrans.position = self.trans.position
	itemTrans.rotation = self.trans.rotation

	itemTrans:moveRelative(localOffset)

	-- Rotate the item to face the cursor
	local dir = game.GetCursor().trans.position - itemTrans.position
	itemTrans.rotation = dir:angle()

	if itemBehaviour.currRecoil ~= nil then
		itemTrans.rotation = itemTrans.rotation + itemBehaviour.currRecoil
	end

	scene.SetComponent(entID, "Transform", itemTrans)

	itemBehaviour.trans = itemTrans

	if itemBehaviour.OnShoot ~= nil then
		if Input.KeyPressed(Input.Key.KEY_SPACE) then
			itemBehaviour:OnShoot()
		end
	end

	if itemBehaviour.OnReload ~= nil then
		if Input.KeyPressed(Input.Key.KEY_R) then
			itemBehaviour:OnReload(self.ammoReserve)
		end
	end

	tracy.ZoneEnd()
end

tracy.ZoneEnd()
return player