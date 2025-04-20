
local function test()
	print("")
	print("Testing Vec2...")
	
	-- Includes
	print("Including Vec2")
	local vec2 = require("Vec2")

	-- Initialize
	print("Setting v1 to (1.5, 2)")
	print("v1 = vec2(1.5, 2)")
	local v1 = vec2(1.5, 2)
	print("Expected:	(1.5, 2)")
	print("Result:		"..tostring(v1))
	print("")
	
	print("Setting v2 to (3, -4.25)")
	print("v2 = vec2(3, -4.25)")
	local v2 = vec2(3, -4.25)
	print("Expected:	(3, -4.25)")
	print("Result:		"..tostring(v2))
	print("")

	-- Meta events
	print("Negating v1.")
	print("-v1")
	print("Expected:	(-1.5, -2)")
	print("Result:		"..tostring(-v1))
	print("")

	print("Adding v2 to v1.")
	print("v1 + v2")
	print("Expected:	(4.5, -2.25)")
	print("Result:		"..tostring(v1 + v2))
	print("")
	
	print("Subtracting v2 from v1.")
	print("v1 - v2")
	print("Expected:	(-1.5, 6.25)")
	print("Result:		"..tostring(v1 - v2))
	print("")
	
	print("Multiplying v1 by 2.")
	print("v1 * 2")
	print("Expected:	(3.0, 4)")
	print("Result:		"..tostring(v1 * 2))
	print("")
	
	print("Multiplying v1 by v2. (v1 * v2)")
	print("Expected:	(4.5, -8.5)")
	print("Result:		"..tostring(v1 * v2))
	print("")

	print("Dividing v1 by 3. (v1 / 3)")
	print("Expected:	(0.5, 0.66666666666667)")
	print("Result:		"..tostring(v1 / 3))
	print("")
	
	print("Dividing v1 by v2. (v1 / v2)")
	print("Expected:	(0.5, -0.47058823529412)")
	print("Result:		"..tostring(v1 / v2))
	print("")
	
	print("Is v1 equal to v2? (v1 == v2)")
	print("Expected:	false")
	print("Result:		"..tostring(v1 == v2))
	print("")
	
	print("Is v1 equal to an identically initialized vector? (v1 == (1.5, 2))")
	print("Expected:	true")
	print("Result:		"..tostring(v1 == vec2(1.5, 2)))
	print("")
	
	print("Getting squared length of v1. (v1:lengthSqr())")
	print("Expected:	6.25")
	print("Result:		"..tostring(v1:lengthSqr()))
	print("")
	
	print("Getting length of v1. (v1:length())")
	print("Expected:	2.5")
	print("Result:		"..tostring(v1:length()))
	print("")

	print("Getting dot product of v1 and v2. (vec2.dot(v1, v2))")
	print("Expected:	-4.0")
	print("Result:		"..tostring(vec2.dot(v1, v2)))
	print("")
	
	print("Getting v1 as normalized vector. (v1:normalized())")
	print("Expected:	(0.6, 0.8)")
	print("Result:		"..tostring(v1:normalized()))
	print("")

	print("Has v1 been modified? (v1 != vec2(1.5, 2))")
	print("Expected:	false")
	print("Result:		"..tostring(not (v1 == vec2(1.5, 2))))
	print("")
	
	print("Normalizing v2 in-place. (v2:normalize())")
	v2:normalize()
	print("Expected:	(0.57668319759866, -0.81696786326476)")
	print("Result:		"..tostring(v2))
	print("")
	
	-- Complete
	print("Testing Vec2 Complete")
	print("")
end

test()