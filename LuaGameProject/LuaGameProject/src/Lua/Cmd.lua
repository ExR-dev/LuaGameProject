-- Transient script for executing various one-off console commands through 'f:Cmd'.
-- Modify as desired, no matter it's current contents; If it's anything worth keeping, it doesn't belong in here.


local function MultiReturn()
	return true, false
end


if MultiReturn() then
	print("Works")
else
	print("Shit dont work")
end