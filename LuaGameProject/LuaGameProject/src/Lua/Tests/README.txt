Location of all lua tests. New files are detected and ran automatically. Test scripts must follow a specific format, as described below:
A test script must consist of a local function, containing all tests. The function must then be called at the end of the script.


Test Template:
	print("[Test Description]")
	print("[Exact Procedure]")
	print("Expected:	[value]")
	print("Result:		"..tostring([Procedure]))
	print("")

Copyable Format:
	print("")
	print("")
	print("Expected:	")
	print("Result:		"..tostring())
	print("")

Example Usage:
	print("Adding v2 to v1.")
	print("v1 + v2")
	print("Expected:	(4.5, -2.25)")
	print("Result:		"..tostring(v1 + v2))
	print("")
	
	
Full Example File:
	local function test()
		print("")
		print("Testing Example...")

		-- Includes
		print("Including Example")
		local example = require("Example")

		-- Initialize
		print("Setting e1 to 1.")
		print("e1 = example(1)")
		local e1 = example(1)
		print("Expected:	(1)")
		print("Result:		"..tostring(e1))
		print("")

		-- Meta events
		print("Is e1 equal to 2?")
		print("e1 == 2")
		print("Expected:	false")
		print("Result:		"..tostring(e1 == 2))
		print("")

		-- Complete
		print("Testing Example Complete")
		print("")
	end

	test()
