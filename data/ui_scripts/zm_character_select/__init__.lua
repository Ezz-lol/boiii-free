if Engine.GetCurrentMap() ~= "core_frontend" or not CoD.LobbyBase or not CoD.LobbyMenus then
  return
end

-- Mods like AAE replace the lobby and bring their own character picker
if
  type(Engine.IsUsingMods) == "function"
  and Engine.IsUsingMods()
  and (type(Engine.UsingModsUgcName) ~= "function" or Engine.UsingModsUgcName() ~= "usermaps")
then
  return
end

local PRIMIS_HEROES = { 0, 1, 2, 3 }
-- Moon is left out since its Easter egg needs a Richtofen, which its own assignment always hands out
local MAP_HEROES = {
  zm_zod = { 5, 6, 7, 8 },
  zm_factory = PRIMIS_HEROES,
  zm_castle = PRIMIS_HEROES,
  zm_island = PRIMIS_HEROES,
  zm_stalingrad = PRIMIS_HEROES,
  zm_genesis = PRIMIS_HEROES,
  zm_tomb = PRIMIS_HEROES,
  zm_prototype = PRIMIS_HEROES,
  zm_asylum = PRIMIS_HEROES,
  zm_sumpf = PRIMIS_HEROES,
  zm_theater = PRIMIS_HEROES,
  zm_cosmodrome = PRIMIS_HEROES,
  zm_temple = PRIMIS_HEROES,
}

local pending = {}

local function mapHeroes()
  return MAP_HEROES[Engine.GameLobbyGetMap()]
end

-- Keeps long names inside the button
local function shortName(name)
  if #name > 16 then
    name = name:sub(1, 15) .. "..."
  end
  return name
end

local function heroesTakenByOthers(controller)
  local taken = {}
  local localXuids = {}
  for other = 0, Engine.GetMaxLocalControllers() - 1 do
    if Engine.IsControllerBeingUsed(other) then
      localXuids[tostring(Engine.GetXUID64(other))] = true
      if other ~= controller then
        taken[Engine.GetEquippedHero(other, Enum.eModes.MODE_ZOMBIES)] = Engine.GetGamertagForController(other)
          or "TAKEN"
      end
    end
  end
  if not Engine.IsLobbyActive(Enum.LobbyType.LOBBY_TYPE_GAME) then
    return taken
  end
  -- Each member's equipped hero is part of the client info the lobby syncs to everyone. Split screen players are read
  -- above, since this returns stale data for local clients.
  local lobby = Engine.LobbyGetSessionClients(Enum.LobbyModule.LOBBY_MODULE_CLIENT, Enum.LobbyType.LOBBY_TYPE_GAME)
  for _, client in ipairs(lobby and lobby.sessionClients or {}) do
    if not localXuids[tostring(client.xuid)] then
      local clientNum = Engine.GetLobbyClientNumByXUID(
        Enum.LobbyModule.LOBBY_MODULE_CLIENT,
        Enum.LobbyType.LOBBY_TYPE_GAME,
        client.xuid
      )
      local hero = clientNum and Engine.GetHeroIndexForClientNum(controller, clientNum)
      if hero then
        taken[hero] = client.gamertag or "TAKEN"
      end
    end
  end
  return taken
end

CoD.OverlayUtility.AddSystemOverlay("ZMCharacterSelect", {
  menuName = "SystemOverlay_Full",
  title = "CHOOSE CHARACTER",
  description = "",
  categoryType = CoD.OverlayUtility.OverlayTypes.GenericMessage,
  [CoD.OverlayUtility.GoBackPropertyName] = function()
    return function(_, _, controller, menu)
      pending.onPicked = nil
      GoBack(menu, controller)
    end
  end,
  listDatasource = function()
    DataSources.ZMCharacterSelectList = DataSourceHelpers.ListSetup("ZMCharacterSelectList", function()
      -- The overlay passes the lobby menu's controller, which is player 1's even when player 2 opened it
      local controller = pending.controller
      local heroList = Engine.GetHeroList(Enum.eModes.MODE_ZOMBIES)
      local equipped = Engine.GetEquippedHero(controller, Enum.eModes.MODE_ZOMBIES)
      local taken = heroesTakenByOthers(controller)
      local items = {}
      for _, hero in ipairs(mapHeroes() or {}) do
        local name = Engine.Localize(heroList[hero + 1].displayName)
        if taken[hero] ~= nil then
          name = name .. " (" .. shortName(taken[hero]) .. ")"
        elseif hero == equipped then
          name = name .. " (CURRENT)"
        end
        table.insert(items, {
          models = { displayText = name, disabled = taken[hero] ~= nil },
          properties = {
            selectIndex = hero == equipped and taken[hero] == nil,
            action = function(_, _, menuController, _, menu)
              if heroesTakenByOthers(controller)[hero] ~= nil then
                return
              end
              Engine.SetHero(controller, Enum.eModes.MODE_ZOMBIES, hero)
              -- Other players only see the pick once the lobby is sent this client's info again
              local lobbyType = Enum.LobbyType.LOBBY_TYPE_PRIVATE
              if Engine.InLobbyParty(Enum.LobbyType.LOBBY_TYPE_GAME) then
                lobbyType = Enum.LobbyType.LOBBY_TYPE_GAME
              end
              Engine.SendClientHeartbeat(controller, lobbyType)
              GoBack(menu, menuController)
              local onPicked = pending.onPicked
              pending.onPicked = nil
              if onPicked then
                onPicked()
              else
                -- The lobby only redraws the character in the background when its room is set up again
                Engine.SendClientScriptNotify(
                  controller,
                  "menu_change" .. CoD.GetLocalClientAdjustedNum(controller),
                  "Main",
                  "closeToMenu"
                )
              end
            end,
          },
        })
      end
      return items
    end, true)
    return "ZMCharacterSelectList"
  end,
})

