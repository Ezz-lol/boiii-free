local root = assert(arg[1], "pass the checkout directory")
local function fixture()
  BoiiiZombiesCoopPolicy = nil
  local state = {
    limit = 8,
    mode = 1,
    host = true,
    capacity = 4,
    active = true,
    inGame = false,
    launching = false,
    recovery = false,
    map = "core_frontend",
    screen = 402,
    playlist = 1,
  }
  local writes = {}
  Enum = {
    eModes = { MODE_ZOMBIES = 1, MODE_MULTIPLAYER = 2 },
    LobbyMainMode = { LOBBY_MAINMODE_ZM = 1, LOBBY_MAINMODE_MP = 2 },
    LobbyType = { LOBBY_TYPE_GAME = 7, LOBBY_TYPE_PRIVATE = 8 },
    LobbyMode = { LOBBY_MODE_CUSTOM = 3, LOBBY_MODE_PUBLIC = 4, LOBBY_MODE_THEATER = 5 },
    LobbyNetworkMode = { LOBBY_NETWORKMODE_LIVE = 1, LOBBY_NETWORKMODE_LAN = 2 },
  }
  Engine = {
    DvarInt = function(name, key)
      return key == "zm_coopPlayerLimit" and state.limit or 0
    end,
    CurrentSessionMode = function()
      return state.mode
    end,
    IsInGame = function()
      return state.inGame
    end,
    GetCurrentMap = function()
      return state.map
    end,
    IsLobbyActive = function()
      return state.active
    end,
    IsLobbyHost = function()
      return state.host
    end,
    GetLobbyMaxClients = function()
      return state.capacity
    end,
    LobbyHostLaunchTaskInProgress = function()
      return state.launching
    end,
    IsLobbyInRecovery = function()
      return state.recovery
    end,
    GetLobbyUIScreen = function()
      return state.screen
    end,
    GetPlaylistID = function()
      return state.playlist
    end,
    SetLobbyMaxClients = function(kind, value)
      assert(kind == 7)
      state.capacity = value
    end,
    SetDvar = function(name, value)
      writes[name] = value
    end,
  }
  local function target(id, mode, capacity, mainMode, networkMode)
    return {
      id = id,
      mainMode = mainMode or 1,
      lobbyType = 7,
      lobbyMode = mode,
      networkMode = networkMode or 1,
      maxClients = capacity,
      maxLocalClients = 2,
      maxLocalClientsNetwork = 2,
    }
  end
  LobbyData = {
    UITargets = {
      UI_ZMLOBBYONLINE = target(400, 3, 4),
      UI_ZMLOBBYONLINEPUBLICGAME = target(401, 4, 4),
      UI_ZMLOBBYONLINECUSTOMGAME = target(402, 3, 4),
      UI_ZMLOBBYONLINETHEATER = target(403, 5, 1),
      UI_ZMLOBBYLANGAME = target(410, 3, 4, 1, 2),
      UI_ZM_UNKNOWN = target(450, 3, 4),
      UI_MPLOBBY = target(300, 3, 18, 2),
    },
  }
  LobbyData.UITargetFromId = function(self, id)
    for _, item in pairs(self.UITargets) do
      if item.id == id then
        return item
      end
    end
  end
  CoD, DataSources, game = {}, {}, nil
  local policy = dofile(root .. "/data/ui_scripts/party/zombies_coop.lua")
  return policy, state, writes
end

local names = { "UI_ZMLOBBYONLINE", "UI_ZMLOBBYONLINECUSTOMGAME", "UI_ZMLOBBYONLINEPUBLICGAME", "UI_ZMLOBBYLANGAME" }
for limit = 1, 8 do
  local policy, state = fixture()
  state.limit = limit
  policy.applyTargets()
  for _, name in ipairs(names) do
    local item = LobbyData.UITargets[name]
    assert(item.maxClients == limit and item.maxLocalClients == 2 and item.maxLocalClientsNetwork == 2)
  end
  assert(LobbyData.UITargets.UI_ZMLOBBYONLINETHEATER.maxClients == 1)
  assert(LobbyData.UITargets.UI_ZM_UNKNOWN.maxClients == 4)
  assert(LobbyData.UITargets.UI_MPLOBBY.maxClients == 18)
