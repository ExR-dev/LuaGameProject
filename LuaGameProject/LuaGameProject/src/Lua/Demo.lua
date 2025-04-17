
-- Vec2 ----------------------------------------------------
print("")
print("Testing Vec2...")
local vec2 = require("Vec2")

-- Initialize
local v1 = vec2(1.5, 2)
print("v1 = "..tostring(v1))

local v2 = vec2(3, -4.25)
print("v2 = "..tostring(v2))

-- Meta events
local v3 = -v1
print("-v1 = "..tostring(v3))

v3 = v1 + v2
print("v1 + v2 = "..tostring(v3))

v3 = v1 - v2
print("v1 - v2 = "..tostring(v3))

v3 = v1 * 2
print("v1 * 2 = "..tostring(v3))

v3 = v1 * v2
print("v1 * v2 = "..tostring(v3))

v3 = v1 / 3
print("v1 / 3 = "..tostring(v3))

v3 = v1 / v2
print("v1 / v2 = "..tostring(v3))

print("v1 == v2 = "..tostring(v1 == v2))

print("v1 == (1.5, 2) = "..tostring(v1 == vec2(1.5, 2)))

print("v1:lengthSqr() = "..tostring(v1:lengthSqr()))

print("v1:length() = "..tostring(v1:length()))

print("vec2.dot(v1, v2) = "..tostring(vec2.dot(v1, v2)))

print("v1:normalized() = "..tostring(v1:normalized()))
print("v1 = "..tostring(v1))

print("v2:normalize() = "..tostring(v2:normalize()))
print("v2 = "..tostring(v2))

------------------------------------------------------------


-- Transform2 ----------------------------------------------
print("")
print("Testing Transform2...")
local transform2 = require("Transform2")

-- Initialize
local t1 = transform2(vec2(1, 2), 0, vec2(1, 1))
print("t1 = "..tostring(t1))

local t2 = transform2(vec2(3, -4), 0.5, vec2(1, 1.5))
print("t2 = "..tostring(t2))

-- Meta events
print("t1 == t2 = "..tostring(t1 == t2))

print("t1 == (p: (1, 2), r: 0, s: (1, 1)) = "..tostring(t1 == transform2(vec2(1, 2), 0, vec2(1, 1))))

------------------------------------------------------------
