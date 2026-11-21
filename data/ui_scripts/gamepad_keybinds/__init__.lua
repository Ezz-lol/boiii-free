if not CoD.isPC or not pcall(require, "ui.uieditor.widgets.StartMenu.StartMenu_Options_KeyBinder") then
  return
end

local buttons = Enum.LUIButton
local flags = Enum.LUIButtonFlags
local hasFlag = CoD.BitUtility.IsBitwiseAndNonZero
local keys = {
  [buttons.LUI_KEY_XBA_PSCROSS] = "BUTTON_A",
  [buttons.LUI_KEY_XBB_PSCIRCLE] = "BUTTON_B",
  [buttons.LUI_KEY_XBX_PSSQUARE] = "BUTTON_X",
  [buttons.LUI_KEY_XBY_PSTRIANGLE] = "BUTTON_Y",
  [buttons.LUI_KEY_BACK] = "BUTTON_BACK",
  [buttons.LUI_KEY_START] = "BUTTON_START",
  [buttons.LUI_KEY_LB] = "BUTTON_LSHLDR",
  [buttons.LUI_KEY_RB] = "BUTTON_RSHLDR",
  [buttons.LUI_KEY_LTRIG] = "BUTTON_LTRIG",
  [buttons.LUI_KEY_RTRIG] = "BUTTON_RTRIG",
  [buttons.LUI_KEY_LSTICK_PRESSED] = "BUTTON_LSTICK",
  [buttons.LUI_KEY_RSTICK_PRESSED] = "BUTTON_RSTICK",
  [buttons.LUI_KEY_UP] = "DPAD_UP",
  [buttons.LUI_KEY_DOWN] = "DPAD_DOWN",
  [buttons.LUI_KEY_LEFT] = "DPAD_LEFT",
  [buttons.LUI_KEY_RIGHT] = "DPAD_RIGHT",
}

local keyBinder = CoD.StartMenu_Options_KeyBinder
local newKeyBinder = keyBinder.new
keyBinder.new = function(menu, controller)
  local row = newKeyBinder(menu, controller)
  local selectButton = buttons.LUI_KEY_XBA_PSCROSS
  local originalSelect = row.buttonFunctions[selectButton]
  local capture
  local startTimer
  local timeout
  local previousButtonActions

  local function stopCapture()
    if not capture then
      return
    end
    local listener = capture
    capture = nil
    listener:close()
    if startTimer then
      startTimer:close()
      startTimer = nil
    end
    if timeout then
      timeout:close()
      timeout = nil
    end
    menu.m_disableAllButtonActions = previousButtonActions
  end

  local onSourceChanged = row.m_eventHandlers.input_source_changed
  row:registerEventHandler("input_source_changed", function(element, event)
    stopCapture()
    return onSourceChanged(element, event)
  end)
  LUI.OverrideFunction_CallOriginalFirst(row, "close", stopCapture)

  row.buttonFunctions[selectButton] = function(element, activeMenu, inputController, ...)
    if not Engine.LastInput_Gamepad(inputController) then
      return originalSelect(element, activeMenu, inputController, ...)
    end
    if activeMenu.m_disableNavigation or not activeMenu:AcceptGamePadButtonInputFromModelCallback(inputController) then
      return
    end
    local model = row:getModel()
    local command = model and Engine.GetModelValue(Engine.GetModel(model, "command"))
    local bindIndex = model and Engine.GetModelValue(Engine.GetModel(model, "bindIndex"))
    -- Stock PC controls use the player bind slot. The key name comes from keys;
    -- only allow a single action and its optional numeric argument in the command.
    if
      capture
      or bindIndex ~= CoD.BIND_PLAYER
      or type(command) ~= "string"
      or not command:match("^%+?[%w_]+ ?%d*$")
    then
      return true
    end

    row:setState("WaitingForBind")
    row.keybindText:setText(Engine.Localize("KEY_UNBOUND"))
    previousButtonActions = menu.m_disableAllButtonActions
    menu.m_disableAllButtonActions = true
    capture = LUI.UIElement.new()
    row:addElement(capture)

    -- Subscribe after the Select dispatch has finished. A held button can send
    -- repeats, and stick navigation shares the directional models with the D-pad.
    startTimer = LUI.UITimer.newElementTimer(1, true, function()
      startTimer = nil
      local root = Engine.GetModelForController(inputController)
      for button, key in pairs(keys) do
        local keyName = key
        local buttonModel = Engine.CreateModel(root, "ButtonBits." .. button)
        capture:subscribeToModel(buttonModel, function(changed)
          local bits = Engine.GetModelValue(changed) or 0
          if
            not capture
            or menu.occludedBy
            or not Engine.LastInput_Gamepad(inputController)
            or not hasFlag(bits, flags.FLAG_DOWN)
            or hasFlag(bits, flags.FLAG_ISREPEAT)
            or (keyName:sub(1, 5) == "DPAD_" and not hasFlag(bits, flags.FLAG_DPAD))
          then
            return
          end
          Engine.SetModelValue(changed, 0)
          Engine.ExecNow(inputController, "bind " .. keyName .. ' "' .. command .. '"')
          stopCapture()
          menu:dispatchEventToChildren({ name = "options_refresh", controller = inputController })
        end, false)
      end
    end)
    row:addElement(startTimer)
    timeout = LUI.UITimer.newElementTimer(10000, true, function()
      timeout = nil
      stopCapture()
      row:processEvent({ name = "options_refresh", controller = inputController })
    end)
    row:addElement(timeout)
    return true
  end
  return row
end
