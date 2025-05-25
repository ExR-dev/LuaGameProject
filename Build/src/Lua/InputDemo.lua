
local function UpdateInput()
    if (Input.KeyPressed(Input.Key.KEY_W) == true) then
        print("W - Pressed")
    end
    if (Input.KeyPressed(Input.Key.KEY_A) == true) then
        print("A - Pressed")
    end
    if (Input.KeyPressed(Input.Key.KEY_S) == true) then
        print("S - Pressed")
    end
    if (Input.KeyPressed(Input.Key.KEY_D) == true) then
        print("D - Pressed")
    end
    if (Input.KeyHeld(Input.Key.KEY_W) == true) then
        print("W - Held")
    end
    if (Input.KeyHeld(Input.Key.KEY_A) == true) then
        print("A - Held")
    end
    if (Input.KeyHeld(Input.Key.KEY_S) == true) then
        print("S - Held")
    end
    if (Input.KeyHeld(Input.Key.KEY_D) == true) then
        print("D - Held")
    end
    if (Input.KeyReleased(Input.Key.KEY_W) == true) then
        print("W - Released")
    end
    if (Input.KeyReleased(Input.Key.KEY_A) == true) then
        print("A - Relased")
    end
    if (Input.KeyReleased(Input.Key.KEY_S) == true) then
        print("S - Released")
    end
    if (Input.KeyReleased(Input.Key.KEY_D) == true) then
        print("D - Released")
    end

    local info = Input.GetMouseInfo();
    print("Position : (" .. info.Position.x .. ", " .. info.Position.y .. ")")
    print("Delta    : (" .. info.Delta.x .. ", " .. info.Delta.y .. ")")
    print("Scroll   :  " .. info.Scroll)

end

return UpdateInput