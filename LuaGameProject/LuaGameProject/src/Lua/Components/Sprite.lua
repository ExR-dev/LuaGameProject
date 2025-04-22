local sprite = {}
sprite.__index = sprite

local function new(name)
    assert(name == nil or type(name) == "string", "sprite new - expected args: string or nil")
    
    local s = {
        spriteName = name or "Fallback"
    }
    return setmetatable(s, sprite)
end

local function issprite(s)
    return getmetatable(s) == sprite
end

-- Meta events
function sprite:__newindex(k)
    print("sprite - not possible to assign new fields")
end

function sprite:__tostring()
    return "sprite("..self.spriteName..")"
end

sprite.new = new
sprite.issprite = issprite
return setmetatable(sprite, {
    __call = function(_, ...) 
    return sprite.new(...) end
})