
local function test()
	print("")
	print("Testing Transform2...")

	-- Includes
	print("Including Vec2")
	local vec2 = require("Vec2")

	print("Including Transform2")
	local transform2 = require("Transform2")

	-- Initialize
	print("Setting t1 to ((1, 2), 0, (1, 1)).")
	print("t1 = transform2(vec2(1, 2), 0, vec2(1, 1))")
	local t1 = transform2(vec2(1, 2), 0, vec2(1, 1))
	print("Expected:	(p: (1, 2), r: 0, s: (1, 1))")
	print("Result:		"..tostring(t1))
	print("")
	
	print("Setting t2 to ((3, -4), 0.5, (1, 1.5)).")
	print("t2 = transform2(vec2(3, -4), 0.5, vec2(1, 1.5))")
	local t2 = transform2(vec2(3, -4), 0.5, vec2(1, 1.5))
	print("Expected:	(p: (3, -4), r: 0.5, s: (1, 1.5))")
	print("Result:		"..tostring(t2))
	print("")

	-- Meta events
	print("Are t1 and t2 equal?")
	print("t1 == t2")
	print("Expected:	false")
	print("Result:		"..tostring(t1 == t2))
	print("")

	print("Is t1 equal to an identically initialized transform?")
	print("t1 == transform2(vec2(1, 2), 0, vec2(1, 1))")
	print("Expected:	true")
	print("Result:		"..tostring(t1 == transform2(vec2(1, 2), 0, vec2(1, 1))))
	print("")
	
	-- Complete
	print("Testing Transform2 Complete")
	print("")
end

test()