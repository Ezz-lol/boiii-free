if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

function IsServerBrowserEnabled()
  return true
end

function IsSteamServerBrowserUpdating()
  return game.isserverlistrefreshing()
end

local skullSortedOrder = nil
local skullSortAscending = nil
local activeServerList = nil

local CUSTOM_TYPE_ALL = 100
local CUSTOM_TYPE_CAMPAIGN = 101
local CUSTOM_TYPE_ZOMBIES = 102

local currentCustomMode = nil
local filteredServerIndices = nil
local currentRequestedServerType = nil

local addressToRawIndex = {}
local isRebuildingCustom = false
local customActiveFilters = {}
local customActiveAttributes = {}

local function isCustomTab()
  return currentCustomMode == "all" or currentCustomMode == "cp" or currentCustomMode == "zm"
end

local function isValidServer(info)
  if not info or not info.name or info.name == "" then
    return false
  end
  if not info.gametype or info.gametype == "" then
    return false
  end
  return true
end

local currentSortType = nil

local sortSpecs = {}
for name, field in pairs({
  NAME = "name",
  PING = "ping",
  PLAYERS = "playerCount",
  MAP = "map",
  GAMETYPE = "gametype",
  DEDICATED = "dedicated",
  RANKED = "ranked",
  PROTECTED = "password",
  HARDCORE = "hardcore",
}) do
  local prefix = "STEAM_SERVER_SORT_TYPE_" .. name
  sortSpecs[Enum.SteamServerSortType[prefix .. "_ASCENDING"]] = { field = field, ascending = true }
  sortSpecs[Enum.SteamServerSortType[prefix .. "_DESCENDING"]] = { field = field, ascending = false }
end

local function sortKey(value)
  if type(value) == "boolean" then
    return value and 1 or 0
  elseif type(value) == "string" then
    return string.lower(value)
  end
  return value
end

local function sortFilteredIndices(sortType)
  local spec = sortSpecs[sortType]
  if not spec or not filteredServerIndices then
    return
  end

  local keys = {}
  for _, idx in ipairs(filteredServerIndices) do
    local info = game.getrawserverinfo(idx)
    keys[idx] = info and sortKey(info[spec.field])
  end

  table.sort(filteredServerIndices, function(a, b)
    local va, vb = keys[a], keys[b]
    if va == nil or vb == nil or type(va) ~= type(vb) then
      return va ~= nil and vb == nil
    end
    if va == vb then
      return a < b
    end
    if spec.ascending then
      return va < vb
    end
    return va > vb
  end)
end

local function rebuildAddressMap()
  addressToRawIndex = {}
  local rawCount = game.getrawservercount()
  for i = 0, rawCount - 1 do
    local info = game.getrawserverinfo(i)
    if info and info.connectAddr and info.connectAddr ~= "" then
      addressToRawIndex[info.connectAddr] = i
    end
  end
end

local function passesCustomFilters(info)
  -- Attribute filters
  local attr = Enum.SteamServerAttributeFilter
  if attr then
    if customActiveAttributes[attr.STEAM_SERVER_BROWSER_ATTRIBUTEFILTER_DEDICATED] and not info.dedicated then
      return false
    end
    if customActiveAttributes[attr.STEAM_SERVER_BROWSER_ATTRIBUTEFILTER_NOTDEDICATED] and info.dedicated then
      return false
    end
    if customActiveAttributes[attr.STEAM_SERVER_BROWSER_ATTRIBUTEFILTER_RANKED] and not info.ranked then
      return false
    end
    if customActiveAttributes[attr.STEAM_SERVER_BROWSER_ATTRIBUTEFILTER_UNRANKED] and info.ranked then
      return false
    end
    if customActiveAttributes[attr.STEAM_SERVER_BROWSER_ATTRIBUTEFILTER_HARDCORE] and not info.hardcore then
      return false
    end
    if customActiveAttributes[attr.STEAM_SERVER_BROWSER_ATTRIBUTEFILTER_CORE] and info.hardcore then
      return false
    end
  end

  local ft = Enum.SteamServerFilterType
  if ft then
    -- Map filter
    local mapSet = customActiveFilters[ft.STEAM_SERVER_BROWSER_FILTERTYPE_MAP]
    if mapSet and next(mapSet) and not mapSet[info.map] then
      return false
    end

    -- Gametype filter
    local gtSet = customActiveFilters[ft.STEAM_SERVER_BROWSER_FILTERTYPE_GAMETYPE]
    if gtSet and next(gtSet) and not gtSet[info.gametype] then
      return false
    end

    -- Mod filter
    local modSet = customActiveFilters[ft.STEAM_SERVER_BROWSER_FILTERTYPE_MOD]
    if modSet and next(modSet) and not modSet[info.modName or ""] then
      return false
    end

    -- Keyword filter (match against name/description/mod)
    local kwSet = customActiveFilters[ft.STEAM_SERVER_BROWSER_FILTERTYPE_KEYWORDS]
    if kwSet and next(kwSet) then
      local nameLower = string.lower(info.name or "")
      local descLower = string.lower(info.desc or "")
      local modLower = string.lower(info.modName or "")
      local found = false
      for kw, _ in pairs(kwSet) do
        if
          string.find(nameLower, kw, 1, true)
          or string.find(descLower, kw, 1, true)
          or string.find(modLower, kw, 1, true)
        then
          found = true
          break
        end
      end
      if not found then
        return false
      end
    end
  end

  return true
