local color = {}
color.__index = color

function color.new(r, g, b, a)
    local t = {
        r = r or 1.0,
        g = g or 1.0,
        b = b or 1.0,
        a = a or 1.0
    }
    return setmetatable(t, color)
end

function color.iscolor(t)
    return getmetatable(t) == color
end

function color.__newindex(t, k, v)
    print("color - not possible to assign new fields")
end

function color.__tostring(t)
    return "color("..t.r..", "..t.g..", "..t.b..", "..t.a..")"
end

-- Meta logic operators
function color.__eq(a, b)
    assert(color.iscolor(a) and color.iscolor(b), "color eq - expected args: color, color")
    return a.r==b.r and a.g==b.g and a.b==b.b and a.a==b.a
end

return setmetatable(color, {
    __call = function(_, ...)
        return color.new(...)
    end
})