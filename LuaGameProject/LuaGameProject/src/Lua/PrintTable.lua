
-- Taken from noita modding tools
function DebugPrintTable(table_to_print, table_depth, parent_table)
	local table_depth_ = table_depth or 1
	local parent_table_ = parent_table or "TABLE"
	local result = parent_table_ .. ": "
	
	if (table_depth_ > 1) then
		for i=1,table_depth_ - 1 do
			result = result .. " - "
		end
	end
	
	local subtables = {}
	
	if (table_to_print ~= nil) and (tostring(type(table_to_print)) == "table") then
		for i,v in pairs(table_to_print) do
			result = result .. tostring(i) .. "(" .. tostring(v) .. "), "
			
			if (tostring(type(v)) == "table") then
				table.insert(subtables, {i, v})
			end
		end
	end
	
	print( result )
	
	for i,v in ipairs( subtables ) do
		DebugPrintTable( v[2], table_depth_ + 1, "subtable " .. v[1] )
	end
end

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

function PrintTable(table, name)
	io.write((name or "table").." = ")
	PrintTableRec(table, "", 0)
end
