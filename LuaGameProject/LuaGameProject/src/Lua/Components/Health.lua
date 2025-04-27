local health = {}
health.__index = health

local function new(c, m)
    assert(type(c) == "number", "health new - expected args: (number, number or nil)")
    assert(m == nil or type(m) == "number", "health new - expected args: (number, number or nil)")
    
    local h = { -- If only one arg is passed, both current and max are set to that value
        current = c,
        max = m or c
    }
    return setmetatable(h, health)
end

local function ishealth(h)
    return getmetatable(h) == health
end

-- Meta events
function health:__newindex(k)
    print("health - not possible to assign new fields")
end

function health:__tostring()
    return "health("..tostring(self.current).."/"..tostring(self.max)..")"
end

health.new = new
health.ishealth = ishealth
return setmetatable(health, {
    __call = function(_, ...) 
    return health.new(...) end
})