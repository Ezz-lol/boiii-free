local root = assert(arg[1], "pass the checkout directory")

for _, order in ipairs({ "listing-first", "settings-first" }) do
  BoiiiZombiesCoopPolicy = nil
  local capacity, originalLaunchCalls, originalButtonCalls = 8, 0, 0
  Enum = {
    eModes = { MODE_ZOMBIES = 1 },
    LobbyType = { LOBBY_TYPE_GAME = 7 },
    LobbyMainMode = { LOBBY_MAINMODE_ZM = 1 },
    LobbyMode = { LOBBY_MODE_CUSTOM = 3, LOBBY_MODE_PUBLIC = 4 },
  }
  Engine = {
    CurrentSessionMode = function()
      return 1
    end,
    GetCurrentMap = function()
      return "core_frontend"
    end,
    IsInGame = function()
      return false
    end,
    IsLobbyActive = function()
      return true
    end,
    IsLobbyHost = function()
      return true
    end,
    LobbyHostLaunchTaskInProgress = function()
      return false
    end,
    GetLobbyUIScreen = function()
      return 400
    end,
    GetPlaylistID = function()
      return 150
    end,
    GetLobbyMaxClients = function()
      return capacity
    end,
    SetLobbyMaxClients = function(_, value)
      capacity = value
    end,
    SetDvar = function() end,
  }
  local function target(id, mode)
    return { id = id, mainMode = 1, lobbyType = 7, lobbyMode = mode, maxClients = 4 }
  end
  LobbyData = {
    UITargets = {
      UI_ZMLOBBYONLINE = target(400, 3),
      UI_ZMLOBBYONLINECUSTOMGAME = target(402, 3),
    },
  }
  game = {
    cooplobbyinfo = function()
      return { enabled = true, canAdvertise = true, advertise = false }
    end,
  }
  CoD = {
    Menu = {},
    OptionsUtility = { CreateDvarSettings = function() end },
    LobbyBase = {
      LaunchGame = function()
        originalLaunchCalls = originalLaunchCalls + 1
        assert(capacity == 4)
        return "original launch", nil, "tail"
      end,
    },
    LobbyMenus = {
      AddButtonsForTarget = function()
        originalButtonCalls = originalButtonCalls + 1
        return { { customId = "stock" } }, false, nil, "tail"
      end,
    },
  }
  LUI, DataSources = { createMenu = {} }, {}
  DataSourceHelpers = {
    ListSetup = function()
      return {}
    end,
  }
  local policy = dofile(root .. "/data/ui_scripts/party/zombies_coop.lua")
  local settings = dofile(root .. "/data/ui_scripts/party/zombies_coop_settings.lua")
  local function installButtons()
    if order == "listing-first" then
      policy.installListingButton()
      assert(settings.install())
    else
      assert(settings.install())
      policy.installListingButton()
    end
  end
  local function verifyButtons()
    local buttons, flag, empty, tail = CoD.LobbyMenus.AddButtonsForTarget(0, 402)
    assert(#buttons == 3 and flag == false and empty == nil and tail == "tail")
    local counts = {}
    for _, button in ipairs(buttons) do
      counts[button.customId] = (counts[button.customId] or 0) + 1
    end
    assert(counts.stock == 1 and counts.btnCoopListCustomLobby == 1 and counts.btnCoopLobbySettings == 1)
    assert(select("#", CoD.LobbyMenus.AddButtonsForTarget(0, 402)) == 4)
  end
  policy.installSoloLaunchGuard()
  assert(select("#", CoD.LobbyBase.LaunchGame({}, 0, 7)) == 3)
  assert(originalLaunchCalls == 1)
  installButtons()
  verifyButtons()
  assert(originalButtonCalls == 2)

  for _ = 1, 3 do
    local previousLaunch, previousButtons = CoD.LobbyBase.LaunchGame, CoD.LobbyMenus.AddButtonsForTarget
    capacity = 8
    local freshLaunchCalls, freshButtonCalls = 0, 0
    -- A primary UI reload can replace stock functions while retaining our markers.
    CoD.LobbyBase.LaunchGame = function()
      freshLaunchCalls = freshLaunchCalls + 1
      assert(capacity == 4)
      return "fresh launch", nil, "tail"
    end
    CoD.LobbyMenus.AddButtonsForTarget = function()
      freshButtonCalls = freshButtonCalls + 1
      return { { customId = "stock" } }, false, nil, "tail"
    end
    policy.installSoloLaunchGuard()
    installButtons()
    assert(CoD.LobbyBase.LaunchGame ~= previousLaunch and CoD.LobbyMenus.AddButtonsForTarget ~= previousButtons)
    local installedLaunch, installedButtons = CoD.LobbyBase.LaunchGame, CoD.LobbyMenus.AddButtonsForTarget
    for attempt = 1, 3 do
      policy.installSoloLaunchGuard()
      policy.installListingButton()
      settings.install()
      assert(CoD.LobbyBase.LaunchGame == installedLaunch and CoD.LobbyMenus.AddButtonsForTarget == installedButtons)
    end
    assert(select("#", CoD.LobbyBase.LaunchGame({}, 0, 7)) == 3)
    verifyButtons()
    assert(freshLaunchCalls == 1 and freshButtonCalls == 2)
    assert(originalLaunchCalls == 1 and originalButtonCalls == 2)
  end
end
print(
  "PASS: primary UI function ownership, Solo reinstall, fresh listing/settings chains in either order, one delegation and stable repeated install"
)
