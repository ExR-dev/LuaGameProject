
local function PrintTableRec(table, indent, depth)
	if depth < 8 then
		io.write("{\n")
		local innerIndent = indent..":   "

		for key, val in pairs(table) do
			io.write(innerIndent..key.." = ")

			if type(val) == "table" then
				PrintTableRec(val, innerIndent, depth + 1)
			else
				io.write(tostring(val).."\n")
			end
		end

		io.write(indent.."}\n")
	else
		io.write(indent.."...\n")
	end
end

function PrintTable(table)
	io.write("table = ")
	PrintTableRec(table, "", 0)
end