end
for _, field in ipairs({ "mainMode", "lobbyType", "lobbyMode" }) do
  local policy = fixture()
  LobbyData.UITargets.UI_ZMLOBBYONLINEPUBLICGAME[field] = -1
  policy.applyTargets()
  assert(LobbyData.UITargets.UI_ZMLOBBYONLINEPUBLICGAME.maxClients == 4)
end

-- Frontend migration preserves an observed remote cap, even if the new owner saved four.
local policy, state = fixture()
state.limit, state.capacity = 4, 8
policy.applyTargets()
for _, name in ipairs(names) do
  assert(LobbyData.UITargets[name].maxClients == 8)
end
state.limit = 6
policy.applyTargets()
for _, name in ipairs(names) do
  assert(LobbyData.UITargets[name].maxClients == 8)
end
state.active = false
policy.applyTargets()
for _, name in ipairs(names) do
  assert(LobbyData.UITargets[name].maxClients == 6)
end

policy, state = fixture()
state.limit, state.host, state.capacity = 4, false, 8
assert(policy.targetLimit(LobbyData.UITargets.UI_ZMLOBBYONLINEPUBLICGAME) == 8)
state.host = true
assert(policy.targetLimit(LobbyData.UITargets.UI_ZMLOBBYONLINEPUBLICGAME) == 8)
state.active = false
policy.applyTargets()
assert(LobbyData.UITargets.UI_ZMLOBBYONLINEPUBLICGAME.maxClients == 4)
state.active, state.capacity, state.limit = true, 4, 7
assert(policy.targetLimit(LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME) == 7)

for _, phase in ipairs({ "inGame", "launching", "recovery", "loading" }) do
  policy, state = fixture()
  state.limit, state.capacity = 4, 8
  if phase == "loading" then
    state.map = "zm_zod"
  else
    state[phase] = true
  end
  assert(policy.targetLimit(LobbyData.UITargets.UI_ZMLOBBYONLINEPUBLICGAME) == 8)
  -- A mode-selection query must not inherit the old Zombies GAME cap into MP.
  assert(policy.targetLimit(LobbyData.UITargets.UI_MPLOBBY) == 18)
end
policy, state = fixture()
state.limit, state.capacity, state.host = 4, 18, false
assert(policy.gameLimit(nil, 4) == 18) -- foreign host capacity is preserved, never manufactured

-- Actual party entrypoints retain remote GAME capacity during loading and gameplay.
for _, inGame in ipairs({ false, true }) do
  local writes
  policy, state, writes = fixture()
  state.limit, state.host, state.capacity, state.inGame = 4, false, 8, inGame
  state.map = inGame and "zm_zod" or "core_frontend"
  package.loaded.zombies_coop = nil
  package.preload.zombies_coop = function()
    return policy
  end
  for _, name in ipairs({ "datasources_start_menu_game_options", "zombie_toast", "connection_meter" }) do
    package.loaded[name] = nil
    package.preload[name] = function() end
  end
  dofile(root .. "/data/ui_scripts/party/__init__.lua")
  assert(writes.party_maxplayers == 8)
  assert(writes.com_maxclients == (inGame and 8 or nil))
end

-- The listing control delegates eligibility to the native snapshot and rechecks on click.
local writes
policy, state, writes = fixture()
state.screen, state.playlist, state.capacity = 400, 150, 8
local uiLaunchCount = 0
CoD.LobbyBase = {
  LaunchGame = function(...)
    uiLaunchCount = select("#", ...)
    assert(state.capacity == 4) -- the real client UI surface runs before LobbyVM dispatch
    return "ui launch", nil, "tail"
  end,
}
policy.installSoloLaunchGuard()
local uiLaunch = CoD.LobbyBase.LaunchGame
policy.installSoloLaunchGuard()
assert(CoD.LobbyBase.LaunchGame == uiLaunch)
assert(select("#", uiLaunch({}, 0, 7, "extra", nil)) == 3)
assert(uiLaunchCount == 5 and writes.com_maxclients == 4 and state.limit == 8)

