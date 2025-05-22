-- Groups are a collection of prefabs with modifications applied to them.
-- Prefabs in a group are placed relative to the group.
-- Groups are given a position and rotation when created.

local vec2 = require("Vec2")
local transform = require("Transform2")

local groups = {
	-- Summary
	--[[
        ["group name"] = {
            size = vec2(value, value),

            entities = {
                index = {
                    behaviour = {
                        path = "",

                        properties = {
                            ["property"] = value,
                        },
                    },

                    components = {
                        ["component name"] = {
                            ["property"] = value,
                        },
                    },
                }
            }
		},
	--]]
}


if data == nil then
	data = { }
end

if data.groups == nil then
	data.groups = { }
end

if game == nil then
	game = { }
end

if game.SpawnGroup == nil then
	local function SpawnGroup(group, trans)
        local groupData = data.groups[group]

        for k, v in pairs(data.groups) do
            print(k)
        end

        if groupData == nil then
            print("Group not found: "..group)            
            return
        end

        if groupData.entities == nil then
            print("Group doesn't have any entities")
           return 
        end

        -- Spawn entities
        for i, entity in ipairs(groupData.entities) do
            local groupData = entity

            local entity = scene.CreateEntity()

            -- Add behaviour
            if groupData.behaviour then
                scene.SetComponent(entity, "Behaviour", groupData.behaviour.path)

                local behaviour = scene.GetComponent(entity, "Behaviour")

                if behaviour then
                    for propertyName, propertyData in pairs(groupData.behaviour.properties) do
                        behaviour[propertyName] = propertyData
                    end
                else
                    print("Failed to create behaviour "..groupData.behaviour.path.."!")
                end
            end

            -- Add other components
            for componentName, componentData in pairs(groupData.components) do
                scene.SetComponent(entity, componentName, componentData)
		    end

            if scene.HasComponent(entity, "Transform") and transform.istransform2(trans) then
                local t = transform(scene.GetComponent(entity, "Transform"))
                t.position = t.position + trans.position
                t.rotation = t.rotation + trans.rotation
                t.scale = t.scale * trans.scale
	            scene.SetComponent(entity, "Transform", t)
            end

        end
	end
	game.SpawnGroup = SpawnGroup
end

-- TODO: Save behaviour
local componentNames = {
    "Transform",
    "Sprite",
    "Collider", -- TODO: Verify
    "Active",
    "Hardness",
    "Health",
    "CameraData"
}

if game.CreateGroupFromScene == nil then
    local function CreateGroupFromScene(groupName, excludeDebug)
        local excludeDbg = excludeDebug or true;

        data.groups[groupName] = {
            entities = {}
        }

        local entities = scene.GetEntities()
        for i, entity in ipairs(entities) do
            if (excludeDbg and scene.HasComponent(entity, "Debug")) == false then
                data.groups[groupName].entities[i] = {
                    components = {}
                }

                for _, comp in ipairs(componentNames) do
                    local component = scene.GetComponent(entity, comp)
                    if (component ~= nil) then
                        data.groups[groupName].entities[i].components[comp] = component
                    end
                end
            end
        end
    end
    game.CreateGroupFromScene = CreateGroupFromScene
end

data.groups = groups -- HACK