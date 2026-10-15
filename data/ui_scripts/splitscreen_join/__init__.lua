-- Lets a second player join split-screen by pressing A / Cross on their
-- controller. The stock menus only handle this press on console.

if Engine.IsInGame() or not CoD.isPC then
  return
end

local gamepadButton = CoD.Menu.GamepadButton
CoD.Menu.GamepadButton = function(menu, event)
  if
    menu.menuName == "Lobby"
    and event.button == "primary"
    and event.down == true
    and not menu.m_inputDisabled
    and not menu.occludedBy
    and not Engine.IsControllerBeingUsed(event.controller)
    and LuaUtils.LobbyProcessQueueEmpty()
  then
    LobbyAddLocalClient(menu, event.controller)
    return true
  end
  return gamepadButton(menu, event)
end
