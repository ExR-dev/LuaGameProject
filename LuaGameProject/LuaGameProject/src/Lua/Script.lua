
-- Print all environment variables 
function PrintEnv ()
	for k,v in pairs(_G) do print(k,v) end
end