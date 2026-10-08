local function split_string(str, delimiter)
  local tokens = {}
  local pattern = string.format("([^%s]+)", delimiter)
  for token in string.gmatch(str, pattern) do
    table.insert(tokens, token)
  end
  return tokens
end

Lobby.Process.CreateDedicatedModsLobby = function(controller, toTarget)
  local lobby_mode = Engine.DvarString(nil, "sv_lobby_mode")
  if lobby_mode ~= "" then
    if lobby_mode == "zm" then
      toTarget = LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME
    elseif lobby_mode == "cp" then
      toTarget = LobbyData.UITargets.UI_CPLOBBYONLINECUSTOMGAME
    elseif lobby_mode == "cpzm" then
      toTarget = LobbyData.UITargets.UI_CP2LOBBYONLINECUSTOMGAME
    elseif lobby_mode == "fr" then
      toTarget = LobbyData.UITargets.UI_FRLOBBYONLINEGAME
      toTarget.maxClients = Engine.DvarInt(nil, "com_maxclients")
    elseif lobby_mode == "doa" then
      toTarget = LobbyData.UITargets.UI_DOALOBBYONLINE
    elseif lobby_mode == "mp" then
      toTarget = LobbyData.UITargets.UI_MPLOBBYONLINEMODGAME
    elseif lobby_mode == "arena" then
      toTarget = LobbyData.UITargets.UI_MPLOBBYONLINEARENAGAME
    end
  end
  local playlistID = Dvar.sv_playlist
  Engine.SetPlaylistID(playlistID:get())
  local lobbyInit = Lobby.Actions.ExecuteScript(function()
    Lobby.ProcessNavigate.SetupLobbyMapAndGameType(controller, toTarget)
  end)
  local setNetworkMode = Lobby.Actions.SetNetworkMode(controller, Enum.LobbyNetworkMode.LOBBY_NETWORKMODE_LIVE)
  local lobbySettings = Lobby.Actions.LobbySettings(controller, toTarget)
  local updateUI = Lobby.Actions.UpdateUI(controller, toTarget)
  local createGameHost = Lobby.Actions.LobbyHostStart(
    controller,
    toTarget.mainMode,
    toTarget.lobbyType,
    toTarget.lobbyMode,
    toTarget.maxClients
  )
  local lobbyAdvertise = Lobby.Actions.AdvertiseLobby(true)
  local hostingEvent = Lobby.Actions.ExecuteScript(function()
    Engine.QoSProbeListenerEnable(toTarget.lobbyType, true)
    Engine.SetDvar("live_dedicatedReady", 1)
    Engine.RunPlaylistRules(controller)
    Engine.RunPlaylistSettings(controller)
    Lobby.Timer.HostingLobby({
      controller = controller,
      lobbyType = toTarget.lobbyType,
      mainMode = toTarget.mainMode,
      lobbyTimerType = toTarget.lobbyTimerType,
    })
    if Engine.DvarInt(nil, "sv_skip_lobby") == 1 then
      local map_rotation_string = Engine.DvarString(nil, "sv_maprotation")
      if map_rotation_string ~= "" then
        local map_rotation_tokens = split_string(map_rotation_string, " ")
        if map_rotation_tokens[1] == "gametype" then
          Engine.Exec(0, "lobby_setgametype " .. map_rotation_tokens[2])
          if map_rotation_tokens[3] == "map" then
            Engine.Exec(0, "lobby_setmap " .. map_rotation_tokens[4])
          end
        end
      end
      Engine.Exec(0, "launchgame")
    end
  end)
  Lobby.Process.AddActions(setNetworkMode, lobbySettings)
  Lobby.Process.AddActions(lobbySettings, lobbyInit)
  Lobby.Process.AddActions(lobbyInit, updateUI)
  Lobby.Process.AddActions(updateUI, createGameHost)
  Lobby.Process.AddActions(createGameHost, lobbyAdvertise)
  Lobby.Process.AddActions(lobbyAdvertise, hostingEvent)
  Lobby.Process.AddActions(hostingEvent, nil)
  return { head = setNetworkMode, interrupt = Lobby.Interrupt.NONE, force = true, cancellable = true }
end
