if not game or not game.getfriendcount or not game.getrecentplayer then
  return
end

IsGroupsEnabled = function()
  return false
end

require("ui.uieditor.datasources")
require("ui.uieditor.actions")
require("ui.uieditor.widgets.Social.Social_PlayersListButton")
require("ui.uieditor.widgets.Social.Social_InfoPane")

local friendEmblems = {
  "uie_t7_mp_hud_faction_icon_faction1",
  "uie_t7_mp_hud_faction_icon_faction2_widget",
  "uie_t7_codcaster_faction1",
  "uie_t7_codcaster_faction2",
  "uie_t7_icons_classification_specialists",
  "uie_t7_callingcard_prestige_skull",
}

local function stableIndex(value, count)
  local hash = 5381
  for index = 1, #value do
    hash = (hash * 33 + string.byte(value, index)) % 2147483647
  end
  return hash % count + 1
end

local defaultBackgrounds = {}
local function friendBackground(controller, xuid)
  if not defaultBackgrounds[controller] then
    local available = Engine.GetBackgroundsForCategoryName(controller, "default") or {}
    local unlocked = {}
    for _, background in ipairs(available) do
      if not background.isBGLocked then
        table.insert(unlocked, background.id)
      end
    end
    defaultBackgrounds[controller] = unlocked
  end
  local backgrounds = defaultBackgrounds[controller]
  if #backgrounds == 0 then
    return 0
  end
  return backgrounds[stableIndex(xuid, #backgrounds)]
end

local function wrapFriendEmblemWidget(widget, right, bottom)
  if not widget or type(widget.new) ~= "function" or widget.boiiiEmblemsWrapped then
    return
  end
  widget.boiiiEmblemsWrapped = true
  local nativeNew = widget.new
  widget.new = function(menu, controller)
    local instance = nativeNew(menu, controller)
    if instance.emblem then
      local background = LUI.UIImage.new()
      background:setLeftRight(true, false, 0, right)
      background:setTopBottom(true, false, 0, bottom)
      background:setRGB(0.06, 0.08, 0.1)
      background:setImage(RegisterImage("uie_default_white_255"))
      background:setAlpha(0)
      background:linkToElementModel(instance, "boiiiFriendIcon", true, function(model)
        local icon = Engine.GetModelValue(model)
        background:setAlpha(icon and icon ~= "" and 1 or 0)
      end)
      instance:addElement(background)

      local fallback = LUI.UIImage.new()
      fallback:setLeftRight(true, false, 0, right)
      fallback:setTopBottom(true, false, 0, bottom)
      fallback:setAlpha(0)
      fallback:linkToElementModel(instance, "boiiiFriendIcon", true, function(model)
        local icon = Engine.GetModelValue(model)
        if icon and icon ~= "" then
          fallback:setImage(RegisterImage(icon))
          fallback:setAlpha(1)
        else
          fallback:setAlpha(0)
        end
      end)
      instance:addElement(fallback)
      instance.boiiiFriendEmblemBackground = background
      instance.boiiiFriendEmblem = fallback
    end
    return instance
  end
end

wrapFriendEmblemWidget(CoD.Social_PlayersListButton, 96, 60)
wrapFriendEmblemWidget(CoD.Social_InfoPane, 324, 191)

local nativeSocialPlayers = DataSources.SocialPlayersList
if BoiiiSocialPlayersList then
  DataSources.SocialPlayersList = BoiiiSocialPlayersList
  return
end
if not nativeSocialPlayers then
  return
end

local function currentTab()
  local root = Engine.GetModel(Engine.GetGlobalModel(), "socialRoot")
  local tab = root and Engine.GetModel(root, "tab")
  return tab and Engine.GetModelValue(tab) or "friends"
end

local function isOurTab()
  local tab = currentTab()
  return tab == "friends" or tab == "recent"
end

local function hexOf(xuid)
  return xuid and Engine.UInt64ToString(xuid)
end

local function presenceForFriend(friend)
  if friend.invited and friend.status == 2 then
    return Enum.PresencePrimary.PRESENCE_PRIMARY_TITLE,
      Enum.PresenceActivity.PRESENCE_ACTIVITY_MENU_INLOBBY,
      "^2Invited you to play"
  elseif friend.status == 2 then
    return Enum.PresencePrimary.PRESENCE_PRIMARY_TITLE,
      Enum.PresenceActivity.PRESENCE_ACTIVITY_MENU_INLOBBY,
      "In a joinable game"
  elseif friend.status == 1 then
    return Enum.PresencePrimary.PRESENCE_PRIMARY_ONLINE,
      Enum.PresenceActivity.PRESENCE_ACTIVITY_ONLINE_NOT_IN_TITLE,
      "^3Online, party closed"
  end
  return Enum.PresencePrimary.PRESENCE_PRIMARY_OFFLINE, Enum.PresenceActivity.PRESENCE_ACTIVITY_OFFLINE, "Offline"
end

local function createPlayerSlot(parent, index)
  local name = "boiii_player_" .. index
  local previous = Engine.GetModel(parent, name)
  if previous then
    Engine.UnsubscribeAndFreeModel(previous)
  end
  local root = Engine.CreateModel(parent, name)
  local model = Engine.CreateModel(root, "model")
  local fields = {
    xuid = Engine.StringToXUIDDecimal("0"),
    boiiiFriendIcon = "",
    backgroundId = 0,
    gamertag = "",
    clantag = "",
    fullname = "",
    activity = Enum.PresenceActivity.PRESENCE_ACTIVITY_OFFLINE,
    context = 0,
    difficulty = 0,
    playlist = 1,
    joinable = 0,
    gametype = 0,
    mapid = 0,
    friend = 1,
    primaryPresence = Enum.PresencePrimary.PRESENCE_PRIMARY_OFFLINE,
    titlePresence = "",
    platformPresence = "",
    cpPlayed = false,
    cpRank = 0,
    cpRankIcon = "",
    cpPrestige = 0,
    mpPlayed = false,
    mpRank = 0,
    mpRankIcon = "",
    mpPrestige = 0,
    zmPlayed = false,
    zmRank = 0,
    zmRankIcon = "",
    zmPrestige = 0,
    partySize = 1,
    partyMax = 18,
  }
  for key, value in pairs(fields) do
    Engine.SetModelValue(Engine.CreateModel(model, key), value)
  end
  return {
    model = model,
    properties = {
      xuid = Engine.StringToXUIDDecimal("0"),
      showyourfriend = 0,
      showlastmet = 0,
      gametype = 0,
      mapid = 0,
      difficulty = 0,
      playlist = 1,
      party = { members = {}, total = 1, available = 1, leader = "" },
    },
  }
end

local function updatePlayerSlot(controller, slot, player)
  local xuid = Engine.StringToXUIDDecimal(player.steam_id)
  local primary, activity, presence = presenceForFriend(player)
  if player.seen then
    presence = (player.friend and "Friend, " or "") .. "last seen " .. player.seen
  end
  local values = {
    xuid = xuid,
    boiiiFriendIcon = friendEmblems[stableIndex(player.steam_id, #friendEmblems)],
    backgroundId = friendBackground(controller, player.steam_id),
    gamertag = player.name,
    fullname = player.name,
    friend = player.friend == false and 0 or 1,
    primaryPresence = primary,
    activity = activity,
    titlePresence = presence,
    platformPresence = presence,
    joinable = (player.server or "") ~= "" and 1 or 0,
  }
  for key, value in pairs(values) do
    Engine.SetModelValue(Engine.GetModel(slot.model, key), value)
  end
  slot.properties.xuid = xuid
  slot.properties.showyourfriend = player.seen and 1 or 0
  slot.properties.showlastmet = player.seen and 1 or 0
end

local function sourceForTab(tab)
  if tab == "recent" then
    return {
      count = game.getrecentcount,
      get = game.getrecentplayer,
    }
  end
  return {
    count = game.getfriendcount,
    get = game.getfriend,
  }
end

local customSocialPlayers = {
  prepare = function(controller, list, filter)
    if not isOurTab() then
      list.boiiiPlayers = nil
      return nativeSocialPlayers.prepare(controller, list, filter)
    end

    local tab = currentTab()
    list.boiiiPlayers = sourceForTab(tab)
    list.numElementsInList = list.vCount
    list.playerCount = list.boiiiPlayers.count()
    list.players = {}
    local socialRoot = Engine.CreateModel(Engine.GetGlobalModel(), "socialRoot")
    local tabRoot = Engine.CreateModel(socialRoot, tab == "recent" and "recentPlayers" or "friends")
    local updateModel = Engine.CreateModel(Engine.CreateModel(socialRoot, "friends"), "update")
    for index = 1, list.numElementsInList do
      list.players[index] = createPlayerSlot(tabRoot, index)
    end

    list.updateModels = function(_, currentList, offset, count)
      currentList.playerCount = currentList.boiiiPlayers.count()
      local limit = math.min(count, currentList.playerCount - offset)
      for item = 1, limit do
        local slotIndex = (offset + item - 1) % currentList.numElementsInList + 1
        updatePlayerSlot(controller, currentList.players[slotIndex], currentList.boiiiPlayers.get(offset + item - 1))
      end
      return currentList.players[offset % currentList.numElementsInList + 1].model
    end

    list.updateModels(controller, list, 0, list.numElementsInList)
    if list.socialUpdateSubscription then
      list:removeSubscription(list.socialUpdateSubscription)
    end
    list.socialUpdateSubscription = list:subscribeToModel(updateModel, function()
      if list.boiiiPlayers then
        list.boiiiPlayers = sourceForTab(currentTab())
        RefreshListFindSelectedXuid(controller, list)
      end
    end, false)
    if not list.boiiiFriendsRefreshTimer then
      list.boiiiFriendsRefreshTimer = LUI.UITimer.newElementTimer(5000, false, function()
        if list.boiiiPlayers then
          game.refreshfriends()
        end
      end)
      list:addElement(list.boiiiFriendsRefreshTimer)
    end
    game.refreshfriends()
  end,
  getCount = function(list)
    if list.boiiiPlayers then
      list.playerCount = list.boiiiPlayers.count()
      return list.playerCount
    end
    return nativeSocialPlayers.getCount(list)
  end,
  getItem = function(controller, list, index)
    if list.boiiiPlayers then
      list.updateModels(controller, list, index - 1, 1)
      return list.players[(index - 1) % list.numElementsInList + 1].model
    end
    return nativeSocialPlayers.getItem(controller, list, index)
  end,
  getCustomPropertiesForItem = function(list, index)
    if list.boiiiPlayers then
      return list.players[(index - 1) % list.numElementsInList + 1].properties
    end
    return nativeSocialPlayers.getCustomPropertiesForItem(list, index)
  end,
  cleanup = function(list)
    list.boiiiPlayers = nil
    if list.socialUpdateSubscription then
      list:removeSubscription(list.socialUpdateSubscription)
      list.socialUpdateSubscription = nil
    end
    if list.boiiiFriendsRefreshTimer then
      list.boiiiFriendsRefreshTimer:close()
      list.boiiiFriendsRefreshTimer = nil
    end
    if nativeSocialPlayers.cleanup then
      return nativeSocialPlayers.cleanup(list)
    end
  end,
}

BoiiiSocialPlayersList = customSocialPlayers
DataSources.SocialPlayersList = customSocialPlayers

local function ourPlayerCount()
  if currentTab() == "recent" then
    return game.getrecentcount()
  end
  return game.getfriendcount()
end

local nativeHasFriends = HasFriends
HasFriends = function(controller)
  if isOurTab() then
    return ourPlayerCount() > 0
  end
  return nativeHasFriends and nativeHasFriends(controller) or false
end

HasRecentPlayers = function()
  return game.getrecentcount() > 0
end

local nativeListHasPlayers = IsSocialPlayersListEmpty
IsSocialPlayersListEmpty = function(controller)
  if isOurTab() then
    return ourPlayerCount() > 0
  end
  return nativeListHasPlayers and nativeListHasPlayers(controller) or false
end

local function joinPlayer(hex)
  local player = hex and game.getsocialplayer(hex)
  if not player or not player.friend then
    return false
  end
  game.connectsocialfriend(hex)
  return true
end

local nativeSocialJoin = SocialJoin
SocialJoin = function(menu, element, controller, param, parentMenu)
  if joinPlayer(param and hexOf(param.xuid)) then
    GoBackToMenu(GoBack(menu, controller), controller, "Lobby")
    return
  end
  return nativeSocialJoin(menu, element, controller, param, parentMenu)
end

local nativeLobbyQuickJoin = LobbyQuickJoin
LobbyQuickJoin = function(menu, element, controller, joinType, closeMenu)
  local xuidModel = element and element.getModel and element:getModel(controller, "xuid")
  if joinPlayer(xuidModel and hexOf(Engine.GetModelValue(xuidModel))) then
    if closeMenu then
      GoBack(menu, controller)
    end
    return
  end
  return nativeLobbyQuickJoin(menu, element, controller, joinType, closeMenu)
end

LobbyInviteFriend = function(menu, element, controller, param)
  local xuid = param and param.xuid
  if not xuid and element then
    xuid = Engine.GetModelValue(element:getModel(controller, "xuid"))
  end
  if xuid then
    game.invitefriend(game.getsocialplayer(hexOf(xuid)).steam_id)
  end
end

local function goBackAfter(action)
  return function(menu, element, controller, param)
    action(param)
    GoBack(menu, controller)
  end
end

local function detailsButtons(controller)
  local buttons = {}
  local controllerModel = Engine.GetModelForController(controller)
  local xuid = Engine.GetModelValue(Engine.CreateModel(controllerModel, "Social.selectedFriendXUID"))
  if not xuid or xuid == Engine.GetXUID64(controller) then
    return buttons
  end

  local player = game.getsocialplayer(hexOf(xuid))
  local params = { controller = controller, xuid = xuid, steamId = player.steam_id, hex = hexOf(xuid) }
  local inParty = Engine.LobbyIsPlayerInLobby(Enum.LobbyType.LOBBY_TYPE_PRIVATE, xuid)
  local inGameLobby = Engine.LobbyIsPlayerInLobby(Enum.LobbyType.LOBBY_TYPE_GAME, xuid)
  local function add(text, action, lastInGroup)
    table.insert(buttons, { text = text, action = action, params = params, lastInGroup = lastInGroup })
  end

  if inParty and not Engine.IsInGame() and Engine.IsLeader(controller, Enum.LobbyType.LOBBY_TYPE_PRIVATE) then
    add("MENU_PROMOTE_TO_PARTY_LEADER", PromoteToLeader)
    if not Engine.IsLocalClient(xuid) then
      add("MENU_REMOVE_FROM_PARTY", DisconnectClient, true)
    end
  end

  if player.friend and player.server ~= "" and not inParty and not inGameLobby then
    add(player.invited and "Accept Invite" or "MENU_JOIN_GAME", function(menu, element, controllerIndex)
      joinPlayer(params.hex)
      GoBackToMenu(GoBack(menu, controllerIndex), controllerIndex, "Lobby")
    end)
  end

  if not inParty and not inGameLobby then
    add(
      Engine.IsLobbyActive(Enum.LobbyType.LOBBY_TYPE_GAME) and "MENU_INVITE_GAME" or "MENU_INVITE_TO_PARTY",
      goBackAfter(function(p)
        game.invitefriend(p.steamId)
      end),
      true
    )
  end

  if player.friend then
    add(
      "Remove Friend",
      goBackAfter(function(p)
        game.removefriend(p.steamId)
      end)
    )
  else
    add(
      "MENU_SEND_FRIEND_REQUEST",
      goBackAfter(function(p)
        game.addfriend(p.steamId, player.name)
      end)
    )
  end

  add(
    "Copy Friend Code",
    goBackAfter(function(p)
      FileIO.ClipboardSet(p.steamId)
    end),
    not (inParty or inGameLobby)
  )

  if inParty or inGameLobby then
    local lobby = Engine.IsLobbyActive(Enum.LobbyType.LOBBY_TYPE_GAME) and Enum.LobbyType.LOBBY_TYPE_GAME
      or Enum.LobbyType.LOBBY_TYPE_PRIVATE
    if Engine.IsPlayerMuted(controller, lobby, xuid) then
      add("MENU_UNMUTE_PLAYER", UnMutePlayer, true)
    else
      add("MENU_MUTE_PLAYER", MutePlayer, true)
    end
  end

  local items = {}
  for _, button in ipairs(buttons) do
    table.insert(items, {
      models = { displayText = Engine.Localize(button.text) },
      properties = {
        action = button.action,
        actionParam = button.params,
        isLastButtonInGroup = button.lastInGroup,
      },
    })
  end
  return items
end

local nativeDetailsButtons = DataSources.SocialPlayerDetailsButtons
if nativeDetailsButtons then
  DataSources.SocialPlayerDetailsButtons = ListHelper_SetupDataSource(
    "SocialPlayerDetailsButtons",
    detailsButtons,
    nil,
    nil,
    nil,
    nativeDetailsButtons.getSpacerAfterRow
  )
end

local nativeSocialTabs = DataSources.SocialTabs
if nativeSocialTabs then
  local nativePrepare = nativeSocialTabs.prepare
  nativeSocialTabs.prepare = function(controller, list, filter)
    nativePrepare(controller, list, filter)
    for _, item in ipairs(list.SocialTabs or {}) do
      if item.properties and item.properties.tabId == "party" then
        item.properties.disabled = false
      end
    end
  end
end

if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

require("ui.uieditor.widgets.Social.Social_Party_PC")

local function partyLabel()
  return Engine.DvarBool(nil, "friends_open") and "^2PARTY IS OPEN" or "^1PARTY IS CLOSED"
end

local function managePartyButton(label, action)
  return {
    models = {
      label = label,
      profileType = "user",
      widgetType = "button",
      onPressFn = function(element, controller)
        ProcessListAction(element.gridInfoTable.parentGrid.menu.TabFrame.framedWidget, element, controller)
      end,
    },
    properties = { action = action },
  }
end

local nativePartyControls = DataSources.PartyControlsPCList
if nativePartyControls then
  DataSources.PartyControlsPCList = DataSourceHelpers.ListSetup("PartyControlsPCList", function(controller)
    local items = {
      managePartyButton(partyLabel(), function(_, element, controllerIndex)
        Engine.ExecNow(controllerIndex, "friends_open")
        Engine.SetModelValue(element:getModel(controllerIndex, "label"), partyLabel())
      end),
    }
    if ShouldShowPartyPrivacy(controller) then
      table.insert(items, {
        models = {
          label = "MENU_PLAYER_LIMIT_CAPS",
          profileVarName = "party_maxplayers",
          profileType = "user",
          datasource = "SocialPartyMaxSizePresets",
          widgetType = "dropdown",
        },
        properties = CoD.PCUtil.OptionsGenericDropdownProperties,
      })
    end
    if ShouldShowLeaveParty(controller) then
      table.insert(
        items,
        managePartyButton(
          Engine.ToUpper(Engine.Localize("MENU_MANAGE_PARTY_LEAVE_BUTTON")),
          function(_, _, controllerIndex, _, menu)
            CoD.OverlayUtility.CreateOverlay(
              controllerIndex,
              menu,
              "LobbyLeavePopup",
              LuaEnums.LEAVE_LOBBY_POPUP.LEAVE_PARTY
            )
          end
        )
      )
    end
    if ShouldShowBootPlayer(controller) then
      table.insert(
        items,
        managePartyButton(
          Engine.ToUpper(Engine.Localize("MENU_MANAGE_PARTY_KICK_BUTTON")),
          function(self, element, controllerIndex, _, menu)
            ShowManagePartyPopup(menu, self, element, controllerIndex, "KICK")
          end
        )
      )
    end
    return items
  end, true)
  DataSources.PartyControlsPCList.getWidgetTypeForItem = nativePartyControls.getWidgetTypeForItem
end