end

local function rebuildFilteredIndices()
  local rawCount = game.getrawservercount()
  filteredServerIndices = {}
  addressToRawIndex = {}
  for i = 0, rawCount - 1 do
    local info = game.getrawserverinfo(i)
    if info and info.connectAddr and info.connectAddr ~= "" then
      addressToRawIndex[info.connectAddr] = i
    end
    if isValidServer(info) then
      local modeOk = false
      if currentCustomMode == "all" then
        modeOk = true
      elseif currentCustomMode == "zm" then
        modeOk = (info.zombies == true)
      elseif currentCustomMode == "cp" then
        modeOk = (info.campaign and info.campaign == 1)
      end

      if modeOk and passesCustomFilters(info) then
        table.insert(filteredServerIndices, i)
      end
    end
  end

  if currentSortType then
    sortFilteredIndices(currentSortType)
  end

  return #filteredServerIndices
end

-- Capture real engine originals only once (survives lua_reload)
if not __sb_originals then
  __sb_originals = {
    RequestServers = Engine.SteamServerBrowser_RequestServers,
    Sort = Engine.SteamServerBrowser_Sort,
    HeaderNew = CoD.ServerBrowserHeader and CoD.ServerBrowserHeader.new or nil,
    CreateLobbyBrowser = LUI.createMenu and LUI.createMenu.LobbyServerBrowserOnline or nil,
    AddFilter = Engine.SteamServerBrowser_AddFilter,
    RemoveFilter = Engine.SteamServerBrowser_RemoveFilter,
    ClearFilter = Engine.SteamServerBrowser_ClearFilter,
    SetAttributeFilter = Engine.SteamServerBrowser_SetAttributeFilter,
    ClearAttributeFilters = Engine.SteamServerBrowser_ClearAttributeFilters,
    GetCurrentServerRequestType = Engine.SteamServerBrowser_GetCurrentServerRequestType,
  }
end
local SB = __sb_originals

if SB.RequestServers then
  Engine.SteamServerBrowser_RequestServers = function(serverType)
    filteredServerIndices = nil
    skullSortedOrder = nil
    skullSortAscending = nil
    currentSortType = nil
    currentRequestedServerType = serverType

    local customModes = { [CUSTOM_TYPE_ALL] = "all", [CUSTOM_TYPE_CAMPAIGN] = "cp", [CUSTOM_TYPE_ZOMBIES] = "zm" }
    currentCustomMode = customModes[serverType]
    pcall(
      SB.RequestServers,
      currentCustomMode and Enum.SteamServerRequestType.STEAM_SERVER_REQUEST_TYPE_INTERNET or serverType
    )
  end
end

-- Override GetCurrentServerRequestType so RefreshLobbyServerBrowser preserves custom tab state
if SB.GetCurrentServerRequestType then
  Engine.SteamServerBrowser_GetCurrentServerRequestType = function()
    if currentRequestedServerType then
      return currentRequestedServerType
    end
    local ok, result = pcall(SB.GetCurrentServerRequestType)
    if ok then
      return result
    end
    return Enum.SteamServerRequestType.STEAM_SERVER_REQUEST_TYPE_INTERNET
  end
end

if SB.Sort then
  Engine.SteamServerBrowser_Sort = function(sortType)
    pcall(SB.Sort, sortType)
    if isCustomTab() and filteredServerIndices then
      currentSortType = sortType
      skullSortedOrder = nil
      skullSortAscending = nil
      sortFilteredIndices(sortType)
      if activeServerList then
        pcall(function()
          activeServerList:updateDataSource(false, false)
        end)
      end
    end
  end
end

