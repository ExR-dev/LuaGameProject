local vector = {}
vector.__index = vector

function vector.new(x, y, z)
    local t = {
        x = x or 0,
        y = y or 0,
        z = z or 0
    }
    return setmetatable(t, vector)
end

function vector.isvector(t)
    return getmetatable(t) == vector
end

function vector.__newindex(t, k, v)
    print("vector - not possible to assign new fields")
end

function vector.__tostring(t)
    return "("..t.x..", "..t.y..", "..t.z..")"
end

-- Meta logic operators
function vector.__unm(t)
    assert(vector.isvector(t), "vector unm - expected args: vector")
    return vector.new(-t.x, -t.y, -t.z)
end

function vector.__add(a, b)
    assert(vector.isvector(a) and vector.isvector(b), "vector add - expected args: vector, vector")
    return vector.new(a.x+b.x, a.y+b.y, a.z+b.z)
end

function vector.__sub(a, b)
    assert(vector.isvector(a) and vector.isvector(b), "vector sub - expected args: vector, vector")
    return vector.new(a.x-b.x, a.y-b.y, a.z-b.z)
end

function vector.__mul(a, b)
    -- Check for scalar multiplication
    if type(a) == "number" and vector.isvector(b) then
        return vector.new(a*b.x, a*b.y, a*b.z)
    elseif type(b) == "number" and vector.isvector(a) then
        return vector.new(a.x*b, a.y*b, a.z*b)
    elseif vector.isvector(a) and vector.isvector(b) then
        return vector.new(a.x*b.x, a.y*b.y, a.z*b.z)
    else
        assert(false, "vector mul - expected args: vector, vector or vector, number or number, vector")
    end
end

function vector.__div(a, b)
    assert(vector.isvector(a), "vector div - expected args: vector, vector or vector, number")
    if type(b) == "number" then
        return vector.new(a.x/b, a.y/b, a.z/b)
    else
        assert(vector.isvector(b), "vector div - expected args: vector, vector or vector, number")
        return vector.new(a.x/b.x, a.y/b.y, a.z/b.z)
    end
end

function vector.__eq(a, b)
    assert(vector.isvector(a) and vector.isvector(b), "vector eq - expected args: vector, vector")
    return a.x==b.x and a.y==b.y and a.z==b.z
end

function vector:length()
    return math.sqrt(self.x*self.x + self.y*self.y + self.z*self.z)
end

return setmetatable(vector, {
    __call = function(_, ...)
        return vector.new(...)
    end
})