Location of all lua tests. New files are detected and ran automatically. Test scripts should follow a specific format, as described below:
A test script consists of a local function which performs all tests.
The function must return two values (in the given order): 
	1. The total amount of tests done.
	2. The amount of tests passed.
The result of calling the function must be returned at the end of the script.


Test Template:
	totalTestsTracker = totalTestsTracker + 1
	print("[Test Description]")
	print("[Procedure]")
	expectedVal = [Expected]
	resultVal = [Procedure]
	print("Expected:	"..tostring(expectedVal))
	print("Result:		"..tostring(resultVal))
	if expectedVal == resultVal
		print("Passed!")
		passedTestsTracker = passedTestsTracker + 1
	else
		print("Failed!")
	print("")

Copyable Format:
	totalTestsTracker = totalTestsTracker + 1
	print("")
	print("")
	expectedVal = 
	resultVal = 
	print("Expected:	"..tostring(expectedVal))
	print("Result:		"..tostring(resultVal))
	if expectedVal == resultVal
		print("Passed!")
		passedTestsTracker = passedTestsTracker + 1
	else
		print("Failed!")
	print("")

Example Usage:
	totalTestsTracker = totalTestsTracker + 1
	print("Adding v2 to v1.")
	print("v1 + v2")
	print("Expected:	"..tostring(expectedVal))
	print("Result:		"..tostring(resultVal))
	if expectedVal == resultVal
		print("Passed!")
		passedTestsTracker = passedTestsTracker + 1
	else
		print("Failed!")
	print("")
	


Full Example File:
	local function test()
		local totalTestsTracker = 0
		local passedTestsTracker = 0
		local expectedVal = nil 
		local resultVal = nil

		-- Includes
		print("Including Example")
		local example = require("Example")

		print("")

		-- Initialize
		print("Setting e1 to 1.")
		print("e1 = example(1)")
		local e1 = example(1)
		print("")

		-- Tests
		totalTestsTracker = totalTestsTracker + 1
		print("Is e1 equal to 2?")
		print("e1 == 2")
		expectedVal = false
		resultVal = (e1 == 2)
		print("Expected:	"..tostring(expectedVal))
		print("Result:		"..tostring(resultVal))
		if expectedVal == resultVal
			print("Passed!")
			passedTestsTracker = passedTestsTracker + 1
		else
			print("Failed!")
		print("")

		return totalTestsTracker, passedTestsTracker
	end

	return test()
