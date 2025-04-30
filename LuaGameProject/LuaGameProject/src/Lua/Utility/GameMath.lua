local gameMath = {
	metersToPixels = 100.0,
	pixelsToMeters = 0.01
}

local vec2 = require("Vec2")

-- Approximate equality function with optional threshold parameter (exclusive)
function gameMath.approx(a, b, t)
	local threshold = t or 0.00001

	if vec2.isvec2(a) then
		return (
			(math.abs(a.x - b.x) < threshold) and 
			(math.abs(a.y - b.y) < threshold)
		)
	end

	return (math.abs(a - b) < threshold)
end

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

-- Framerate independent exponential decay function
--[[
	a - start value
	b - end value
	d - decay rate (common range 1 - 25)
	dT - delta time
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

-- Generate a random number in the range [-1, 1]
function gameMath.randomSigned()
	return (math.random() * 2.0) - 1.0
end

-- Generate a random number with a normal distribution in the range [-1, 1]
function gameMath.randomND()
   local x
   repeat
      x = (math.sqrt(math.log(1 / math.random())) * math.cos(math.pi * math.random()) * 1.5) / 5.0
   until x > -1.0 and x <= 1.0
   return x
end

-- Angle between two vectors in radians
function gameMath.angleBetween(a, b)
	assert(vec2.isvec2(a) and vec2.isvec2(b), "gameMath angleBetween - expected args: (vec2, vec2)")
	local dot = a:dot(b)
	local lenA = a:length()
	local lenB = b:length()
	if lenA == 0 or lenB == 0 then
		return 0
	end
	return math.deg(math.acos(dot / (lenA * lenB)))
end

-- Create a vector from an angle in degrees
function gameMath.vecFromAngle(angle)
	assert(type(angle) == "number", "gameMath vecFromAngle - expected args: (number)")
	angle = math.rad(angle)
	return vec2(math.cos(angle), math.sin(angle))
end

-- Remap a value from one range to another
function gameMath.remap(val, aMin, aMax, bMin, bMax)
	return (val - aMin) * (bMax - bMin) / (aMax - aMin) + bMin;
end

-- Interpolate between angles
function rot_lerp(a, b, weight)
    local shortest = ((a - b) + 90.0) % 270.0
    return (b + (shortest * weight) % 360.0)
end

return gameMath