-- Hook filter APIs to track state and trigger rebuilds on custom tabs
local function refreshCustomList(list)
  if isRebuildingCustom or not list.serverBrowserRootModel then
    return
  end
  isRebuildingCustom = true
  pcall(function()
    local count = rebuildFilteredIndices()
    list.serverCount = count
    for _, name in ipairs({ "serverListCount", "serverListUpdatedCount" }) do
      local model = Engine.GetModel(list.serverBrowserRootModel, name)
      if model then
        Engine.SetModelValue(model, count)
      end
    end
    list:updateDataSource(false, false)
  end)
  isRebuildingCustom = false
end

local function triggerCustomRebuild()
  if isCustomTab() and activeServerList then
    refreshCustomList(activeServerList)
  end
end

if SB.AddFilter then
  Engine.SteamServerBrowser_AddFilter = function(filterType, id)
    customActiveFilters[filterType] = customActiveFilters[filterType] or {}
    customActiveFilters[filterType][id] = true
    pcall(SB.AddFilter, filterType, id)
    triggerCustomRebuild()
  end
end

if SB.RemoveFilter then
  Engine.SteamServerBrowser_RemoveFilter = function(filterType, id)
    if customActiveFilters[filterType] then
      customActiveFilters[filterType][id] = nil
    end
    pcall(SB.RemoveFilter, filterType, id)
    triggerCustomRebuild()
  end
end

if SB.ClearFilter then
  Engine.SteamServerBrowser_ClearFilter = function(filterType)
    customActiveFilters[filterType] = nil
    pcall(SB.ClearFilter, filterType)
    triggerCustomRebuild()
  end
end

if SB.SetAttributeFilter then
  Engine.SteamServerBrowser_SetAttributeFilter = function(attr, active)
    customActiveAttributes[attr] = active or nil
    pcall(SB.SetAttributeFilter, attr, active)
    triggerCustomRebuild()
  end
end

if SB.ClearAttributeFilters then
  Engine.SteamServerBrowser_ClearAttributeFilters = function()
    customActiveAttributes = {}
    pcall(SB.ClearAttributeFilters)
    triggerCustomRebuild()
  end
end

DataSources.ServerBrowserCategories = ListHelper_SetupDataSource("ServerBrowserCategories", function(controller)
  local tabs = {}

  table.insert(tabs, {
    models = { tabIcon = CoD.buttonStrings.shoulderl },
    properties = { m_mouseDisabled = true },
  })

  table.insert(tabs, {
    models = {
      tabName = "ALL SERVERS",
      serverType = CUSTOM_TYPE_ALL,
    },
  })

  table.insert(tabs, {
    models = {
      tabName = "MENU_MULTIPLAYER_CAPS",
      serverType = Enum.SteamServerRequestType.STEAM_SERVER_REQUEST_TYPE_INTERNET,
    },
  })

  table.insert(tabs, {
    models = {
      tabName = "MENU_ZOMBIES_CAPS",
      serverType = CUSTOM_TYPE_ZOMBIES,
    },
  })

  table.insert(tabs, {
    models = {
      tabName = "CAMPAIGN",
      serverType = CUSTOM_TYPE_CAMPAIGN,
    },
  })

  table.insert(tabs, {
    models = {
      tabName = "MENU_LAN",
      serverType = Enum.SteamServerRequestType.STEAM_SERVER_REQUEST_TYPE_LAN,
    },
  })

  table.insert(tabs, {
    models = {
      tabName = "MENU_RECENT_CAPS",
      serverType = Enum.SteamServerRequestType.STEAM_SERVER_REQUEST_TYPE_HISTORY,
    },
  })

  table.insert(tabs, {
    models = {
      tabName = "MENU_FRIENDS_CAPS",
      serverType = Enum.SteamServerRequestType.STEAM_SERVER_REQUEST_TYPE_FRIENDS,
    },
  })

  table.insert(tabs, {
    models = {
      tabName = "MENU_FAVORITES_CAPS",
      serverType = Enum.SteamServerRequestType.STEAM_SERVER_REQUEST_TYPE_FAVORITES,
    },
  })

  table.insert(tabs, {
    models = { tabIcon = CoD.buttonStrings.shoulderr },
    properties = { m_mouseDisabled = true },
  })

  return tabs
end, true)

