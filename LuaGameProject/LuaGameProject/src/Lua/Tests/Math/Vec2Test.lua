
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

	print("")

	-- Initialize
	print("Setting v1 to (1.5, 2)")
	print("v1 = vec2(1.5, 2)")
	local v1 = vec2(1.5, 2)
	print("")
	
	print("Setting v2 to (3, -4.25)")
	print("v2 = vec2(3, -4.25)")
	local v2 = vec2(3, -4.25)
	print("")

	-- Meta events
	totalTestsTracker = totalTestsTracker + 1
	print("Negating v1.")
	print("-v1")
	expectedVal = vec2(-1.5, -2)
	resultVal = -v1
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
	print("Adding v2 to v1.")
	print("v1 + v2")
	expectedVal = vec2(4.5, -2.25)
	resultVal = v1 + v2
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
	print("Subtracting v2 from v1.")
	print("v1 - v2")
	expectedVal = vec2(-1.5, 6.25)
	resultVal = v1 - v2
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
	print("Multiplying v1 by 2.")
	print("v1 * 2")
	expectedVal = vec2(3.0, 4)
	resultVal = v1 * 2
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
	print("Multiplying v1 by v2.")
	print("v1 * v2")
	expectedVal = vec2(4.5, -8.5)
	resultVal = v1 * v2
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
	print("Dividing v1 by 3.")
	print("v1 / 3")
	expectedVal = vec2(0.5, 2 / 3)
	resultVal = v1 / 3
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
	print("Dividing v1 by v2.")
	print("v1 / v2")
	expectedVal = vec2(0.5, 2 / -4.25)
	resultVal = v1 / v2
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
	print("Is v1 equal to v2?")
	print("v1 == v2")
	expectedVal = false
	resultVal = (v1 == v2)
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
	print("Is v1 equal to an identically initialized vector?")
	print("v1 == vec2(1.5, 2)")
	expectedVal = true
	resultVal = (v1 == vec2(1.5, 2))
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
	print("Getting squared length of v1.")
	print("v1:lengthSqr()")
	expectedVal = 6.25
	resultVal = v1:lengthSqr()
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
	print("Getting length of v1.")
	print("v1:length()")
	expectedVal = 2.5
	resultVal = v1:length()
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
	print("Getting dot product of v1 and v2.")
	print("vec2.dot(v1, v2)")
	expectedVal = -4.0
	resultVal = vec2.dot(v1, v2)
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
	print("Getting dot product of v1 and (3.7, -2.25) using instance method.")
	print("v1:dot(vec2(3.7, -2.25))")
	expectedVal = 1.05
	resultVal = v1:dot(vec2(3.7, -2.25))
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
	print("Getting v1 as normalized vector.")
	print("v1:normalized()")
	expectedVal = vec2(0.6, 0.8)
	resultVal = v1:normalized()
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
	print("Has v1 been modified?")
	print("not (v1 == vec2(1.5, 2))")
	expectedVal = false
	resultVal = not (v1 == vec2(1.5, 2))
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
	print("Normalizing v2 in-place.")
	print("v2:normalize()")
	v2:normalize()
	expectedVal = vec2(3 / math.sqrt(27.0625), -4.25 / math.sqrt(27.0625))
	resultVal = v2
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
	print("Getting angle of v1.")
	print("v1:angle()")
	print(tostring(v1))
	expectedVal = math.deg(0.9272952180016)
	resultVal = v1:angle()
	print("Expected:	"..tostring(expectedVal))
	print("Result:		"..tostring(resultVal))
	if gameMath.approx(expectedVal, resultVal) then
		print("Passed!")
		passedTestsTracker = passedTestsTracker + 1
	else
		print("Failed!")
	end
	print("")
	
	return totalTestsTracker, passedTestsTracker
end

return test()