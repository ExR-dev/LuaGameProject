local runTest = true

local function test()
	local totalTestsTracker = 0
	local passedTestsTracker = 0
	local expectedVal = nil 
	local resultVal = nil
	
	-- Includes

	-- Initialize
    local time = 0.0
    local timeLimit = 7.0

    local testKeys = {
        W = Input.Key.KEY_W,
        H = Input.Key.KEY_H,
        Up_Arrow = Input.Key.KEY_UP,
        Enter = Input.Key.KEY_ENTER,
        Tab = Input.Key.KEY_TAB,
        Minus = Input.Key.KEY_MINUS
    }

	-- Meta events
	print("Check Not Pressed")
	expectedVal = false
    for k, v in pairs(testKeys) do
	    totalTestsTracker = totalTestsTracker + 1

	    print("Input.KeyPressed(" .. k .. ")")
        resultVal = Input.KeyPressed(v)
        print("Expected:	"..tostring(expectedVal))
        print("Result:		"..tostring(resultVal))
        if expectedVal == resultVal then
            print("Passed!")
            passedTestsTracker = passedTestsTracker + 1
        else
            print("Failed!")
	    end
    end
	print("")


	print("Check Is Pressed")
	print("Input.KeyPressed(key)")
	expectedVal = true
    for k, v in pairs(testKeys) do
	    totalTestsTracker = totalTestsTracker + 1
        resultVal = false
        time = os.clock()
        print('Press: "' .. k .. '" [In the game]')
        while (os.clock() - time) < timeLimit and resultVal == false do
            if Input.KeyPressed(v) == true then
                resultVal = true
            end
        end
        print("Expected:	"..tostring(expectedVal))
        print("Result:		"..tostring(resultVal))
        if expectedVal == resultVal then
            print("Passed!")
            passedTestsTracker = passedTestsTracker + 1
        else
            print("Failed!")
	    end
    end
	print("")

	return totalTestsTracker, passedTestsTracker
end

if not runTest then
    test = function() return 0, 0 end
end

return test()