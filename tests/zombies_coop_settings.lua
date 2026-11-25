local root = assert(arg[1], "pass the checkout directory")
local state = {
  mode = 1,
  host = true,
  active = true,
  map = "core_frontend",
  screen = 402,
  limit = 4,
  capacity = 8,
  enabled = true,
  launching = false,
}
local writes, opened, updated = {}, {}, 0
Enum = {
  eModes = { MODE_ZOMBIES = 1, MODE_MULTIPLAYER = 2, MODE_CAMPAIGN = 3 },
  LobbyType = { LOBBY_TYPE_GAME = 7 },
  LobbyMainMode = { LOBBY_MAINMODE_ZM = 1 },
  LobbyMode = { LOBBY_MODE_CUSTOM = 3, LOBBY_MODE_PUBLIC = 4 },
}
Engine = {
  GetCurrentMap = function()
    return state.map
  end,
  CurrentSessionMode = function()
    return state.mode
  end,
  IsInGame = function()
    return false
  end,
  IsLobbyActive = function()
    return state.active
  end,
  IsLobbyHost = function()
    return state.host
  end,
  GetLobbyUIScreen = function()
    return state.screen
  end,
  LobbyHostLaunchTaskInProgress = function()
    return state.launching
  end,
  DvarInt = function(_, name)
    assert(name == "zm_coopPlayerLimit")
    return state.limit
  end,
  SetDvar = function(name, value)
    assert(name == "zm_coopPlayerLimit", "settings must not write live game/party capacity")
    table.insert(writes, value)
    state.limit = value
  end,
  SetLobbyMaxClients = function()
    error("settings must not resize an existing native lobby")
  end,
}
game = {
  cooplobbyinfo = function()
    return { enabled = state.enabled, canAdvertise = false, advertise = false }
  end,
}
LobbyData = { UITargets = {} }
for name, id in pairs({
  UI_ZMLOBBYONLINE = 400,
  UI_ZMLOBBYONLINEPUBLICGAME = 401,
  UI_ZMLOBBYONLINECUSTOMGAME = 402,
  UI_ZMLOBBYLANGAME = 410,
}) do
  LobbyData.UITargets[name] = { id = id, maxClients = 4, mainMode = 1, lobbyType = 7, lobbyMode = id == 401 and 4 or 3 }
end
local created
CoD = {
  Menu = {},
  OptionsUtility = {
    CreateDvarSettings = function(controller, title, description, id, dvarName, values, _, callback)
      created =
        { title = title, description = description, id = id, dvarName = dvarName, values = values, callback = callback }
      return created
    end,
  },
  LobbyMenus = {
    AddButtonsForTarget = function()
      return { { customId = "stock" } }, "stock result", nil, "tail"
    end,
  },
}
LUI = { createMenu = {} }
DataSources = {}
DataSourceHelpers = {
  ListSetup = function(_, builder)
    return { build = builder }
  end,
}
UpdateInfoModels = function()
  updated = updated + 1
end
OpenPopup = function(menu, name, controller)
  table.insert(opened, { menu = menu, name = name, controller = controller })
end

local settings = dofile(root .. "/data/ui_scripts/party/zombies_coop_settings.lua")
assert(settings.install())
local buttonBuilder = CoD.LobbyMenus.AddButtonsForTarget
assert(settings.install() and CoD.LobbyMenus.AddButtonsForTarget == buttonBuilder)
local source = DataSources.BoiiiZombiesLobbySettings
assert(#source.build(0) == 1 and created.dvarName == "zm_coopPlayerLimit")
assert(created.description:find("Leave and recreate", 1, true))
assert(#created.values == 8)
for index, option in ipairs(created.values) do
  assert(option.value == index and option.option == tostring(index) and option.default == (index == 4))
end
for _, value in ipairs({ 5, 6, 7, 8 }) do
  created.callback(nil, { value = value }, nil, created.dvarName)
  assert(state.limit == value and state.capacity == 8)
  for _, target in pairs(LobbyData.UITargets) do
    assert(target.maxClients == 4)
  end
end
assert(#writes == 4 and updated == 4)
for _, bad in ipairs({ 0, 9, "8", 1.5, false, math.huge, 0 / 0 }) do
  created.callback(nil, { value = bad }, nil, created.dvarName)
end
created.callback(nil, { value = 5 }, nil, "com_maxclients")
assert(#writes == 4)

for _, id in ipairs({ 400, 401, 402, 410 }) do
  state.screen = id
  local buttons, stock = buttonBuilder(0, id)
  assert(#buttons == 2 and buttons[2].customId == "btnCoopLobbySettings" and stock == "stock result")
  assert(select("#", buttonBuilder(0, id)) == 4)
end
state.screen = 402
local button = buttonBuilder(0, 402)[2]
local menu = {}
button.action(nil, nil, 0, nil, menu)
assert(#opened == 1 and opened[1].name == "BoiiiZombiesLobbySettingsMenu" and opened[1].menu == menu)

for _, denied in ipairs({ "mp", "cp", "member", "flagoff", "closed", "launching", "ingame" }) do
  state.mode, state.host, state.active, state.enabled, state.launching, state.map =
    1, true, true, true, false, "core_frontend"
  if denied == "mp" then
    state.mode = 2
  elseif denied == "cp" then
    state.mode = 3
  elseif denied == "member" then
    state.host = false
  elseif denied == "flagoff" then
    state.enabled = false
  elseif denied == "closed" then
    state.active = false
  elseif denied == "launching" then
    state.launching = true
  elseif denied == "ingame" then
    state.map = "zm_zod"
  end
  assert(#source.build(0) == 0 and #buttonBuilder(0, 402) == 1, denied)
  created.callback(nil, { value = 5 }, nil, created.dvarName)
  button.action(nil, nil, 0, nil, menu)
  assert(#writes == 4 and #opened == 1, denied)
end
print(
  "PASS: frontend Zombies settings, host/flag/mode gates, next-size5/6/7/8 callbacks, default4 and zero live-capacity writes"
)
