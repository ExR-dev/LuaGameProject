local transform2 = {}
transform2.__index = transform2

local vec2 = require("Vec2")

local function new(p, r, s)
    if not vec2.isvec2(p) and type(p) == "table" then
        local t = {
            position = vec2(p.position),
            rotation = p.rotation,
            scale = vec2(p.scale)
        }
        return setmetatable(t, transform2)
    else
        local t = {
            position = p or vec2(),
            rotation = r or 0.0,
            scale = s or vec2()
        }
        return setmetatable(t, transform2)
    end
end

local function istransform2(t)
    return getmetatable(t) == transform2
end

-- Meta events
function transform2:__newindex(k, v)
    print("transform2 - not possible to assign new fields")
end

function transform2:__tostring()
    return "(p: "..self.position:__tostring()..
           ", r: "..tostring(self.rotation)..
           ", s: "..self.scale:__tostring()..")"
end

-- Meta logic operators
function transform2.__eq(a, b)
    assert(istransform2(a) and istransform2(b), "transform2 eq - expected args: transform2, transform2")
    return (a.position == b.position) and
           (a.rotation == b.rotation) and
           (a.scale == b.scale)
end

function transform2:getForward()
    return vec2(math.cos(math.rad(self.rotation)), math.sin(math.rad(self.rotation)))
end

function transform2:getRight()
    return vec2(math.cos(math.rad(self.rotation - 90.0)), math.sin(math.rad(self.rotation - 90.0)))
end

function transform2:moveRelative(movement)
    assert(vec2.isvec2(movement), "transform2 moveRelative : expected args: vec2")
    local fwd = self:getForward()
    local right = self:getRight()
    local relativeMovement = (fwd * movement.y) + (right * movement.x)
    self.position = self.position + relativeMovement
end

transform2.new = new
transform2.istransform2 = istransform2
return setmetatable(transform2, {
    __call = function(_, ...) 
    return transform2.new(...) end
})