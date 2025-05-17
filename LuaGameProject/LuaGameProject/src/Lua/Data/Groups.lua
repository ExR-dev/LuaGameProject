-- Groups are a collection of prefabs with modifications applied to them.
-- Prefabs in a group are placed relative to the group.
-- Groups are given a position and rotation when created.

local vec2 = require("Vec2")

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

	["TestRoom"] = {
        entities = {
            [1] = {
                behaviour = {
                    path = "Behaviours/Enemy",

                    properties = {

                    }
                },

                components = {
                    ["Transform"] = {
                        ["scale"] = vec2(60, 60)
                    },

                    ["Collider"] = {
                        ["tag"] = "Enemy",
                        ["debug"] = false,
                        ["offset"] = vec2(0, 0),
                        ["extents"] = vec2(1, 1),
                        ["rotation"] = 0
                    },
                    
                    ["Sprite"] = {
                        ["spriteName"] = "Maxwell.png",
                        ["priority"] = 30,
                        ["color"] = {r=1,g=1,b=1,a=1}
                    }
                }
            },
            [2] = {
                behaviour = {
                    path = "Behaviours/Enemy",

                    properties = {

                    }
                },

                components = {
                    ["Transform"] = {
                        ["position"] = vec2(100, 0),
                        ["scale"] = vec2(60, 60)
                    },

                    ["Collider"] = {
                        ["tag"] = "Enemy",
                        ["debug"] = false,
                        ["offset"] = vec2(0, 0),
                        ["extents"] = vec2(1, 1),
                        ["rotation"] = 0
                    },
                    
                    ["Sprite"] = {
                        ["spriteName"] = "Maxwell.png",
                        ["priority"] = 30,
                        ["color"] = {r=1,g=1,b=1,a=1}
                    }
                }
            }, 
            [3] = {
                behaviour = {
                    path = "Behaviours/Enemy",

                    properties = {

                    }
                },

                components = {
                    ["Transform"] = {
                        ["position"] = vec2(-100, 0),
                        ["scale"] = vec2(60, 60)
                    },

                    ["Collider"] = {
                        ["tag"] = "Enemy",
                        ["debug"] = false,
                        ["offset"] = vec2(0, 0),
                        ["extents"] = vec2(1, 1),
                        ["rotation"] = 0
                    },
                    
                    ["Sprite"] = {
                        ["spriteName"] = "Maxwell.png",
                        ["priority"] = 30,
                        ["color"] = {r=1,g=1,b=1,a=1}
                    }
                }
            } 
        }
	}
}


if data == nil then
	data = { }
end

if data.prefabs == nil then
	data.groups = { }
end

if game == nil then
	game = { }
end

if game.SpawnGroup == nil then
	local function SpawnGroup(group)
        local groupData = data.groups[group]

        if (groupData == nil) then
            print("Group not found"..group)            
            return
        end

        -- Spawn entities
        for i, entity in ipairs(groupData.entities) do
            local prefabData = entity

            local entity = scene.CreateEntity()

            -- Add behaviour
            if prefabData.behaviour then
                scene.SetComponent(entity, "Behaviour", prefabData.behaviour.path)

                local behaviour = scene.GetComponent(entity, "Behaviour")

                if behaviour then
                    for propertyName, propertyData in pairs(prefabData.behaviour.properties) do
                        behaviour[propertyName] = propertyData
                    end
                else
                    print("Failed to create behaviour "..prefabData.behaviour.path.."!")
                end
            end

            -- Add other components
            for componentName, componentData in pairs(prefabData.components) do
                    scene.SetComponent(entity, componentName, componentData)
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
    local function CreateGroupFromScene(groupName)
        if data.groups[groupName] ~= nil then
            print("Group '" .. groupName .. "' has been overwritten!")
        end

        data.groups[groupName] = {
            entities = {}
        }

        local entities = scene.GetEntities()
        for i, entity in ipairs(entities) do
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
    game.CreateGroupFromScene = CreateGroupFromScene
end

data.groups = groups -- HACK