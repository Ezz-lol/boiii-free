-- Lets a second player join split-screen by pressing A / Cross on their
-- controller. The stock menus only handle this press on console.

if Engine.GetCurrentMap() ~= "core_frontend" or not CoD.isPC then
  return
end

if not LUI.createMenu.Lobby then
  pcall(require, "ui.uieditor.menus.Lobby.Lobby")
end

local createLobby = LUI.createMenu.Lobby
if createLobby then
  LUI.createMenu.Lobby = function(controller)
    local menu = createLobby(controller)
    local pressed = {}
    -- Menus read buttons from each controller's ButtonBits models rather than
    -- gamepad_button events, and only for the controller that owns them
    for joining = 1, Engine.GetMaxLocalControllers() - 1 do
      local model =
        Engine.CreateModel(Engine.GetModelForController(joining), "ButtonBits." .. Enum.LUIButton.LUI_KEY_XBA_PSCROSS)
      menu:subscribeToModel(model, function(buttonModel)
        local value = Engine.GetModelValue(buttonModel) or 0
        if CoD.BitUtility.IsBitwiseAndNonZero(value, Enum.LUIButtonFlags.FLAG_DOWN) then
          pressed[joining] = not Engine.IsControllerBeingUsed(joining)
          return
        end
        -- Joining on release, since joining on press makes the player active
        -- while the button is still down and the lobby then handles it as a select
        local wasPressed = pressed[joining]
        pressed[joining] = false
        if
          wasPressed
          and not menu.m_inputDisabled
          and not menu.occludedBy
          and not Engine.IsControllerBeingUsed(joining)
          and LuaUtils.LobbyProcessQueueEmpty()
        then
          LobbySplitscreenToggle(menu, joining)
        end
      end, false)
    end
    return menu
  end
end

local removeLocalClient = LobbyRemoveLocalClientFromLobby
if removeLocalClient then
  LobbyRemoveLocalClientFromLobby = function(menu, controller)
    local primary = Engine.GetPrimaryController()
    if IsGamepad(primary) or controller == primary or not Engine.IsControllerBeingUsed(controller) then
      return removeLocalClient(menu, controller)
    end
    -- The stock version maps any gamepad to player 1 when player 1 has none,
    -- which takes player 2's controller from a mouse and keyboard host
    CoD.LobbyBase.RemoveLocalPlayerFromTheLobby(menu, controller)
    CoD.perController[primary].localClientChanged = true
    ForceLobbyButtonUpdate(controller)
  end
end

if not CoD.FE_ListAdditonal then
  pcall(require, "ui.uieditor.widgets.Lobby.Common.FE_ListAdditonal")
end

if not CoD.FE_ListAdditonal then
  return
end

local createListAdditional = CoD.FE_ListAdditonal.new
CoD.FE_ListAdditonal.new = function(menu, controller)
  local self = createListAdditional(menu, controller)
  for _, state in ipairs(self.stateConditions or {}) do
    if state.stateName == "AddControllerState_Play" then
      state.condition = function()
        return IsWaitingForSignInToPlaySplitscreen() and ShouldShowAddController(menu, self, controller)
      end
    end
  end
  return self
end