DataSources.LobbyServer = {
  prepare = function(controller, list, filter)
    list.numElementsInList = list.vCount
    list.controller = controller
    list.serverBrowserRootModel = Engine.CreateModel(Engine.GetGlobalModel(), "serverBrowser")
    local serverListCountModel = Engine.GetModel(list.serverBrowserRootModel, "serverListCount")
    if serverListCountModel then
      list.serverCount = Engine.GetModelValue(serverListCountModel) or 0
    else
      list.serverCount = 0
    end
    activeServerList = list
    list.servers = {}
    local serversModel = Engine.CreateModel(list.serverBrowserRootModel, "servers")
    for i = 1, list.numElementsInList, 1 do
      list.servers[i] = {}
      list.servers[i].root = Engine.CreateModel(serversModel, "server_" .. i)
      list.servers[i].model = Engine.CreateModel(list.servers[i].root, "model")
    end
    list.updateModels = function(controller, list, offset, displayOffset)
      local ok, serverInfo = pcall(function()
        if isCustomTab() then
          return game.getrawserverinfo(offset)
        else
          local info = Engine.SteamServerBrowser_GetServerInfo(offset)
          if info and (not info.modName or info.modName == "") then
            local addr = info.connectAddr
            if addr and addressToRawIndex[addr] then
              local rawInfo = game.getrawserverinfo(addressToRawIndex[addr])
              if rawInfo and rawInfo.modName and rawInfo.modName ~= "" then
                info.modName = rawInfo.modName
              end
            end
          end
          return info
        end
      end)
      if not ok then
        serverInfo = nil
      end
      if serverInfo and serverInfo.name then
        local SetModelValue = function(model, key, value)
          local model = Engine.CreateModel(model, key)
          if model then
            Engine.SetModelValue(model, value)
          end
        end

        local elementIndex = (displayOffset or offset) % list.numElementsInList + 1
        local serverModel = list.servers[elementIndex].model
        SetModelValue(serverModel, "serverIndex", serverInfo.serverIndex or offset)
        SetModelValue(serverModel, "connectAddr", serverInfo.connectAddr or "")
        SetModelValue(serverModel, "ping", serverInfo.ping or 0)
        SetModelValue(serverModel, "modName", serverInfo.modName or "")
        SetModelValue(serverModel, "mapName", serverInfo.map or "")
        SetModelValue(serverModel, "desc", serverInfo.desc or "")
        local clientCount = (serverInfo.playerCount or 0) - (serverInfo.botCount or 0)
        SetModelValue(serverModel, "clientCount", clientCount)
        SetModelValue(serverModel, "maxClients", serverInfo.maxPlayers or 0)
        SetModelValue(serverModel, "passwordProtected", serverInfo.password or false)
        SetModelValue(serverModel, "secure", serverInfo.secure or false)
        SetModelValue(serverModel, "name", serverInfo.name or "")
        SetModelValue(serverModel, "gameType", serverInfo.gametype or "")
        SetModelValue(serverModel, "dedicated", serverInfo.dedicated or false)
        SetModelValue(serverModel, "ranked", serverInfo.ranked or false)
        SetModelValue(serverModel, "hardcore", serverInfo.hardcore or false)
        SetModelValue(serverModel, "zombies", serverInfo.zombies or false)
        SetModelValue(serverModel, "campaign", serverInfo.campaign or 0)
        SetModelValue(serverModel, "botCount", serverInfo.botCount or 0)
        SetModelValue(serverModel, "rounds", serverInfo.rounds or 0)

        return serverModel
      else
        return nil
      end
    end

    if list.serverListUpdateSubscription then
      list:removeSubscription(list.serverListUpdateSubscription)
    end
    local serverListUpdateModel = Engine.CreateModel(list.serverBrowserRootModel, "serverListCount")
    list.serverListUpdateSubscription = list:subscribeToModel(serverListUpdateModel, function(model)
      pcall(function()
        if not list or not list.serverBrowserRootModel then
          return
        end
        skullSortedOrder = nil
        skullSortAscending = nil

        if isCustomTab() then
          refreshCustomList(list)
        else
          rebuildAddressMap()
          list.serverCount = (model and Engine.GetModelValue(model)) or 0
          list:updateDataSource(false, false)
        end
      end)
    end, false)
    if list.serverUpdatedCountSubscription then
      list:removeSubscription(list.serverUpdatedCountSubscription)
    end
    local serverUpdatedCountModel = Engine.CreateModel(list.serverBrowserRootModel, "serverListUpdatedCount")
    list.serverUpdatedCountSubscription = list:subscribeToModel(serverUpdatedCountModel, function(model)
      pcall(function()
        if not isCustomTab() then
          return
        end
        if isRebuildingCustom then
          return
        end
        if not list or not list.serverBrowserRootModel then
          return
        end
        local count = list.serverCount or 0
        if model and Engine.GetModelValue(model) ~= count then
          Engine.SetModelValue(model, count)
        end
      end)
    end, false)
    if list.serverListSortTypeSubscription then
      list:removeSubscription(list.serverListSortTypeSubscription)
    end
    local serverListSortTypeModel = Engine.CreateModel(list.serverBrowserRootModel, "serverListSortType")
    list.serverListSortTypeSubscription = list:subscribeToModel(serverListSortTypeModel, function(model)
      pcall(function()
        if not list or not list.serverBrowserRootModel then
          return
        end
        skullSortedOrder = nil
        skullSortAscending = nil
        list:updateDataSource(false, false)
      end)
    end, false)
  end,
  getCount = function(list)
    return list.serverCount
  end,
  getItem = function(controller, list, index)
    local ok, result = pcall(function()
      local displayOffset = index - 1
      local engineOffset = displayOffset
      if skullSortedOrder then
        engineOffset = skullSortedOrder[index]
        if engineOffset == nil then
          return nil
        end
      elseif filteredServerIndices then
        engineOffset = filteredServerIndices[index]
        if engineOffset == nil then
          return nil
        end
      end
      return list.updateModels(controller, list, engineOffset, displayOffset)
    end)
    if not ok then
      return nil
    end
    return result
  end,
  cleanup = function(list)
    activeServerList = nil
    if list.serverListUpdateSubscription then
      list:removeSubscription(list.serverListUpdateSubscription)
      list.serverListUpdateSubscription = nil
    end
    if list.serverUpdatedCountSubscription then
      list:removeSubscription(list.serverUpdatedCountSubscription)
      list.serverUpdatedCountSubscription = nil
    end
    if list.serverListSortTypeSubscription then
      list:removeSubscription(list.serverListSortTypeSubscription)
      list.serverListSortTypeSubscription = nil
    end
    list.serverBrowserRootModel = nil
  end,
}

