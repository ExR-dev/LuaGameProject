local sprite = {}
sprite.__index = sprite

local Color = require("Color")

local function new(n, c, p)
    assert(n == nil or type(n) == "string", "sprite new - expected args: (string or nil, color or nil, number or nil)")
    assert(c == nil or Color.iscolor(c), "sprite new - expected args: (string or nil, color or nil, number or nil)")
    assert(p == nil or type(p) == "numer", "sprite new - expected args: (string or nil, color or nil, number or nil)")
    
    local s = {
        spriteName = name or "Fallback.png",
        color = c or Color(),
        priority = p or 0,
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
    return "(n: "..self.spriteName..", c: "..self.color:__tostring()..", p: "..tostring(self.priority)..")"
end

sprite.new = new
sprite.issprite = issprite
return setmetatable(sprite, {
    __call = function(_, ...) 
    return sprite.new(...) end
})