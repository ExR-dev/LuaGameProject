require("Utility/TableSave")


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