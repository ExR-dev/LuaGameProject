tracy.ZoneBeginN("Lua Player.lua")
local player = {}

local gameMath = require("Utility/GameMath")
local vec2 = require("Vec2")
local transform = require("Transform2")
local collider = require("Components/Collider")

-- Global player getter
local function GetPlayer()
	return player
end
game.GetPlayer = GetPlayer

function player:OnCreate()
	tracy.ZoneBeginN("Lua player:OnCreate")
	
	self.trans = transform(scene.GetComponent(self.ID, "Transform"))
	self.speed = 200.0

	self.interactOptions = nil
	
	-- Create player collider
	local c = collider("Player", true, vec2(0, 0), vec2(1.0, 1.0), function(other) 
		tracy.ZoneBeginN("Lua Lambda player:Collide")

		local o = scene.GetComponent(other, "Collider")
		if (o.tag == "Weapon") then
			-- Ensure that weapon is not currently held
			if other == self.rHandEntity or other == self.lHandEntity then
				tracy.ZoneEnd()
				return
			end

			-- Ensure that weapon is not already in interactOptions
			if self.interactOptions then
				for _, option in ipairs(self.interactOptions) do
					if other == option then
						tracy.ZoneEnd()
						return
					end
				end
			end

			-- Ensure that weapon is not currently holstered
			for _, holstered in ipairs(self.holsteredEntities) do
				if other == holstered then
					tracy.ZoneEnd()
					return
				end
			end
			
			if not self.interactOptions then
				self.interactOptions = { }
			end

			table.insert(self.interactOptions, other)
		end

		tracy.ZoneEnd()
	end)

	scene.SetComponent(self.ID, "Collider", c)

	-- For tracking held items like weapons
	-- Idea: two-handed weapons could take up both hands, 
	-- preventing the player from using healing items without first holstering the weapon.
	-- Could act as an incentive to use weaker weapons like pistols, as they'd offer more flexible controls.
	-- Also allows for akimbo, I guess. People seem to like that.
	self.rHandEntity = nil
	self.lHandEntity = nil

	self.rHandHoldTime = 0.0
	self.lHandHoldTime = 0.0

	self.nextHandFire = 0 -- 0 for right, 1 for left
	self.didFire = false

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
	local cursorPos = game.GetCursor().trans.position
	local dir = cursorPos - self.trans.position
	self.trans.rotation = dir:angle()

	scene.SetComponent(self.ID, "Transform", self.trans)
	
	if self.interactOptions then
		-- Remove all interact options that are further than 1.25m from the player
		for i, option in ipairs(self.interactOptions) do
			local optionTrans = transform(scene.GetComponent(option, "Transform"))
			local offset = optionTrans.position - self.trans.position
			local distSqr = offset:lengthSqr()
			if distSqr > 125*125 then
				-- Remove the option from the list
				table.remove(self.interactOptions, i)
			end
		end

		if #self.interactOptions == 0 then
			self.interactOptions = nil
		end

		-- Check if the player is trying to interact
		if Input.KeyPressed(Input.Key.KEY_E) and self.interactOptions then
			-- Determine the ideal target out of the options based on distance to cursor
			
			local closest = nil
			local closestDistSqr = math.huge

			for _, entID in ipairs(self.interactOptions) do
				local entTrans = transform(scene.GetComponent(entID, "Transform"))
				local distSqr = (entTrans.position - cursorPos):lengthSqr()

				if distSqr < closestDistSqr then
					closestDistSqr = distSqr
					closest = entID
				end
			end

			if closest then
				local closestBehaviour = scene.GetComponent(closest, "Behaviour")
				closestBehaviour:Interact()
			end

			self.interactOptions = nil
		end
	end

	if self.rHandEntity ~= nil then
		-- Update the transform of the entity in the players right hand
		self.rHandHoldTime = self.rHandHoldTime + delta
		
		local itemBehaviour = scene.GetComponent(self.rHandEntity, "Behaviour")
		
		local isSemi = (itemBehaviour.stats.fireMode == "Semi")
		local akimbo = (
			isSemi and -- Only alternate semiautomatic fire
			self.lHandEntity and -- Left hand is also full
			self.rHandEntity ~= self.lHandEntity -- Both hands are not the same entity
		)
		
		self:UpdateHeldItem(
			self.rHandEntity, 
			self.holdOffset, 
			(not akimbo) or (self.nextHandFire == 0)
		)
		self.didFire = self.didFire and isSemi -- Prevents automatic primary from disabing semi-automatic secondary
	end

	if (self.lHandEntity ~= nil) and (self.lHandEntity ~= self.rHandEntity) then
		-- Update the transform of the entity in the players left hand
		self.lHandHoldTime = self.lHandHoldTime + delta

		local itemBehaviour = scene.GetComponent(self.lHandEntity, "Behaviour")
		
		local akimbo = (
			(itemBehaviour.stats.fireMode == "Semi") and -- Only alternate semiautomatic fire
			self.rHandEntity -- Right hand is also full
		)
		
		self:UpdateHeldItem(
			self.lHandEntity, 
			(self.holdOffset * vec2(-1.0, 1.0)), 
			(not akimbo) or ((not self.didFire) and (self.nextHandFire == 1))
		)
	end
	self.didFire = false

	tracy.ZoneEnd()
end

function player:UpdateHeldItem(entID, localOffset, allowFire)
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

	if allowFire and itemBehaviour.OnShoot ~= nil then
		if Input.KeyPressed(Input.Key.KEY_SPACE) then
			itemBehaviour:OnShoot()
			self.nextHandFire = (self.nextHandFire + 1) % 2
			self.didFire = true
		end
	end

	if itemBehaviour.OnReload ~= nil then
		if Input.KeyPressed(Input.Key.KEY_R) then
			itemBehaviour:OnReload(self.ammoReserve)
		end
	end

	tracy.ZoneEnd()
end

function player:DropItems(count)
	-- if count is 1, drop the oldest held item
	-- otherwise, empty both hands

	if self.rHandEntity == self.lHandEntity then
		-- Held item is two-handed, empty both hands
		local itemBehaviour = scene.GetComponent(self.rHandEntity, "Behaviour")
		itemBehaviour:Drop()

		self.rHandEntity = nil
		self.lHandEntity = nil
		self.rHandHoldTime = 0.0
		self.lHandHoldTime = 0.0
	elseif count == 1 then
		-- Drop the oldest held item
		if self.rHandHoldTime > self.lHandHoldTime then
			local itemBehaviour = scene.GetComponent(self.rHandEntity, "Behaviour")
			itemBehaviour:Drop()

			self.rHandEntity = nil
			self.rHandHoldTime = 0.0
		else
			local itemBehaviour = scene.GetComponent(self.lHandEntity, "Behaviour")
			itemBehaviour:Drop()

			self.lHandEntity = nil
			self.lHandHoldTime = 0.0
		end
	else
		-- Drop both items
		if self.rHandEntity then
			local itemBehaviour = scene.GetComponent(self.rHandEntity, "Behaviour")
			itemBehaviour:Drop()

			self.rHandEntity = nil
			self.rHandHoldTime = 0.0
		end

		if self.lHandEntity then
			local itemBehaviour = scene.GetComponent(self.lHandEntity, "Behaviour")
			itemBehaviour:Drop()

			self.lHandEntity = nil
			self.lHandHoldTime = 0.0
		end
	end
end

tracy.ZoneEnd()
return player