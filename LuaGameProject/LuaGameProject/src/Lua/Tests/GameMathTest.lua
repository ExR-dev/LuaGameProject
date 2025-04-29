
local function PrintBar(char, len, prefix, suffix)
	local bar = prefix or ""

	for i = 1, len do
		bar = bar..char
	end

	bar = bar..(suffix or "")
	print(bar)
end

local function TestRandomND()
	local gMath = require("Utility/GameMath")

	local ranges = { }
	local bars = 15
	local hBars = bars / 2.0
	local fullBarLength = 30
	local barMultiplier = 4.0

	for i = 1, bars do
		ranges[i] = 0
	end

	local samples = 500000
	for i = 1, samples do
		local val = gMath.randomND()
		local index = math.floor(((val + 1.0) * hBars)) + 1
		index = math.min(index, bars)
		index = math.max(index, 1)
		ranges[index] = ranges[index] + 1
	end

	local invSamples = 1.0 / samples

	for i = 1, bars do
		local barLengthUncut = invSamples * barMultiplier * fullBarLength * ranges[i]
		local barLength = math.floor(barLengthUncut)
		local rest = barLengthUncut - barLength

		barLength = math.min(barLength, fullBarLength)
		barLength = math.max(barLength, 0)

		local prefix = tostring(i).."\t|"

		local suffix = nil
		if rest < 0.05 then
			suffix = ""
		elseif rest < 0.2 then
			suffix = "."
		elseif rest < 0.4 then
			suffix = "+"
		elseif rest < 0.6 then
			suffix = "x"
		elseif rest < 0.8 then
			suffix = "X"
		else
			suffix = "#"
		end

		PrintBar("#", barLength, prefix, suffix)
	end
end


local function test()
	local totalTestsTracker = 0
	local passedTestsTracker = 0
	local expectedVal = nil 
	local resultVal = nil
	
	-- Includes
	print("Including Vec2")
	local vec2 = require("Vec2")

	print("Including GameMath")
	local gameMath = require("Utility/GameMath")

	print("")

	-- Initialize
	print("Setting i to 7")
	print("i = 7")
	local i = 7
	print("")

	print("Setting f to 12.45")
	print("f = 12.45")
	local f = 12.45
	print("")

	print("Setting v to (2.8, -5)")
	print("v = vec2(2.8, -5)")
	local v = vec2(2.8, -5)
	print("")

	-- Meta events
	totalTestsTracker = totalTestsTracker + 1
	print("Approximately comparing i to 7.")
	print("gameMath.approx(i, 7)")
	expectedVal = true
	resultVal = gameMath.approx(i, 7)
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
	print("Approximately comparing i to -7.")
	print("gameMath.approx(i, -7)")
	expectedVal = false
	resultVal = gameMath.approx(i, -7)
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
	print("Approximately comparing i to 8, with threshold of 1.")
	print("gameMath.approx(i, 8, 1)")
	expectedVal = false
	resultVal = gameMath.approx(i, 8, 1)
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
	print("Approximately comparing i to 7.001, with a threshold of 0.1.")
	print("gameMath.approx(i, 7.001, 0.1)")
	expectedVal = true
	resultVal = gameMath.approx(i, 7.001, 0.1)
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
	print("Approximately comparing f to 12.450002848.")
	print("gameMath.approx(f, 12.450002848)")
	expectedVal = true
	resultVal = gameMath.approx(f, 12.450002848)
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
	print("Approximately comparing f to 12.449991.")
	print("gameMath.approx(f, 12.449991)")
	expectedVal = true
	resultVal = gameMath.approx(f, 12.449991)
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
	print("Approximately comparing f to 12.4499.")
	print("gameMath.approx(f, 12.4499)")
	expectedVal = false
	resultVal = gameMath.approx(f, 12.4499)
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
	print("Approximately comparing 1/3 to 0.333, with a threshold of 0.001.")
	print("gameMath.approx(1.0 / 3.0, 0.333, 0.001)")
	expectedVal = true
	resultVal = gameMath.approx(1.0 / 3.0, 0.333, 0.001)
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
	print("Approximately comparing v to (2.799999, -5).")
	print("gameMath.approx(v, vec2(2.799999, -5))")
	expectedVal = true
	resultVal = gameMath.approx(v, vec2(2.799999, -5))
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
	print("Approximately comparing v to (2.8, -5.1).")
	print("gameMath.approx(v, vec2(2.8, -5.1))")
	expectedVal = false
	resultVal = gameMath.approx(v, vec2(2.8, -5.1))
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
	print("Lerping 0.0 to 2.5 by time 1.0.")
	print("gameMath.lerp(0.0, 2.5, 1.0)")
	expectedVal = 2.5
	resultVal = gameMath.lerp(0.0, 2.5, 1.0)
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
	print("Lerping f to -5.38 by time 0.3.")
	print("gameMath.lerp(f, -5.3, 0.3)")
	expectedVal = 7.125
	resultVal = gameMath.lerp(f, -5.3, 0.3)
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
	print("Lerping v to (0, -2.5) by time 2.0.")
	print("gameMath.lerp(v, vec2(0.0, -2.5), 2.0)")
	expectedVal = vec2(-2.8, 0.0)
	resultVal = gameMath.lerp(v, vec2(0.0, -2.5), 2.0)
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
	print("Exponentially decaying 1 to 0 by delta time 1/60 with a rate of 12.")
	print("gameMath.expDecay(1.0, 0.0, 12.0, 1.0 / 60.0)")
	expectedVal = 0.818730753078
	resultVal = gameMath.expDecay(1.0, 0.0, 12.0, 1.0 / 60.0)
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
	print("Exponentially decaying (2, -7.41) to (1.5, 40.0) by delta time 0.1 with a rate of 4.")
	print("gameMath.expDecay(vec2(2, -7.41), vec2(1.5, 40.0), 4.0, 0.1)")
	expectedVal = vec2(1.8351600230178, 8.2201266174503)
	resultVal = gameMath.expDecay(vec2(2, -7.41), vec2(1.5, 40.0), 4.0, 0.1)
	print("Expected:	"..tostring(expectedVal))
	print("Result:		"..tostring(resultVal))
	if gameMath.approx(expectedVal, resultVal) then
		print("Passed!")
		passedTestsTracker = passedTestsTracker + 1
	else
		print("Failed!")
	end
	print("")

	-- Manual test
	print("Testing distribution of randomND. Verify manually by observing curve.")
	print("TestRandomND()")
	TestRandomND()
	print("")

	-- TODO: Add tests for angleBetween and vecFromAngle

	return totalTestsTracker, passedTestsTracker
end

return test()