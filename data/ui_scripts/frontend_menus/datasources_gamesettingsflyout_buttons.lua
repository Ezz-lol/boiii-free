local SetupPCMouseHandling = function(menu, controller)
  if not CoD.useMouse then
    return
  else
    menu.Options:setHandleMouse(true)
    menu.Options:registerEventHandler("leftclick_outside", function(element, event)
      CoD.PCUtil.SimulateButtonPress(event.controller, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE)
      return true
    end)
  end
end

local PostLoadFunc = function(menu, controller)
  SetupPCMouseHandling(menu, controller)
  menu.disableBlur = true
  menu.disablePopupOpenCloseAnim = true
  Engine.SetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "GameSettingsFlyoutOpen"), true)
  LUI.OverrideFunction_CallOriginalSecond(menu, "close", function(element)
    Engine.SetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "GameSettingsFlyoutOpen"), false)
  end)
  menu:registerEventHandler("occlusion_change", function(element, event)
    local parentElement = element:getParent()
    if parentElement then
      local siblingElement = parentElement:getFirstChild()
      while siblingElement ~= nil do
        if siblingElement.menuName == "Lobby" then
          break
        end
        siblingElement = siblingElement:getNextSibling()
      end
      if siblingElement then
        if event.occluded == true then
          siblingElement:setAlpha(0)
        end
        siblingElement:setAlpha(1)
      end
    end
    element:OcclusionChange(event)
  end)
  menu:subscribeToModel(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.lobbyNav", true), function(model)
    local occludingMenu = menu.occludedBy
    while occludingMenu do
      if occludingMenu.occludedBy ~= nil then
        occludingMenu = occludingMenu.occludedBy
      end
      while occludingMenu and occludingMenu.menuName ~= "Lobby" do
        occludingMenu = GoBack(occludingMenu, controller)
      end
      Engine.SendClientScriptNotify(
        controller,
        "menu_change" .. Engine.GetLocalClientNum(controller),
        "Main",
        "closeToMenu"
      )
      return
    end
    GoBack(menu, controller)
  end, false)
end

DataSources.GameSettingsFlyoutButtonsCustom = DataSourceHelpers.ListSetup(
  "GameSettingsFlyoutButtonsCustom",
  function(controller)
    local buttonDefinitions = {
      {
        optionDisplay = "MPUI_CHANGE_MAP_CAPS",
        customId = "btnChangeMap",
        action = OpenChangeMap,
      },
      -- {
      -- optionDisplay = "MPUI_CHANGE_GAME_MODE_CAPS",
      -- customId = "btnChangeGameMode",
      -- action = OpenChangeGameMode
      -- },
      {
        optionDisplay = "MENU_SETUP_BOTS_CAPS",
        customId = "btnSetupBots",
        action = OpenBotSettings,
      },
      {
        optionDisplay = "MPUI_EDIT_GAME_RULES_CAPS",
        customId = "btnEditGameRules",
        action = OpenEditGameRules,
      },
    }
    -- if CoD.isPC and IsServerBrowserEnabled() then
    -- table.insert( buttonDefinitions, {
    -- optionDisplay = "PLATFORM_SERVER_SETTINGS_CAPS",
    -- customID = "btnServerSettings",
    -- action = OpenServerSettings
    -- } )
    -- end
    local customButtonsList = {}
    for index, buttonData in ipairs(buttonDefinitions) do
      table.insert(customButtonsList, {
        models = {
          displayText = Engine.Localize(buttonData.optionDisplay),
          customId = buttonData.customId,
          disabled = buttonData.disabled,
        },
        properties = {
          title = buttonData.optionDisplay,
          desc = buttonData.desc,
          action = buttonData.action,
          actionParam = buttonData.actionParam,
        },
      })
    end
    return customButtonsList
  end,
  nil,
  nil,
  nil
)

