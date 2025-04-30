
local function test()
	local totalTestsTracker = 0
	local passedTestsTracker = 0
	local expectedVal = nil 
	local resultVal = nil

	-- Includes
	print("Including GameMath")
	local gameMath = require("Utility/GameMath")

	print("Including Vec2")
	local vec2 = require("Vec2")

	print("Including Transform2")
	local transform2 = require("Transform2")

	print("")

	-- Initialize
	print("Setting t1 to ((1, 2), 0, (1, 1)).")
	print("t1 = transform2(vec2(1, 2), 0, vec2(1, 1))")
	local t1 = transform2(vec2(1, 2), 0, vec2(1, 1))
	print("")
	
	print("Setting t2 to ((3, -4), 45.0, (1, 1.5)).")
	print("t2 = transform2(vec2(3, -4), 45.0, vec2(1, 1.5))")
	local t2 = transform2(vec2(3, -4), 45.0, vec2(1, 1.5))
	print("")

	-- Meta events
	totalTestsTracker = totalTestsTracker + 1
	print("Are t1 and t2 equal?")
	print("t1 == t2")
	expectedVal = false
	resultVal = t1 == t2
	print("Expected:	"..tostring(expectedVal))
	print("Result:		"..tostring(resultVal))
	if expectedVal == resultVal then
		print("Passed!")
		passedTestsTracker = passedTestsTracker + 1
	else
		print("Failed!")
	end
	print("")

	totalTestsTracker = totalTestsTracker + 1
	print("Is t1 equal to an identically initialized transform?")
	print("t1 == transform2(vec2(1, 2), 0, vec2(1, 1))")
	expectedVal = true
	resultVal = t1 == transform2(vec2(1, 2), 0, vec2(1, 1))
	print("Expected:	"..tostring(expectedVal))
	print("Result:		"..tostring(resultVal))
	if expectedVal == resultVal then
		print("Passed!")
		passedTestsTracker = passedTestsTracker + 1
	else
		print("Failed!")
	end
	print("")

	totalTestsTracker = totalTestsTracker + 1
	print("Getting forward vector of t1.")
	print("t1:getForward()")
	expectedVal = vec2(1.0, 0.0)
	resultVal = t1:getForward()
	print("Expected:	"..tostring(expectedVal))
	print("Result:		"..tostring(resultVal))
	if gameMath.approx(expectedVal, resultVal) then
		print("Passed!")
		passedTestsTracker = passedTestsTracker + 1
	else
		print("Failed!")
	end
	print("")

	totalTestsTracker = totalTestsTracker + 1
	print("Getting right vector of t2.")
	print("t2:getRight()")
	expectedVal = vec2(math.cos(math.rad(-45.0)), math.sin(math.rad(-45.0)))
	resultVal = t2:getRight()
	print("Expected:	"..tostring(expectedVal))
	print("Result:		"..tostring(resultVal))
	if gameMath.approx(expectedVal, resultVal) then
		print("Passed!")
		passedTestsTracker = passedTestsTracker + 1
	else
		print("Failed!")
	end
	print("")

	totalTestsTracker = totalTestsTracker + 1
	print("Moving t1 in local space by vector (0, 3).")
	print("t1:moveRelative(vec2(0, 3))")
	t1:moveRelative(vec2(0, 3))
	expectedVal = transform2(vec2(4.0, 2.0), 0, vec2(1, 1))
	resultVal = t1
	print("Expected:	"..tostring(expectedVal))
	print("Result:		"..tostring(resultVal))
	if expectedVal == resultVal then
		print("Passed!")
		passedTestsTracker = passedTestsTracker + 1
	else
		print("Failed!")
	end
	print("")

	totalTestsTracker = totalTestsTracker + 1
	print("Moving t2 in local space by vector (-2.0, 0).")
	print("t2:moveRelative(vec2(-2.0, 0))")
	t2:moveRelative(vec2(-2.0, 0))
	expectedVal = transform2(vec2(3.0 + (-2.0 * math.cos(math.rad(-45.0))), -4.0 + (-2.0 * math.sin(math.rad(-45.0)))), 45.0, vec2(1, 1.5))
	resultVal = t2
	print("Expected:	"..tostring(expectedVal))
	print("Result:		"..tostring(resultVal))
	if expectedVal == resultVal then
		print("Passed!")
		passedTestsTracker = passedTestsTracker + 1
	else
		print("Failed!")
	end
	print("")
		
	return totalTestsTracker, passedTestsTracker
end

return test()