local function openCharacterSelect(menu, controller, onPicked)
  -- The lobby button can outlive a switch to a map that isn't supported
  if not mapHeroes() then
    return
  end
  pending.controller = controller
  pending.onPicked = onPicked
  CoD.OverlayUtility.CreateOverlay(controller, menu, "ZMCharacterSelect")
end

local changeMapParent = nil
local openChangeMap = CoD.LobbyBase.ZMOpenChangeMap
CoD.LobbyBase.ZMOpenChangeMap = function(menu, controller, selectType, data)
  changeMapParent = menu
  return openChangeMap(menu, controller, selectType, data)
end

-- Solo launches as soon as a map is picked, so ask then. Going back from the overlay cancels the launch.
local launchGame = CoD.LobbyBase.LaunchGame
CoD.LobbyBase.LaunchGame = function(menu, controller, lobbyType)
  if menu and menu.selectType == CoD.LobbyBase.MapSelect.LAUNCH and changeMapParent and mapHeroes() then
    openCharacterSelect(changeMapParent, controller, function()
      launchGame(menu, controller, lobbyType)
    end)
    return
  end
  return launchGame(menu, controller, lobbyType)
end

local characterSelectButton = {
  stringRef = "CHOOSE CHARACTER",
  action = function(_, _, controller, _, menu)
    openCharacterSelect(menu, controller, nil)
  end,
  customId = "btnZMCharacterSelect",
}

local addButtonsForTarget = CoD.LobbyMenus.AddButtonsForTarget
CoD.LobbyMenus.AddButtonsForTarget = function(controller, id)
  local result = addButtonsForTarget(controller, id)
  local targets = LobbyData.UITargets
  if
    not result
    or not mapHeroes()
    or (id ~= targets.UI_ZMLOBBYONLINECUSTOMGAME.id and id ~= targets.UI_ZMLOBBYLANGAME.id)
  then
    return result
  end
  local index = #result + 1
  for buttonIndex, button in ipairs(result) do
    if button.optionDisplay == CoD.LobbyButtons.ZM_CHANGE_MAP.stringRef then
      index = buttonIndex + 1
      break
    end
  end
  table.insert(result, index, {
    optionDisplay = characterSelectButton.stringRef,
    action = characterSelectButton.action,
    customId = characterSelectButton.customId,
    isLargeButton = false,
    isLastButtonInGroup = false,
    disabled = false,
    selected = false,
  })
  return result
end

-- The lobby only rebuilds its buttons on a map change in campaign, so the button wouldn't update when switching maps
local prepareLobbyButtons = DataSources.LobbyButtons.prepare
DataSources.LobbyButtons.prepare = function(controller, list, filter)
  prepareLobbyButtons(controller, list, filter)
  if list.characterSelectMapSub then
    list:removeSubscription(list.characterSelectMapSub)
  end
  list.characterSelectSupported = mapHeroes() ~= nil
  local mapModel = Engine.CreateModel(Engine.GetGlobalModel(), "MapVote.mapVoteMapNext")
  list.characterSelectMapSub = list:subscribeToModel(mapModel, function()
    local supported = mapHeroes() ~= nil
    if supported ~= list.characterSelectSupported then
      list.characterSelectSupported = supported
      list:updateDataSource()
    end
  end, false)
end
