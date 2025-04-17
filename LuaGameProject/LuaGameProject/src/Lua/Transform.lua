local transform = {}
transform.__index = transform

local vector = require("Vector")

local function new(p, r, s)
    assert(p == nil or vector.isvector(p), "transform new - expected args: vector or nil")
    
    local t = {
        position = p or vector(),
        rotation = r or vector(),
        scale = s or vector()
    }
    return setmetatable(t, transform)
end

local function istransform(t)
    return getmetatable(t) == transform
end

-- Meta events
function transform:__newindex(k, v)
    print("transform - not possible to assign new fields")
end

function transform:__tostring()
    return "(p: "..self.position:__tostring()..
           ", r: "..self.rotation:__tostring()..
           ", s: "..self.scale:__tostring()..")"
end

-- Meta logic operators
function transform.__eq(a, b)
    assert(istransform(a) and istransform(b), "transform eq - expected args: transform, transform")
    return a.position==b.position and a.rotation==b.rotation and a.scale==b.scale
end

-- Wrap up and send :)
transform.new = new
transform.istransform = istransform
return setmetatable(transform, {
    __call = function(_, ...) 
    return transform.new(...) end
})