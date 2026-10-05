if not Engine.IsInGame() then
  return
end

local function option(text, description, action)
  return { models = { displayText = text, description = description, action = action } }
end

local modePrefixes = {
  [Enum.eModes.MODE_ZOMBIES] = "zm_",
  [Enum.eModes.MODE_MULTIPLAYER] = "mp_",
  [Enum.eModes.MODE_CAMPAIGN] = "cp_",
}

local function changeMap(mapName)
  return function(self, element, controller)
    Engine.Exec(controller, "map " .. mapName)
  end
end

DataSources.BoiiiChangeMap = ListHelper_SetupDataSource("BoiiiChangeMap", function()
  local options = {}
  local mode = Engine.CurrentSessionMode()
  local prefix = modePrefixes[mode] or ""
  local current = Engine.GetCurrentMap()
  local maps = Engine.GetGDTMapsTable()

  local function isListed(mapName)
    local map = maps[mapName]
    return map.session_mode == mode
      and LUI.startswith(mapName, prefix)
      and not LUI.startswith(mapName, "mp_freerun")
      and Engine.IsMapValid(mapName)
  end

  local function byOrder(a, b)
    return maps[a].unique_id < maps[b].unique_id
  end

  for mapName, map in LUI.IterateTableBySortedKeys(maps, byOrder, isListed) do
    local name = Engine.Localize(map.mapName)
    table.insert(
      options,
      option(mapName == current and "^3" .. name or name, Engine.Localize(map.mapDescription), changeMap(mapName))
    )
  end

  local count = Engine.Mods_Lists_GetInfoEntriesCount(LuaEnums.USERMAP_BASE_PATH)
  local usermaps = count > 0 and Engine.Mods_Lists_GetInfoEntries(LuaEnums.USERMAP_BASE_PATH, 0, count) or {}
  for index = 0, #usermaps do
    local usermap = usermaps[index]
    if usermap and LUI.startswith(usermap.internalName, prefix) then
      local name = usermap.internalName == current and "^3" .. usermap.name or usermap.name
      table.insert(options, option(name, usermap.description, changeMap(usermap.internalName)))
    end
  end
  return options
end, true)

DataSources.BoiiiKickPlayers = ListHelper_SetupDataSource("BoiiiKickPlayers", function()
  local options = {}
  local session = Engine.LobbyGetSessionClients(Enum.LobbyModule.LOBBY_MODULE_HOST, Enum.LobbyType.LOBBY_TYPE_GAME)
  for _, client in ipairs(session and session.sessionClients or {}) do
    if not client.isHost and not client.isLocal then
      table.insert(
        options,
        option(client.gamertag, "Remove this player from the match.", function(self, element, controller, param, menu)
          Engine.KickClient(
            controller,
            Enum.LobbyType.LOBBY_TYPE_GAME,
            client.xuid,
            Enum.LobbyDisconnectClient.LOBBY_DISCONNECT_CLIENT_KICK,
            "EXE_PLAYERKICKED"
          )
          GoBack(menu, controller)
        end)
      )
    end
  end

  if #options == 0 then
    table.insert(options, option("NO PLAYERS TO KICK", "Only you are in this match.", nil))
  end
  return options
end, true)

local function createListMenu(name, title, dataSource)
  LUI.createMenu[name] = function(controller)
    local self = CoD.Menu.NewForUIEditor(name)
    self.soundSet = "ChooseDecal"
    self:setOwner(controller)
    self:setLeftRight(true, true, 0, 0)
    self:setTopBottom(true, true, 0, 0)
    self:playSound("menu_open", controller)
    self.buttonModel = Engine.CreateModel(Engine.GetModelForController(controller), name .. ".buttonPrompts")
    self.anyChildUsesUpdateState = true

    local background = LUI.UIImage.new()
    background:setLeftRight(true, true, 0, 0)
    background:setTopBottom(true, true, 0, 0)
    background:setRGB(0, 0, 0)
    background:setAlpha(0.7)
    self:addElement(background)

    local list = LUI.UIList.new(self, controller, 2, 0, nil, false, false, 0, 0, false, false)
    list:makeFocusable()
    list:setLeftRight(true, false, 64, 444)
    list:setTopBottom(true, false, 136, 640)
    list:setWidgetType(CoD.List1ButtonLarge_PH)
    list:setVerticalCount(14)
    list:setDataSource(dataSource)
    self:addElement(list)
    self.Options = list

    local heading = LUI.UIText.new()
    heading:setLeftRight(true, false, 500, 1180)
    heading:setTopBottom(true, false, 136, 172)
    heading:setTTF("fonts/escom.ttf")
    heading:linkToElementModel(list, "displayText", true, function(model)
      heading:setText(Engine.GetModelValue(model) or "")
    end)
    self:addElement(heading)

    local description = LUI.UIText.new()
    description:setLeftRight(true, false, 500, 1180)
    description:setTopBottom(true, false, 188, 210)
    description:setTTF("fonts/default.ttf")
    description:setRGB(0.75, 0.75, 0.75)
    description:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
    description:linkToElementModel(list, "description", true, function(model)
      description:setText(Engine.GetModelValue(model) or "")
    end)
    self:addElement(description)

    local frame = CoD.GenericMenuFrame.new(self, controller)
    frame:setLeftRight(true, true, 0, 0)
    frame:setTopBottom(true, true, 0, 0)
    frame.titleLabel:setText(title)
    pcall(function()
      frame.cac3dTitleIntermediary0.FE3dTitleContainer0.MenuTitle.TextBox1.Label0:setText(title)
    end)
    frame:setModel(self.buttonModel, controller)
    self:addElement(frame)
    self.MenuFrame = frame

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
      element.MenuFrame:close()
      element.Options:close()
      Engine.UnsubscribeAndFreeModel(
        Engine.GetModel(Engine.GetModelForController(controller), name .. ".buttonPrompts")
      )
    end)
    return self
  end
end

createListMenu("BoiiiChangeMapMenu", "CHANGE MAP", "BoiiiChangeMap")
createListMenu("BoiiiKickPlayersMenu", "KICK PLAYER", "BoiiiKickPlayers")
