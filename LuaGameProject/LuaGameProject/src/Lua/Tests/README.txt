Location of all lua tests. New files are detected and ran automatically. Test scripts must follow a specific format, as described below:
A test script must consist of a local function, containing all tests. The result of calling the function must then be returned at the end of the script.
The local function must return two values, the total amount of tests and the amount passed.


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