LUI.createMenu.GameSettingsFlyoutMPCustom = function(controller)
  local self = CoD.Menu.NewForUIEditor("GameSettingsFlyoutMPCustom")
  if PreLoadFunc then
    PreLoadFunc(self, controller)
  end
  self.soundSet = "default"
  self:setOwner(controller)
  self:setLeftRight(true, true, 0, 0)
  self:setTopBottom(true, true, 0, 0)
  self:playSound("menu_open", controller)
  self.buttonModel = Engine.CreateModel(Engine.GetModelForController(controller), "GameSettingsFlyoutMP.buttonPrompts")
  self.anyChildUsesUpdateState = true

  local Options = LUI.UIList.new(self, controller, -2, 0, nil, false, false, 0, 0, false, false)
  Options:makeFocusable()
  Options:setLeftRight(true, false, 243.43, 523.43)
  Options:setTopBottom(true, false, 177.56, 329.56)
  Options:setYRot(25)
  Options:setWidgetType(CoD.FE_List1ButtonLarge_PH)
  Options:setVerticalCount(3)
  Options:setSpacing(-2)
  Options:setDataSource("GameSettingsFlyoutButtonsCustom")
  Options:registerEventHandler("gain_focus", function(element, event)
    local returnValue = nil
    if element.gainFocus then
      returnValue = element:gainFocus(event)
    elseif element.super.gainFocus then
      returnValue = element.super:gainFocus(event)
    end
    CoD.Menu.UpdateButtonShownState(element, self, controller, Enum.LUIButton.LUI_KEY_XBA_PSCROSS)
    return returnValue
  end)
  Options:registerEventHandler("lose_focus", function(element, event)
    local returnValue = nil
    if element.loseFocus then
      returnValue = element:loseFocus(event)
    elseif element.super.loseFocus then
      returnValue = element.super:loseFocus(event)
    end
    return returnValue
  end)
  self:AddButtonCallbackFunction(
    Options,
    controller,
    Enum.LUIButton.LUI_KEY_XBA_PSCROSS,
    "ENTER",
    function(element, menu, controller, model)
      ProcessListAction(self, element, controller)
      return true
    end,
    function(element, menu, controller)
      CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBA_PSCROSS, "MENU_SELECT")
      return true
    end,
    false
  )
  self:addElement(Options)
  self.Options = Options

  self:mergeStateConditions({
    {
      stateName = "Local",
      condition = function(menu, element, event)
        return IsLobbyNetworkModeLAN()
      end,
    },
  })
  self:subscribeToModel(Engine.GetModel(Engine.GetGlobalModel(), "lobbyRoot.lobbyNetworkMode"), function(model)
    local menu = self
    local eventData = {
      controller = controller,
      name = "model_validation",
      modelValue = Engine.GetModelValue(model),
      modelName = "lobbyRoot.lobbyNetworkMode",
    }
    CoD.Menu.UpdateButtonShownState(menu, self, controller, Enum.LUIButton.LUI_KEY_XBY_PSTRIANGLE)
  end)
  self:subscribeToModel(Engine.GetModel(Engine.GetGlobalModel(), "lobbyRoot.lobbyNav"), function(model)
    local menu = self
    local eventData = {
      controller = controller,
      name = "model_validation",
      modelValue = Engine.GetModelValue(model),
      modelName = "lobbyRoot.lobbyNav",
    }
    CoD.Menu.UpdateButtonShownState(menu, self, controller, Enum.LUIButton.LUI_KEY_XBY_PSTRIANGLE)
  end)
  self:AddButtonCallbackFunction(
    self,
    controller,
    Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE,
    nil,
    function(element, menu, controller, model)
      GoBack(self, controller)
      ClearMenuSavedState(menu)
      return true
    end,
    function(element, menu, controller)
      CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE, "")
      return false
    end,
    false
  )
  self:AddButtonCallbackFunction(
    self,
    controller,
    Enum.LUIButton.LUI_KEY_START,
    "M",
    function(element, menu, controller, model)
      GoBackAndOpenOverlayOnParent(self, "StartMenu_Main", controller)
      return true
    end,
    function(element, menu, controller)
      CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_START, "MENU_MENU")
      return true
    end,
    false
  )
  self:AddButtonCallbackFunction(
    self,
    controller,
    Enum.LUIButton.LUI_KEY_XBY_PSTRIANGLE,
    "S",
    function(element, menu, controller, model)
      if not IsLAN() and not IsPlayerAGuest(controller) and IsPlayerAllowedToPlayOnline(controller) then
        GoBackAndOpenOverlayOnParent(self, "Social_Main", controller)
        return true
      else
      end
    end,
    function(element, menu, controller)
      if not IsLAN() and not IsPlayerAGuest(controller) and IsPlayerAllowedToPlayOnline(controller) then
        CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBY_PSTRIANGLE, "")
        return false
      else
        return false
      end
    end,
    false
  )
  self:AddButtonCallbackFunction(
    self,
    controller,
    Enum.LUIButton.LUI_KEY_LB,
    nil,
    function(element, menu, controller, model)
      SendButtonPressToOccludedMenu(menu, controller, model, Enum.LUIButton.LUI_KEY_LB)
      return true
    end,
    function(element, menu, controller)
      CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_LB, "")
      return false
    end,
    false
  )
  self:AddButtonCallbackFunction(
    self,
    controller,
    Enum.LUIButton.LUI_KEY_RB,
    nil,
    function(element, menu, controller, model)
      SendButtonPressToOccludedMenu(menu, controller, model, Enum.LUIButton.LUI_KEY_RB)
      return true
    end,
    function(element, menu, controller)
      CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_RB, "")
      return false
    end,
    false
  )
  Options.id = "Options"
  self:processEvent({
    name = "menu_loaded",
    controller = controller,
  })
  self:processEvent({
    name = "update_state",
    menu = self,
  })
  if not self:restoreState() then
    self.Options:processEvent({
      name = "gain_focus",
      controller = controller,
    })
  end
  LUI.OverrideFunction_CallOriginalSecond(self, "close", function(element)
    element.Options:close()
    Engine.UnsubscribeAndFreeModel(
      Engine.GetModel(Engine.GetModelForController(controller), "GameSettingsFlyoutMP.buttonPrompts")
    )
  end)
  if PostLoadFunc then
    PostLoadFunc(self, controller)
  end
  return self
end
