if not game or not game.getfriendcount or not game.getfriend then
  return
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
  "uie_t7_callingcard_prestige_skull"
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

local function isFriendsTab()
  local root = Engine.GetModel(Engine.GetGlobalModel(), "socialRoot")
  local tab = root and Engine.GetModel(root, "tab")
  return not tab or Engine.GetModelValue(tab) == "friends"
end

local function presenceForStatus(status)
  if status == 2 then
    return Enum.PresencePrimary.PRESENCE_PRIMARY_TITLE,
      Enum.PresenceActivity.PRESENCE_ACTIVITY_MENU_INLOBBY,
      "In a joinable game"
  elseif status == 1 then
    return Enum.PresencePrimary.PRESENCE_PRIMARY_ONLINE,
      Enum.PresenceActivity.PRESENCE_ACTIVITY_ONLINE_NOT_IN_TITLE,
      "Online"
  end
  return Enum.PresencePrimary.PRESENCE_PRIMARY_OFFLINE,
    Enum.PresenceActivity.PRESENCE_ACTIVITY_OFFLINE,
    "Offline"
end

local function createPlayerSlot(parent, index)
  local root = Engine.CreateModel(parent, "boiii_player_" .. index)
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
    partyMax = 18
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
      party = { members = {}, total = 1, available = 1, leader = "" }
    }
  }
end

local function updatePlayerSlot(controller, slot, friend)
  local xuid = Engine.StringToXUIDDecimal(friend.steam_id)
  local primary, activity, presence = presenceForStatus(friend.status)
  local values = {
    xuid = xuid,
    boiiiFriendIcon = friendEmblems[stableIndex(friend.steam_id, #friendEmblems)],
    backgroundId = friendBackground(controller, friend.steam_id),
    gamertag = friend.name,
    fullname = friend.name,
    primaryPresence = primary,
    activity = activity,
    titlePresence = presence,
    platformPresence = presence,
    joinable = friend.server ~= "" and 1 or 0
  }
  for key, value in pairs(values) do
    Engine.SetModelValue(Engine.GetModel(slot.model, key), value)
  end
  slot.properties.xuid = xuid
end

local customSocialPlayers = {
  prepare = function(controller, list, filter)
    if not isFriendsTab() then
      list.boiiiFriends = false
      return nativeSocialPlayers.prepare(controller, list, filter)
    end

    list.boiiiFriends = true
    list.numElementsInList = list.vCount
    list.playerCount = game.getfriendcount()
    list.players = {}
    local socialRoot = Engine.CreateModel(Engine.GetGlobalModel(), "socialRoot")
    local friendsRoot = Engine.CreateModel(socialRoot, "friends")
    for index = 1, list.numElementsInList do
      list.players[index] = createPlayerSlot(friendsRoot, index)
    end

    list.updateModels = function(_, currentList, offset, count)
      currentList.playerCount = game.getfriendcount()
      local limit = math.min(count, currentList.playerCount - offset)
      for item = 1, limit do
        local slotIndex = (offset + item - 1) % currentList.numElementsInList + 1
        updatePlayerSlot(controller, currentList.players[slotIndex], game.getfriend(offset + item - 1))
      end
      return currentList.players[offset % currentList.numElementsInList + 1].model
    end

    list.updateModels(controller, list, 0, list.numElementsInList)
  end,
  getCount = function(list)
    if list.boiiiFriends then
      list.playerCount = game.getfriendcount()
      return list.playerCount
    end
    return nativeSocialPlayers.getCount(list)
  end,
  getItem = function(controller, list, index)
    if list.boiiiFriends then
      list.updateModels(controller, list, index - 1, 1)
      return list.players[(index - 1) % list.numElementsInList + 1].model
    end
    return nativeSocialPlayers.getItem(controller, list, index)
  end,
  getCustomPropertiesForItem = function(list, index)
    if list.boiiiFriends then
      return list.players[(index - 1) % list.numElementsInList + 1].properties
    end
    return nativeSocialPlayers.getCustomPropertiesForItem(list, index)
  end
}

BoiiiSocialPlayersList = customSocialPlayers
DataSources.SocialPlayersList = customSocialPlayers

local nativeHasFriends = HasFriends
HasFriends = function(controller)
  if isFriendsTab() then
    return game.getfriendcount() > 0
  end
  return nativeHasFriends and nativeHasFriends(controller) or false
end

local nativeListHasPlayers = IsSocialPlayersListEmpty
IsSocialPlayersListEmpty = function(controller)
  if isFriendsTab() then
    return game.getfriendcount() > 0
  end
  return nativeListHasPlayers and nativeListHasPlayers(controller) or false
end

local nativeSocialJoin = SocialJoin
SocialJoin = function(menu, element, controller, param, parentMenu)
  local hex = param and param.xuid and Engine.UInt64ToString(param.xuid)
  if hex and game.issocialfriend and game.issocialfriend(hex) then
    game.connectsocialfriend(hex)
    GoBackToMenu(GoBack(menu, controller), controller, "Lobby")
    return
  end
  return nativeSocialJoin(menu, element, controller, param, parentMenu)
end

local nativeLobbyQuickJoin = LobbyQuickJoin
LobbyQuickJoin = function(menu, element, controller, joinType, closeMenu)
  local xuidModel = element and element.getModel and element:getModel(controller, "xuid")
  local xuid = xuidModel and Engine.GetModelValue(xuidModel)
  local hex = xuid and Engine.UInt64ToString(xuid)
  if hex and game.issocialfriend and game.issocialfriend(hex) then
    game.connectsocialfriend(hex)
    if closeMenu then
      GoBack(menu, controller)
    end
    return
  end
  return nativeLobbyQuickJoin(menu, element, controller, joinType, closeMenu)
end
