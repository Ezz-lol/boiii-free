if not Engine.IsInGame() then
  return
end

local function createPlayerOption(player)
  local clientNum = player.client_num
  local playerName = tostring(player.name or "Unknown")
  return {
    models = {
      displayText = playerName,
      action = function(self, element, controller)
        if game.kickplayer and game.kickplayer(clientNum) then
          GoBack(self, controller)
        end
      end,
    },
  }
end

DataSources.BoiiiKickPlayers = ListHelper_SetupDataSource("BoiiiKickPlayers", function()
  local options = {}
  local players = game.getkickableplayers and game.getkickableplayers() or {}
  for _, player in ipairs(players) do
    table.insert(options, createPlayerOption(player))
  end

  if #options == 0 then
    table.insert(options, {
      models = {
        displayText = "NO PLAYERS TO KICK",
        disabledFunction = function()
          return true
        end,
      },
    })
  end
  return options
end, true)

LUI.createMenu.BoiiiKickPlayersMenu = function(controller)
  local self = CoD.Menu.NewForUIEditor("BoiiiKickPlayersMenu")
  self.soundSet = "ChooseDecal"
  self:setOwner(controller)
  self:setLeftRight(true, true, 0, 0)
  self:setTopBottom(true, true, 0, 0)
  self:playSound("menu_open", controller)
  self.buttonModel = Engine.CreateModel(Engine.GetModelForController(controller), "BoiiiKickPlayersMenu.buttonPrompts")
  self.anyChildUsesUpdateState = true

  local background = LUI.UIImage.new()
  background:setLeftRight(true, true, 0, 0)
  background:setTopBottom(true, true, 0, 0)
  background:setRGB(0, 0, 0)
  background:setAlpha(0.85)
  self:addElement(background)

  local list = LUI.UIList.new(self, controller, 2, 0, nil, true, false, 0, 0, false, false)
  list:makeFocusable()
  list:setLeftRight(true, false, 26, 406)
  list:setTopBottom(true, false, 135, 369)
  list:setWidgetType(CoD.List1ButtonLarge_PH)
  list:setVerticalCount(7)
  list:setDataSource("BoiiiKickPlayers")
  self:addElement(list)
  self.Options = list

  if CoD.GenericMenuFrame then
    local frame = CoD.GenericMenuFrame.new(self, controller)
    frame:setLeftRight(true, true, 0, 0)
    frame:setTopBottom(true, true, 0, 0)
    frame.titleLabel:setText(Engine.Localize("KICK PLAYER"))
    pcall(function()
      frame.cac3dTitleIntermediary0.FE3dTitleContainer0.MenuTitle.TextBox1.Label0:setText(
        Engine.Localize("KICK PLAYER")
      )
    end)
    frame:setModel(self.buttonModel, controller)
    self:addElement(frame)
    self.MenuFrame = frame
  end

  self:AddButtonCallbackFunction(
    list,
    controller,
    Enum.LUIButton.LUI_KEY_XBA_PSCROSS,
    "ENTER",
    function(element, menu, actionController)
      ProcessListAction(self, element, actionController)
      return true
    end,
    function(element, menu)
      CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBA_PSCROSS, "MENU_SELECT")
      return true
    end,
    false
  )

  self:AddButtonCallbackFunction(
    self,
    controller,
    Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE,
    nil,
    function(element, menu, actionController)
      GoBack(self, actionController)
      return true
    end,
    function(element, menu)
      CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE, "MENU_BACK")
      return true
    end,
    false
  )

  list.id = "Options"
  self:processEvent({ name = "menu_loaded", controller = controller })
  self:processEvent({ name = "update_state", menu = self })
  if not self:restoreState() then
    list:processEvent({ name = "gain_focus", controller = controller })
  end

  LUI.OverrideFunction_CallOriginalSecond(self, "close", function(element)
    if element.MenuFrame then
      element.MenuFrame:close()
    end
    element.Options:close()
    Engine.UnsubscribeAndFreeModel(
      Engine.GetModel(Engine.GetModelForController(controller), "BoiiiKickPlayersMenu.buttonPrompts")
    )
  end)
  return self
end
