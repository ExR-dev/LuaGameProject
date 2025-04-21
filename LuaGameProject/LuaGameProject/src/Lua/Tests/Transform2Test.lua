
local function test()
	local totalTestsTracker = 0
	local passedTestsTracker = 0
	local expectedVal = nil 
	local resultVal = nil

	-- Includes
	print("Including Vec2")
	local vec2 = require("Vec2")

	print("Including Transform2")
	local transform2 = require("Transform2")

	-- Initialize
	print("Setting t1 to ((1, 2), 0, (1, 1)).")
	print("t1 = transform2(vec2(1, 2), 0, vec2(1, 1))")
	local t1 = transform2(vec2(1, 2), 0, vec2(1, 1))
	print("")
	
	print("Setting t2 to ((3, -4), 0.5, (1, 1.5)).")
	print("t2 = transform2(vec2(3, -4), 0.5, vec2(1, 1.5))")
	local t2 = transform2(vec2(3, -4), 0.5, vec2(1, 1.5))
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
	
	return totalTestsTracker, passedTestsTracker
end

return test()