local function addFlag(self, menu, controller, left, right, image, field)
  local flag = CoD.ServerBrowserFlag.new(menu, controller)
  flag:setLeftRight(true, false, left, right)
  flag:setTopBottom(true, true, 0, 0)
  flag.icon:setImage(RegisterImage(image))
  flag:linkToElementModel(self, nil, false, function(model)
    flag:setModel(model, controller)
  end)
  flag:mergeStateConditions({
    {
      stateName = "FlagOn",
      condition = function(menu, element, event)
        return IsSelfModelValueTrue(element, controller, field)
      end,
    },
  })
  flag:linkToElementModel(flag, field, true, function(model)
    menu:updateElementState(flag, {
      name = "model_validation",
      menu = menu,
      modelValue = Engine.GetModelValue(model),
      modelName = field,
    })
  end)
  self:addElement(flag)
  return flag
end

CoD.ServerBrowserRowInternal.new = function(menu, controller)
  local self = LUI.UIHorizontalList.new({
    left = 0,
    top = 0,
    right = 0,
    bottom = 0,
    leftAnchor = true,
    topAnchor = true,
    rightAnchor = true,
    bottomAnchor = true,
    spacing = 2,
  })
  self:setAlignment(LUI.Alignment.Left)
  if PreLoadFunc then
    PreLoadFunc(self, controller)
  end
  self:setUseStencil(false)
  self:setClass(CoD.ServerBrowserRowInternal)
  self.id = "ServerBrowserRowInternal"
  self.soundSet = "default"
  self:setLeftRight(true, false, 0, 700)
  self:setTopBottom(true, false, 0, 22)
  self:makeFocusable()
  self.onlyChildrenFocusable = true
  self.anyChildUsesUpdateState = true

  self.passwordFlag = addFlag(self, menu, controller, 0, 28, "uie_t7_icon_serverbrowser_protected", "passwordProtected")

  self.dedicatedFlag = addFlag(self, menu, controller, 30, 58, "uie_t7_icon_serverbrowser_dedicated", "dedicated")

  self.rankedFlag = addFlag(self, menu, controller, 60, 88, "uie_t7_icon_serverbrowser_ranked", "ranked")

  local name = CoD.horizontalScrollingTextBox_18pt.new(menu, controller)
  name:setLeftRight(true, false, 90, 330)
  name:setTopBottom(true, false, 2, 20)
  name.textBox:setTTF("fonts/default.ttf")
  name.textBox:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  name:linkToElementModel(self, "name", true, function(model)
    local _name = Engine.GetModelValue(model)
    if _name then
      name.textBox:setText(Engine.Localize(_name))
    end
  end)
  self:addElement(name)
  self.name = name

  local spacer = LUI.UIFrame.new(menu, controller, 0, 0, false)
  spacer:setLeftRight(true, false, 332, 339)
  spacer:setTopBottom(true, false, 0, 22)
  spacer:setAlpha(0)
  self:addElement(spacer)
  self.spacer = spacer

  local map = CoD.horizontalScrollingTextBox_18pt.new(menu, controller)
  map:setLeftRight(true, false, 341, 446)
  map:setTopBottom(true, false, 2, 20)
  map.textBox:setTTF("fonts/default.ttf")
  map.textBox:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  map:linkToElementModel(self, "mapName", true, function(model)
    local mapName = Engine.GetModelValue(model)
    if mapName then
      map.textBox:setText(MapNameToLocalizedMapName(mapName))
    end
  end)
  self:addElement(map)
  self.map = map

  local hardcoreFlag = addFlag(self, menu, controller, 448, 470, "uie_t7_icon_serverbrowser_skull", "hardcore")
  self.hardcoreFlag = hardcoreFlag

  local roundText = LUI.UIText.new()
  roundText:setLeftRight(true, true, 0, 0)
  roundText:setTopBottom(true, false, 2, 20)
  roundText:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
  roundText:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)
  roundText:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
  roundText:setAlpha(0)
  roundText:linkToElementModel(self, "rounds", true, function(model)
    local rounds = Engine.GetModelValue(model)
    if rounds and rounds > 0 then
      roundText:setText(tostring(rounds))
    else
      roundText:setText("0")
    end
  end)
  roundText:linkToElementModel(self, "zombies", true, function(model)
    local zombies = Engine.GetModelValue(model)
    if zombies ~= nil then
      roundText:setAlpha(zombies and 1 or 0)
      hardcoreFlag.icon:setAlpha(zombies and 0 or 1)
    end
  end)
  hardcoreFlag:addElement(roundText)
  self.roundText = roundText

  local gametype = LUI.UIText.new()
  gametype:setLeftRight(true, false, 472, 576)
  gametype:setTopBottom(true, false, 2, 20)
  gametype:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
  gametype:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  gametype:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
  gametype:linkToElementModel(self, "gameType", true, function(model)
    local gameType = Engine.GetModelValue(model)
    if gameType then
      gametype:setText(Engine.Localize(GetGameTypeDisplayString(gameType)))
    end
  end)
  gametype:setRGB(0.78, 0.78, 0.78)
  self:addElement(gametype)
  self.gametype = gametype

  self._clientCount = 0
  self._maxClients = 0
  local playerCount

  local function updatePlayerCountColor()
    if self._maxClients > 0 then
      local ratio = self._clientCount / self._maxClients
      if ratio < 0.5 then
        playerCount:setRGB(0.2, 1.0, 0.2)
      elseif ratio < 0.8 then
        playerCount:setRGB(1.0, 0.85, 0.1)
      elseif ratio < 0.95 then
        playerCount:setRGB(1.0, 0.55, 0.1)
      else
        playerCount:setRGB(1.0, 0.2, 0.2)
      end
    else
      playerCount:setRGB(1.0, 1.0, 1.0)
    end
  end

  playerCount = LUI.UIText.new()
  playerCount:setLeftRight(true, false, 593, 613)
  playerCount:setTopBottom(true, false, 2, 20)
  playerCount:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
  playerCount:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_RIGHT)
  playerCount:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
  playerCount:linkToElementModel(self, "clientCount", true, function(model)
    local clientCount = Engine.GetModelValue(model)
    if clientCount then
      playerCount:setText(Engine.Localize(clientCount))
      self._clientCount = tonumber(clientCount) or 0
      updatePlayerCountColor()
    end
  end)
  self:addElement(playerCount)
  self.playerCount = playerCount

  local slash = LUI.UIText.new()
  slash:setLeftRight(true, false, 615, 624)
  slash:setTopBottom(true, false, 2, 20)
  slash:setText(Engine.Localize("/"))
  slash:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
  slash:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  slash:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
  slash:setRGB(0.6, 0.6, 0.6)
  self:addElement(slash)
  self.slash = slash

  local maxPlayers = LUI.UIText.new()
  maxPlayers:setLeftRight(true, false, 626, 645)
  maxPlayers:setTopBottom(true, false, 2, 20)
  maxPlayers:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
  maxPlayers:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  maxPlayers:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
  maxPlayers:setRGB(0.7, 0.7, 0.7)
  maxPlayers:linkToElementModel(self, "maxClients", true, function(model)
    local maxClients = Engine.GetModelValue(model)
    if maxClients then
      maxPlayers:setText(Engine.Localize(maxClients))
      self._maxClients = tonumber(maxClients) or 0
      updatePlayerCountColor()
    end
  end)
  self:addElement(maxPlayers)
  self.maxPlayers = maxPlayers

  local botCount = LUI.UIText.new()
  botCount:setLeftRight(true, false, 637, 659)
  botCount:setTopBottom(true, false, 2, 20)
  botCount:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
  botCount:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  botCount:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
  botCount:linkToElementModel(self, "botCount", true, function(model)
    local _botCount = Engine.GetModelValue(model)
    if _botCount then
      botCount:setText("[" .. Engine.Localize(_botCount) .. "]")
    end
  end)
  botCount:setRGB(0.6, 0.6, 0.6)
  botCount:linkToElementModel(self, "zombies", true, function(model)
    local zombies = Engine.GetModelValue(model)
    if zombies ~= nil then
      botCount:setAlpha(zombies and 0 or 1)
    end
  end)
  self:addElement(botCount)
  self.botCount = botCount

  local ping = LUI.UIText.new()
  ping:setLeftRight(true, false, 661, 699.37)
  ping:setTopBottom(true, false, 2, 20)
  ping:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
  ping:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)
  ping:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
  ping:linkToElementModel(self, "ping", true, function(model)
    local _ping = Engine.GetModelValue(model)
    if _ping then
      ping:setText(Engine.Localize(_ping))
      local pingVal = tonumber(_ping) or 0

      if pingVal <= 100 then
        ping:setRGB(0.0, 1.0, 0.0)
      elseif pingVal <= 150 then
        ping:setRGB(1.0, 0.9, 0.0)
      elseif pingVal <= 200 then
        ping:setRGB(1.0, 0.5, 0.0)
      else
        ping:setRGB(1.0, 0.0, 0.0)
      end
    end
  end)
  self:addElement(ping)
  self.ping = ping

  spacer.id = "spacer"
  self:registerEventHandler("gain_focus", function(self, event)
    if self.m_focusable and self.spacer:processEvent(event) then
      return true
    else
      return LUI.UIElement.gainFocus(self, event)
    end
  end)
  LUI.OverrideFunction_CallOriginalSecond(self, "close", function(element)
    element.passwordFlag:close()
    element.dedicatedFlag:close()
    element.rankedFlag:close()
    element.name:close()
    element.map:close()
    element.hardcoreFlag:close()
    element.gametype:close()
    element.playerCount:close()
    element.maxPlayers:close()
    element.botCount:close()
    element.ping:close()
  end)

  if PostLoadFunc then
    PostLoadFunc(self, controller, menu)
  end

  return self
