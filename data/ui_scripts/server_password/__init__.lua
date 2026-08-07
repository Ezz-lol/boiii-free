if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

local pendingJoin = nil

local function getServerModelValue(model, key)
  local keyModel = Engine.GetModel(model, key)
  if keyModel then
    return Engine.GetModelValue(keyModel)
  end
  return nil
end

local function getSavedPassword(controller)
  local saved = ""
  pcall(function()
    if Dvar.password then
      saved = Dvar.password:get() or ""
    end
  end)
  if saved == "" then
    pcall(function()
      saved = Engine.DvarString(controller, "password") or ""
    end)
  end
  return tostring(saved)
end

local function installPasswordPrefill()
  local loaded = pcall(require, "ui.uieditor.widgets.PC.Utility.InputTextField")
  if not loaded or not CoD.InputTextField or type(CoD.InputTextField.new) ~= "function" then
    return
  end
  if CoD.InputTextField.boiiiServerPasswordPrefillInstalled then
    return
  end

  local originalNew = CoD.InputTextField.new
  CoD.InputTextField.new = function(menu, controller)
    local prefill = BoiiiServerPasswordPrefill
    if prefill ~= nil then
      Engine.SetDvar("ui_keyboard_dvar_edit", prefill)
    end

    local field = originalNew(menu, controller)
    if prefill ~= nil then
      Engine.SetDvar("ui_keyboard_dvar_edit", prefill)
      if field.inputText then
        field.inputText:setText(prefill)
      end
      if field.dummyText then
        field.dummyText:setText(prefill)
      end
      if field.updateCursor then
        field.updateCursor()
      end
      BoiiiServerPasswordPrefill = nil
    end
    return field
  end
  CoD.InputTextField.boiiiServerPasswordPrefillInstalled = true
end

installPasswordPrefill()

local function connectToServer(controller, serverIndex, name, connectAddr)
  if serverIndex and name then
    Engine.SteamServerBrowser_AddFavoriteServer(serverIndex, true)
  end
  if connectAddr and connectAddr ~= "" then
    Engine.Exec(controller, "connect " .. connectAddr)
  end
end

local function ensureKeyboardHandler(menu)
  if not menu or menu.serverPasswordKeyboardHandler then
    return
  end
  menu.serverPasswordKeyboardHandler = true

  local previousHandler = menu.m_eventHandlers and menu.m_eventHandlers.ui_keyboard_input
  menu:registerEventHandler("ui_keyboard_input", function(element, event)
    if event.type == Enum.KeyboardType.KEYBOARD_TYPE_SERVER_PASSWORD and pendingJoin then
      local join = pendingJoin
      pendingJoin = nil
      local enteredPassword = event.input or ""
      Engine.SetDvar("password", enteredPassword)
      Engine.SetDvar("live_steam_server_password", enteredPassword)
      connectToServer(join.controller, join.serverIndex, join.name, join.connectAddr)
      GoBack(join.widget, join.controller)
      return true
    end
    if previousHandler then
      return previousHandler(element, event)
    end
    return false
  end)
end

JoinServerBrowser = function(widget, element, controller, menu)
  local model = element:getModel()
  if not model then
    return
  end

  local serverIndex = getServerModelValue(model, "serverIndex")
  local name = getServerModelValue(model, "name")
  local connectAddr = getServerModelValue(model, "connectAddr")

  if not getServerModelValue(model, "passwordProtected") then
    connectToServer(controller, serverIndex, name, connectAddr)
    GoBack(widget, controller)
    return
  end

  pendingJoin = {
    widget = widget,
    controller = controller,
    serverIndex = serverIndex,
    name = name,
    connectAddr = connectAddr,
  }

  ensureKeyboardHandler(menu or widget)

  local savedPassword = getSavedPassword(controller)
  Engine.SetDvar("live_steam_server_password", savedPassword)
  BoiiiServerPasswordPrefill = savedPassword
  ShowKeyboard(widget, element, controller, "KEYBOARD_TYPE_SERVER_PASSWORD")
end
