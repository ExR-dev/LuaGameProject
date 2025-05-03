tracy.ZoneBeginN("Lua Run PrintEntity.lua")

require("PrintTable")

local function PrintEntityComponent(entity, componentName)
	if scene.HasComponent(entity, componentName) then
		local component = scene.GetComponent(entity, componentName)
		if component == nil then return end
		PrintTable(component, componentName) 
	end
end

function game.PrintEntity(entity)
	tracy.ZoneBeginN("Lua PrintEntity")

	if not scene.IsEntity(entity) then 
		print("nil")
		tracy.ZoneEnd()
		return
	end

	PrintEntityComponent(entity, "Remove")
	PrintEntityComponent(entity, "Active")
	PrintEntityComponent(entity, "Transform")
	PrintEntityComponent(entity, "Collider")
	PrintEntityComponent(entity, "Sprite")
	PrintEntityComponent(entity, "Health")
	PrintEntityComponent(entity, "CameraData")
	PrintEntityComponent(entity, "Behaviour")

	tracy.ZoneEnd()
end

tracy.ZoneEnd()