end

if SB.HeaderNew then
  CoD.ServerBrowserHeader.new = function(menu, controller)
    local ok, self = pcall(SB.HeaderNew, menu, controller)
    if not ok or not self then
      return self
    end

    if self.skull then
      self.skull:registerEventHandler("button_action", function(element, event)
        pcall(function()
          if skullSortAscending == nil then
            skullSortAscending = false
          elseif skullSortAscending == false then
            skullSortAscending = true
          else
            skullSortAscending = nil
            skullSortedOrder = nil
            if activeServerList then
              activeServerList:updateDataSource(false, false)
            end
            return
          end

          local countModel = Engine.GetModel(Engine.GetGlobalModel(), "serverBrowser.serverListCount")
          local totalCount = countModel and Engine.GetModelValue(countModel) or 0

          local indicesToSort = {}
          if filteredServerIndices then
            for _, idx in ipairs(filteredServerIndices) do
              table.insert(indicesToSort, idx)
            end
          else
            for i = 0, totalCount - 1 do
              table.insert(indicesToSort, i)
            end
          end

          local entries = {}
          for _, engineIdx in ipairs(indicesToSort) do
            local info
            if isCustomTab() then
              info = game.getrawserverinfo(engineIdx)
            else
              info = Engine.SteamServerBrowser_GetServerInfo(engineIdx)
            end
            if info then
              local sortVal
              if info.zombies then
                sortVal = info.rounds or 0
              else
                sortVal = info.hardcore and 1 or 0
              end
              table.insert(entries, { idx = engineIdx, val = sortVal })
            end
          end

          if skullSortAscending then
            table.sort(entries, function(a, b)
              return a.val < b.val
            end)
          else
            table.sort(entries, function(a, b)
              return a.val > b.val
            end)
          end

          skullSortedOrder = {}
          for i, entry in ipairs(entries) do
            skullSortedOrder[i] = entry.idx
          end

          if activeServerList then
            activeServerList:updateDataSource(false, false)
          end
        end)
      end)
    end

    return self
  end