policy, state, writes = fixture()
local list = { { optionDisplay = "STOCK" } }
CoD.LobbyMenus = {
  AddButtonsForTarget = function()
    return list, false, nil, "tail"
  end,
}
policy.installListingButton()
local installed = CoD.LobbyMenus.AddButtonsForTarget
policy.installListingButton()
assert(CoD.LobbyMenus.AddButtonsForTarget == installed)
assert(select("#", installed(0, 402)) == 4 and #list == 1) -- API not ready: hidden
local snapshot = { enabled = true, canAdvertise = true, advertise = false }
game = {
  cooplobbyinfo = function()
    return snapshot
  end,
}
installed(0, 401)
assert(#list == 1) -- not a public-match queue
installed(0, 402)
installed(0, 402)
assert(#list == 2 and list[2].customId == "btnCoopListCustomLobby")
list[2].action()
assert(writes.zm_coopAdvertise == 1 and writes.nat_open == nil and writes.friends_open == nil)
writes.zm_coopAdvertise = nil
snapshot.canAdvertise = false
list[2].action()
assert(writes.zm_coopAdvertise == nil) -- stale eligible button cannot open or list a closed lobby
snapshot.canAdvertise, state.host = true, false
list[2].action()
assert(writes.zm_coopAdvertise == nil)
state.host, snapshot.advertise = true, true
list[2].action()
assert(writes.zm_coopAdvertise == 0)
CoD.LobbyMenus.AddButtonsForTarget = function()
  return "foreign"
end
policy.installListingButton()
assert(CoD.LobbyMenus.AddButtonsForTarget() == "foreign")

local function lobbyFixture()
  local p, s, w = fixture()
  local observed = {}
  Lobby = {
    Actions = {
      LobbyHostStart = function(...)
        observed.args, observed.count = { ... }, select("#", ...)
        return "sentinel", false, nil, "tail"
      end,
    },
  }
  LobbyVM = {
    LaunchGame = function(params, ...)
      observed.launch, observed.launchCount = params, select("#", ...)
      return "launched", false, nil, "tail"
    end,
    CanLobbyCanMigrate = function(...)
      observed.migrate = select("#", ...)
      return "stock", false
    end,
  }
  dofile(root .. "/data/lobby_scripts/zombies_coop/__init__.lua")
  return s, w, observed
end
for limit = 1, 8 do
  for _, lobbyType in ipairs({ 7, 8 }) do
    for _, mode in ipairs({ 3, 4 }) do
      local s, w, observed = lobbyFixture()
      s.limit = limit
      assert(select("#", Lobby.Actions.LobbyHostStart(0, 1, lobbyType, mode, 4, "extra", nil)) == 4)
      assert(observed.args[5] == limit and observed.count == 7 and observed.args[6] == "extra")
      assert(w.party_maxplayers == limit)
      assert(w.com_maxclients == (lobbyType == 7 and limit or nil))
    end
  end
end
for _, arguments in ipairs({
  { 0, 1, 7, 3, 1 },
  { 0, 1, 7, 4, 18 },
  { 0, 2, 7, 3, 18 },
  { 0, 1, 7, 5, 4 },
  { 0, 1 },
  { 0, 1, 7, 4, "4" },
}) do
  local s, w, observed = lobbyFixture()
  Lobby.Actions.LobbyHostStart(unpack(arguments))
  assert(observed.args[5] == arguments[5] and next(w) == nil)
end
state, writes = lobbyFixture()
state.inGame, state.limit = true, 4
Lobby.Actions.LobbyHostStart(0, 1, 7, 4, 8)
assert(next(writes) == nil) -- do not apply a future preference to an active/migrated host
for _, lobbyType in ipairs({ 7, 8 }) do
  for _, capacity in ipairs({ 5, 8 }) do
    local s, w, seen = lobbyFixture()
    s.limit = capacity == 8 and 4 or 8
    Lobby.Actions.LobbyHostStart(0, 1, lobbyType, 4, capacity)
    assert(seen.args[5] == capacity and next(w) == nil) -- frontend inherited/explicit capacity wins
  end
end

-- Stock Solo selects playlist150 on screen400, without another HostStart(1).
local observed
state, writes, observed = lobbyFixture()
state.capacity, state.screen, state.playlist = 8, 400, 150
local params = { controller = 0, lobbyType = 7 }
assert(select("#", LobbyVM.LaunchGame(params, "extra", nil)) == 4)
assert(state.capacity == 4 and writes.com_maxclients == 4 and writes.party_maxplayers == 4)
assert(
  state.limit == 8 and writes.zm_coopPlayerLimit == nil and observed.launch == params and observed.launchCount == 2
)
for _, cap in ipairs({ 1, 2, 3, 4, 18 }) do
  state, writes = lobbyFixture()
  state.capacity, state.screen, state.playlist = cap, 400, 150
  LobbyVM.LaunchGame(params)
  assert(state.capacity == cap and next(writes) == nil)
end
for _, screen in ipairs({ 401, 402, 410 }) do
  state, writes = lobbyFixture()
  state.capacity, state.screen, state.playlist = 8, screen, 150
  LobbyVM.LaunchGame(params)
  assert(state.capacity == 8 and next(writes) == nil)
end

for _, capacity in ipairs({ 5, 6, 7, 8 }) do
  state, writes, observed = lobbyFixture()
  state.capacity, state.inGame = capacity, true
  assert(LobbyVM.CanLobbyCanMigrate({ lobbyType = 7, lobbyMode = 4 }) == false)
  assert(observed.migrate == nil)
  assert(LobbyVM.CanLobbyCanMigrate({ lobbyType = 8, lobbyMode = 4 }) == "stock")
  state.mode = 2
  assert(LobbyVM.CanLobbyCanMigrate({ lobbyType = 7, lobbyMode = 4 }) == "stock")
end
state, writes = lobbyFixture()
state.capacity = 8
assert(LobbyVM.CanLobbyCanMigrate({ lobbyType = 7, lobbyMode = 4 }) == "stock") -- frontend migration retains stock routing
state.capacity, state.inGame = 4, true
assert(LobbyVM.CanLobbyCanMigrate({ lobbyType = 7, lobbyMode = 4 }) == "stock")
local hostStart, launchGame, canMigrate = Lobby.Actions.LobbyHostStart, LobbyVM.LaunchGame, LobbyVM.CanLobbyCanMigrate
dofile(root .. "/data/lobby_scripts/zombies_coop/__init__.lua")
assert(
  Lobby.Actions.LobbyHostStart == hostStart
    and LobbyVM.LaunchGame == launchGame
    and LobbyVM.CanLobbyCanMigrate == canMigrate
)
assert(LobbyVM.ZombiesCoopModuleVersion == 4)
-- A stock LobbyVM reload may replace functions while retaining extension fields.
local freshCalls = 0
Lobby.Actions.LobbyHostStart = function(...)
  freshCalls = freshCalls + 1
  return "fresh host"
end
LobbyVM.LaunchGame = function(...)
  freshCalls = freshCalls + 1
  return "fresh launch"
end
LobbyVM.CanLobbyCanMigrate = function(...)
  freshCalls = freshCalls + 1
  return "fresh migrate"
end
dofile(root .. "/data/lobby_scripts/zombies_coop/__init__.lua")
local refreshed = Lobby.Actions.LobbyHostStart
assert(refreshed ~= hostStart and LobbyVM.LaunchGame ~= launchGame and LobbyVM.CanLobbyCanMigrate ~= canMigrate)
state.inGame, state.capacity = false, 4
assert(Lobby.Actions.LobbyHostStart(0, 2, 7, 3, 18) == "fresh host")
assert(LobbyVM.LaunchGame({ lobbyType = 8 }) == "fresh launch")
assert(LobbyVM.CanLobbyCanMigrate({ lobbyType = 8 }) == "fresh migrate")
assert(freshCalls == 3 and LobbyVM.ZombiesCoopModuleVersion == 4)
dofile(root .. "/data/lobby_scripts/zombies_coop/__init__.lua")
assert(Lobby.Actions.LobbyHostStart == refreshed)
print(
  "PASS: stock online/custom/public/LAN contracts, frontend ownership, launch/loading, Solo150, listing eligibility and migration refusal"
)
dofile(root .. "/tests/zombies_coop_dedicated_host.lua")
dofile(root .. "/tests/zombies_coop_settings.lua")
