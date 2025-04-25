local gameMath = {}

local vec2 = require("Vec2")

function gameMath.lerp(a, b, t)
    assert(type(t) == "number", "gameMath lerp - expected args: (number, number, number) or (vec2, vec2, number)")

    if vec2.isvec2(a) then
        assert(vec2.isvec2(b), "gameMath lerp - expected args: (number, number, number) or (vec2, vec2, number)")
        return vec2(
            a.x + (b.x - a.x) * t, 
            a.y + (b.y - a.y) * t
        )
    else
        assert(type(a) == "number" and type(b) == "number", "gameMath lerp - expected args: (number, number, number) or (vec2, vec2, number)")
        return (a + (b - a) * t)
    end
end

--[[
Framerate independent exponential decay function
    a = start value
    b = end value
    d = decay rate (common range 1 - 25)
    dT = delta time
--]]
function gameMath.expDecay(a, b, d, dT)
    assert(type(d) == "number" and type(dT) == "number", "gameMath expDecay - expected args: (number, number, number, number) or (vec2, vec2, number, number)")

    if vec2.isvec2(a) then
        assert(vec2.isvec2(b), "gameMath expDecay - expected args: (number, number, number, number) or (vec2, vec2, number, number)")
        return vec2(
            (b.x + (a.x - b.x) * math.exp(-d * dT)),
            (b.y + (a.y - b.y) * math.exp(-d * dT))
        )
    else
        assert(type(a) == "number" and type(b) == "number", "gameMath expDecay - expected args: (number, number, number, number) or (vec2, vec2, number, number)")
        return (b + (a - b) * math.exp(-d * dT))
    end
end

return gameMath