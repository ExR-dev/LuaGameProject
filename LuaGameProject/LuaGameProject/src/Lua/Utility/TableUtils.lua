require("Utility/TableSave")



local function TableToStringRec(tbl, indent, depth)
	local str = ""

	if depth < 8 then
		str = str.."{\n"
		local innerIndent = indent..":   "

		for key, val in pairs(tbl) do
			str = str..innerIndent..key.." = "

			if type(val) == "table" then
				str = str..TableToStringRec(val, innerIndent, depth + 1)
			else
				str = str..tostring(val).." ("..type(val)..")".."\n"
			end
		end

		str = str..indent.."}\n"
	else
		str = str..indent.."...\n"
	end

	return str
end


function table.toString(tbl)
	return TableToStringRec(tbl, "", 0)
end

function table.len(tbl)
	local count = 0
	for _ in pairs(tbl) do count = count + 1 end
	return count
end

function table.hasKey(tbl, key)
	for k, _ in pairs(tbl) do
		if k == key then
			return true
		end
	end
	return false
end

function table.hasValue(tbl, val)
	for i, v in pairs(tbl) do
		if v == val then
			return true, i
		end
	end
	return false
end


function table.copy(orig)
	local orig_type = type(orig)
	local copy
	if orig_type == 'table' then
		copy = {}
		for orig_key, orig_value in pairs(orig) do
			copy[orig_key] = orig_value
		end
	else -- number, string, boolean, etc
		copy = orig
	end
	return copy
end

-- Save copied tables in 'copies', indexed by original table.
function table.deepCopy(orig, copies)
	copies = copies or {}
	local orig_type = type(orig)
	local copy
	if orig_type == 'table' then
		if copies[orig] then
			copy = copies[orig]
		else
			copy = {}
			copies[orig] = copy
			for orig_key, orig_value in next, orig, nil do
				copy[table.deepCopy(orig_key, copies)] = table.deepCopy(orig_value, copies)
			end
			setmetatable(copy, table.deepCopy(getmetatable(orig), copies))
		end
	else -- number, string, boolean, etc
		copy = orig
	end
	return copy
end