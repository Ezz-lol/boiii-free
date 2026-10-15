-- Lets a second player join split-screen by pressing A / Cross on their
-- controller, like on console, instead of clicking the split-screen button.

if Engine.IsInGame() or not game or not game.getgamepadjoinpress then
  return
end

require("ui.uieditor.menus.Lobby.Lobby")

local createLobby = LUI.createMenu.Lobby
if not createLobby then
  return
end

local function canJoin(menu)
  return not menu.occludedBy
    and not Engine.IsControllerBeingUsed(1)
    and LuaUtils.LobbyProcessQueueEmpty()
    and CoD.LobbyBase.SplitscreenControllersAllowed()
end

LUI.createMenu.Lobby = function(controller)
  local menu = createLobby(controller)

  local timer = LUI.UITimer.newElementTimer(100, false, function()
    -- Player 1's own pad never joins as player 2
    local primary = Engine.GetPrimaryController()
    local ignore = -1
    if Engine.GamepadsConnectedIsActive(primary) then
      ignore = Engine.GamepadsConnectedPort(primary) or -1
    end

    -- Always read the press, so one made while joining isn't possible
    -- doesn't fire later
    local port = game.getgamepadjoinpress(ignore)
    if port >= 0 and canJoin(menu) then
      LobbySplitscreenToggle(menu, primary)
    end
  end)
  menu:addElement(timer)

  return menu
end
