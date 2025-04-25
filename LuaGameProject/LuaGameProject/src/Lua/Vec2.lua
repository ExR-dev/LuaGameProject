local vec2 = {}
vec2.__index = vec2

function vec2.new(x, y)
    if type(x) == "table" then
        return setmetatable(x, vec2)
    else
        local t = {
            x = x or 0,
            y = y or 0
        }
        return setmetatable(t, vec2)
    end
end

function vec2.isvec2(t)
    return getmetatable(t) == vec2
end

function vec2.__newindex(t, k)
    print("vec2 - not possible to assign new fields")
end

function vec2.__tostring(t)
    return "("..t.x..", "..t.y..")"
end

-- Meta logic operators
function vec2.__unm(t)
    assert(vec2.isvec2(t), "vec2 unm - expected args: (vec2)")
    return vec2.new(-t.x, -t.y)
end

function vec2.__add(a, b)
    assert(vec2.isvec2(a) and vec2.isvec2(b), "vec2 add - expected args: (vec2, vec2)")
    return vec2.new(a.x + b.x, a.y + b.y)
end

function vec2.__sub(a, b)
    assert(vec2.isvec2(a) and vec2.isvec2(b), "vec2 sub - expected args: (vec2, vec2)")
    return vec2.new(a.x - b.x, a.y - b.y)
end

function vec2.__mul(a, b)
    -- Check for scalar multiplication
    if vec2.isvec2(a) and type(b) == "number" then
        return vec2.new(a.x * b, a.y * b)
    elseif type(a) == "number" and vec2.isvec2(b) then
        return vec2.new(a * b.x, a * b.y)
    elseif vec2.isvec2(a) and vec2.isvec2(b) then
        return vec2.new(a.x * b.x, a.y * b.y)
    else
        assert(false, "vec2 mul - expected args: (vec2, vec2) or (vec2, number) or (number, vec2)")
    end
end

function vec2.__div(a, b)
    assert(vec2.isvec2(a), "vec2 div - expected args: (vec2, vec2) or (vec2, number)")
    if type(b) == "number" then
        return vec2.new(a.x / b, a.y / b)
    else
        assert(vec2.isvec2(b), "vec2 div - expected args: (vec2, vec2) or (vec2, number)")
        return vec2.new(a.x / b.x, a.y / b.y)
    end
end

function vec2.__eq(a, b)
    assert(vec2.isvec2(a) and vec2.isvec2(b), "vec2 eq - expected args: (vec2, vec2)")
    return a.x == b.x and a.y == b.y
end

function vec2:lengthSqr()
    return (self.x * self.x) + (self.y * self.y)
end

function vec2:length()
    return math.sqrt(self:lengthSqr())
end

function vec2:normalized()
    local len = self:length()
    return self / len
end

function vec2:normalize()
    local len = self:length()
    if len == 0 then
        return self
    end

    self.x = self.x / len
    self.y = self.y / len
    return self
end

function vec2.dot(a, b)
    return (a.x * b.x) + (a.y * b.y)
end

function vec2:angle()
    local norm = self:normalized()
    local angle = math.atan(norm.y, norm.x)
    if angle < 0 then
        angle = angle + (2 * math.pi)
    end
    return angle
end

return setmetatable(vec2, {
    __call = function(_, ...)
        return vec2.new(...)
    end
})