end

Engine.SteamServerBrowser_RequestPlayersInfo = function(serverIndex) end

if SB.CreateLobbyBrowser then
  LUI.createMenu.LobbyServerBrowserOnline = function(...)
    local ok, self = pcall(SB.CreateLobbyBrowser, ...)
    if not ok or not self then
      return self
    end
    pcall(function()
      if self.Tabs and self.Tabs.Tabs and self.Tabs.Tabs.grid then
        self.Tabs.Tabs.grid:setHorizontalCount(10)
      end
    end)
    return self
  end
end

-- Override filter data sources to be mode-aware (zombie/campaign/all)
pcall(function()
  local freerunMaps = {
    mp_freerun_01 = true,
    mp_freerun_02 = true,
    mp_freerun_03 = true,
    mp_freerun_04 = true,
  }

  local function getSessionModeForTab()
    if currentCustomMode == "zm" then
      return Enum.eModes.MODE_ZOMBIES
    elseif currentCustomMode == "cp" then
      return Enum.eModes.MODE_CAMPAIGN
    else
      return Enum.eModes.MODE_MULTIPLAYER
    end
  end

  local function isAllModeFilter()
    return currentCustomMode == nil or currentCustomMode == "all"
  end

  local function mapFilterItems(dlc)
    local items = {}
    local maps = CoD.mapsTable or Engine.GetGDTMapsTable()
    local sessionMode = getSessionModeForTab()
    local sorted = {}
    local seenNames = {}
    for key, data in pairs(maps) do
      local show = isAllModeFilter() or data.session_mode == sessionMode
      if show and not freerunMaps[key] and ((data.dlc_pack or 0) > 0) == dlc and not seenNames[data.mapName] then
        seenNames[data.mapName] = true
        table.insert(sorted, { key = key, data = data })
      end
    end
    table.sort(sorted, function(a, b)
      return (a.data.unique_id or 0) < (b.data.unique_id or 0)
    end)
    for _, entry in ipairs(sorted) do
      table.insert(items, {
        models = {
          type = Enum.SteamServerFilterType.STEAM_SERVER_BROWSER_FILTERTYPE_MAP,
          id = entry.key,
          name = entry.data.mapName,
        },
      })
    end
    return items
  end

  DataSources.ServerBrowserFilter = DataSourceHelpers.ListSetup("ServerBrowserFilter", function(controller)
    local items = mapFilterItems(false)
    table.insert(items, 1, {
      models = {
        type = Enum.SteamServerFilterType.STEAM_SERVER_BROWSER_FILTERTYPE_MAP,
        id = "any",
        name = "PLATFORM_ANY",
      },
    })
    return items
  end, false)

  DataSources.ServerBrowserDLCFilter = DataSourceHelpers.ListSetup("ServerBrowserDLCFilter", function(controller)
    return mapFilterItems(true)
  end, false)

  -- Game modes
  DataSources.ServerBrowserGameModeFilter = DataSourceHelpers.ListSetup(
    "ServerBrowserGameModeFilter",
    function(controller)
      local items = {}
      table.insert(items, {
        models = {
          type = Enum.SteamServerFilterType.STEAM_SERVER_BROWSER_FILTERTYPE_GAMETYPE,
          id = "any",
          name = "PLATFORM_ANY",
        },
      })
      local sessionMode = getSessionModeForTab()
      local modes = {}
      if isAllModeFilter() then
        -- Show gametypes from all modes
        for _, mode in ipairs({ Enum.eModes.MODE_MULTIPLAYER, Enum.eModes.MODE_ZOMBIES }) do
          local ok, gametypes = pcall(Engine.GetGametypesBase, mode)
          if ok and gametypes then
            for _, gt in pairs(gametypes) do
              if not modes[gt.gametype] then
                modes[gt.gametype] = gt
              end
            end
          end
        end
      else
        local ok, gametypes = pcall(Engine.GetGametypesBase, sessionMode)
        if ok and gametypes then
          for _, gt in pairs(gametypes) do
            modes[gt.gametype] = gt
          end
        end
      end
      local sorted = {}
      for _, gt in pairs(modes) do
        table.insert(sorted, gt)
      end
      table.sort(sorted, function(a, b)
        return a.gametype < b.gametype
      end)
      for _, gt in ipairs(sorted) do
        if not CoD.AllowGameType or CoD.AllowGameType(gt.gametype) then
          table.insert(items, {
            models = {
              type = Enum.SteamServerFilterType.STEAM_SERVER_BROWSER_FILTERTYPE_GAMETYPE,
              id = gt.gametype,
              name = gt.name,
            },
          })
        end
      end
      return items
    end,
    false
  )
end)
