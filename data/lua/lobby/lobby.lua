require("lua.Lobby.Common.LobbyCore")
require("lua.Lobby.Process.LobbyProcessNavigate")
require("lua.Lobby.Process.LobbyProcess")
require("lua.Lobby.Process.LobbyProcessQueue")
require("lua.Shared.LuaUtils")
require("lua.Lobby.Leaderboard")
require("lua.Lobby.LobbyAnalytics")
require("lua.Lobby.LobbyAnticheat")
require("lua.Lobby.LobbyDebug")
require("lua.Lobby.LobbyHopper")
require("lua.Lobby.LobbyMapVote")
require("lua.Lobby.LobbyMerge")
require("lua.Lobby.LobbyPartyPrivacy")
require("lua.Lobby.LobbyPlatform")
require("lua.Lobby.LobbyStats")
require("lua.Lobby.LobbySteam")
require("lua.Lobby.LobbyTeamSelection")
require("lua.Lobby.LobbyTheater")
require("lua.Lobby.LobbyTimer")
require("lua.Lobby.LobbyEvents")
require("lua.Lobby.UserGeneratedContent")

Util = {
  IsNil = function(value)
    if value == nil then
      return "true"
    else
      return "false"
    end
  end,
}
LobbyVM = {
  AutoJoin = {},
  DevGui = {},
  lobbyStatus = {
    cleared = true,
    clearedTime = 0,
  },
  playSoundHistory = {},
  hostLaunch = {},
  clientLaunch = {},
  lobbyMonitor = {},
}
LobbyVM.lobbyMonitor.eventRecorded = false
LobbyVM.DevGui.LaunchGame = function(mapName, gameType)
  if mapName ~= nil then
    Dvar.ui_mapname:set(mapName)
  end
  if gameType ~= nil then
    Dvar.ui_gametype:set(gameType)
  end
  if not Engine.IsLobbyActive(Enum.LobbyType.LOBBY_TYPE_GAME) then
    return
  elseif not Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_GAME) then
    return
  else
    Lobby.Timer.Clear(true)
    LobbyVM.LaunchGameExec(Engine.GetPrimaryController(), Enum.LobbyType.LOBBY_TYPE_GAME)
  end
end

LobbyVM.DevGui.CustomGameAdvertise = function()
  if not Engine.IsLobbyActive(Enum.LobbyType.LOBBY_TYPE_GAME) then
    return
  elseif not Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_GAME) then
    return
  elseif Engine.GetLobbyUIScreen() ~= LobbyData.UITargets.UI_MPLOBBYONLINECUSTOMGAME.id then
    return
  end
  local shouldAdvertise = false
  local advertiseState = "off"
  if not Engine.IsAdvertisedLobby(Enum.LobbyType.LOBBY_TYPE_GAME) then
    shouldAdvertise = true
    advertiseState = "on"
    Lobby.Matchmaking.SetupAdvertisingCustomMatch()
  end
  Engine.PrintInfo(Enum.consoleLabel.LABEL_LOBBYHOST, "Turning session advertisement " .. advertiseState .. ".\n")
  Lobby.Matchmaking.SetupMatchmakingQuery(Engine.GetPrimaryController(), Lobby.Matchmaking.SearchMode.PUBLIC)
  Engine.AdvertiseLobby(Lobby.ProcessQueue.INVALID_ACTION_ID, Enum.LobbyType.LOBBY_TYPE_GAME, shouldAdvertise)
end

LobbyVM.DevGui.LobbyReset = function()
  LobbyVM.ErrorShutdown({
    controller = Engine.GetPrimaryController(),
    signoutUsers = false,
  })
end

LobbyVM.PlaySound = function(soundName, cooldownMs)
  local shouldPlay = false
  if LobbyVM.playSoundHistory[soundName] == nil then
    shouldPlay = true
  elseif LobbyVM.playSoundHistory[soundName] < Engine.milliseconds() then
    shouldPlay = true
  end
  if shouldPlay == true then
    Engine.PlaySound(soundName)
    LobbyVM.playSoundHistory[soundName] = Engine.milliseconds() + cooldownMs
  end
end

LobbyVM.ExecuteLobbyVMRequest = function(request)
  local requestModel = Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.lobbyVMRequest")
  if Engine.GetModelValue(requestModel) == request then
    Engine.ForceNotifyModelSubscriptions(requestModel)
  else
    Engine.SetModelValue(requestModel, request)
  end
end

LobbyVM.ErrorShutdown = function(shutdownParams)
  Engine.PrintInfo(Enum.consoleLabel.LABEL_LOBBY, "Performing emergency shutdown.\n")
  Engine.PrintInfo(Enum.consoleLabel.LABEL_LOBBY, "Close all popups in the lobby.\n")
  Engine.ForceNotifyModelSubscriptions(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.closePopups"))
  Engine.PrintInfo(Enum.consoleLabel.LABEL_LOBBY, "Shutting down joins.\n")
  Engine.LobbyJoinErrorShutdown()
  if Engine.LobbyClientTaskErrorShutdown then
    Engine.LobbyClientTaskErrorShutdown()
  end
  if Engine.LobbyHostTaskErrorShutdown then
    Engine.LobbyHostTaskErrorShutdown()
  end
  if Engine.GetLobbyNetworkMode() == Enum.LobbyNetworkMode.LOBBY_NETWORKMODE_LIVE then
    Engine.PrintInfo(Enum.consoleLabel.LABEL_LOBBY, "Shutting down qos probes.\n")
    Engine.QoSErrorShutdown(Enum.LobbyType.LOBBY_TYPE_GAME)
    Engine.PrintInfo(Enum.consoleLabel.LABEL_LOBBY, "Shutting down advertisement.\n")
    Engine.AdvertiseErrorShutdown(Enum.LobbyType.LOBBY_TYPE_GAME)
    Engine.PrintInfo(Enum.consoleLabel.LABEL_LOBBY, "Shutting down search.\n")
    Engine.LobbySearchErrorShutdown()
  end
  Engine.PrintInfo(Enum.consoleLabel.LABEL_LOBBY, "Shutting down private lobby.\n")
  Engine.LobbyClientErrorShutdown(Enum.LobbyType.LOBBY_TYPE_GAME)
  Engine.LobbyHostErrorShutdown(Enum.LobbyType.LOBBY_TYPE_GAME)
  Engine.PrintInfo(Enum.consoleLabel.LABEL_LOBBY, "Shutting down game lobby.\n")
  Engine.LobbyClientErrorShutdown(Enum.LobbyType.LOBBY_TYPE_PRIVATE)
  Engine.LobbyHostErrorShutdown(Enum.LobbyType.LOBBY_TYPE_PRIVATE)
  Engine.PrintInfo(Enum.consoleLabel.LABEL_LOBBY, "Clearing process queue.\n")
  Lobby.ProcessQueue.ErrorShutdown()
  if shutdownParams.signoutUsers ~= nil and shutdownParams.signoutUsers == true then
    Engine.SignOutAllUsers()
  end
  Engine.PrintInfo(Enum.consoleLabel.LABEL_LOBBY, "Forcing UI screen.\n")
  Engine.ForceLobbyUIScreen(LobbyData.UITargets.UI_MAIN.id)
  Engine.LuiVM_Event("open_main", {})
  Engine.PrintInfo(Enum.consoleLabel.LABEL_LOBBY, "Lobby emergency shutdown complete.\n")
end

LobbyVM.ErrorShutdownMessage = function(controller, message)
  LobbyVM.ErrorShutdown({
    controller = controller,
    signoutUsers = false,
  })
  LuaUtils.UI_ShowErrorMessageDialog(controller, message)
end

LobbyVM.LogGlobalData = function()
  local globalData = {
    playlistVersion = Engine.GetPlaylistVersionNumber(),
    protocolVersion = Engine.GetProtocolVersion(),
  }
  if Engine.GetDDLVersion ~= nil then
    globalData.ffotdVersion = Engine.GetFFOTDVersion()
    globalData.mpStatsDDLVersion = Engine.GetDDLVersion("gamedata/ddl/mp/mp_stats.ddl")
    globalData.mpLoadoutsDDLVersion = Engine.GetDDLVersion("gamedata/ddl/loadouts/mp_loadouts.ddl")
    globalData.buildCL = Engine.GetBuildIntField(Enum.BuildIntField.BUILD_INTFIELD_CHANGELIST)
    globalData.buildMachine = Engine.GetBuildStringField(Enum.BuildStringField.BUILD_STRINGFIELD_BUILD_MACHINE)
    globalData.buildTime = Engine.GetBuildStringField(Enum.BuildStringField.BUILD_STRINGFIELD_BUILD_TIME)
    globalData.buildType = Engine.GetBuildStringField(Enum.BuildStringField.BUILD_STRINGFIELD_BUILD_TYPE)
    globalData.buildName = Engine.GetBuildStringField(Enum.BuildStringField.BUILD_STRINGFIELD_BUILD_NAME)
  end
  LuaUtils.LogQoS("global_data", globalData)
end

LobbyVM.OnInit = function(controller)
  Lobby.ProcessQueue.Init()
  Lobby.TeamSelection.Init()
  LobbyData.InitLobbyNav()
  Lobby.Platform.OnInit(controller)
  Lobby.Debug.OnInit(controller)
  Lobby.Anticheat.OnInit()
end

LobbyVM.OnUILoad = function(uiLoadData)
  local isInit = uiLoadData.init
  Lobby.Debug.OnUILoad(uiLoadData)
  if isInit then
    Lobby.MapVote.Init()
  end
end

LobbyVM.OnSessionStart = function(sessionData)
  local lobbyModule = sessionData.lobbyModule
  local lobbyType = sessionData.lobbyType
  local lobbyMode = sessionData.lobbyMode
  Lobby.MapVote.OnSessionStart(sessionData)
  Lobby.PartyPrivacy.OnSessionStart(sessionData)
  Lobby.Theater.OnSessionStart(sessionData)
  Lobby.Timer.OnSessionStart(sessionData)
  Lobby.TeamSelection.OnSessionStart(sessionData)
  Lobby.Platform.OnSessionStart(sessionData)
  Lobby.Debug.OnSessionStart(sessionData)
  Lobby.Steam.OnSessionStart(sessionData)
end

LobbyVM.OnSessionEnd = function(sessionData)
  local lobbyModule = sessionData.lobbyModule
  local lobbyType = sessionData.lobbyType
  local lobbyMode = sessionData.lobbyMode
  Lobby.Hopper.OnSessionEnd(sessionData)
  Lobby.TeamSelection.OnSessionEnd(sessionData)
  Lobby.Theater.OnSessionEnd(sessionData)
  Lobby.Timer.OnSessionEnd(sessionData)
  Lobby.MapVote.OnSessionEnd(sessionData)
  Lobby.Platform.OnSessionEnd(sessionData)
  Lobby.Debug.OnSessionEnd(sessionData)
  Lobby.Steam.OnSessionEnd(sessionData)
end

LobbyVM.OnClientAdded = function(clientData)
  local lobbyModule = clientData.lobbyModule
  local lobbyType = clientData.lobbyType
  local lobbyMode = clientData.lobbyMode
  local xuid = clientData.xuid
  if lobbyModule == Enum.LobbyModule.LOBBY_MODULE_HOST then
    local isHost = Engine.GetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.gameClient.isHost"))
    if isHost and isHost ~= 0 then
      LuaUtils.ForceLobbyButtonUpdate()
    end
  end
  Lobby.Matchmaking.OnClientAdded(clientData)
  Lobby.TeamSelection.OnClientAdded(clientData)
  Lobby.Pregame.OnClientAdded(clientData)
  Lobby.Leaderboard.OnClientAdded(clientData)
  Lobby.Steam.OnClientAdded(clientData)
  Engine.QoSProbeListenerUpdate(lobbyType)
  if lobbyModule == Enum.LobbyModule.LOBBY_MODULE_CLIENT then
    Engine.AddLobbyClientExtracam(clientData.xuid)
  end
  if
      lobbyModule == Enum.LobbyModule.LOBBY_MODULE_CLIENT
      and not Engine.IsLocalClient(xuid)
      and not Engine.IsInGame()
  then
    LobbyVM.PlaySound(LobbyData.Sounds.ClientsAddedToLobby, 500)
  end
  if
      LuaUtils.isPS4 == true
      and Enum.LobbyNetworkMode.LOBBY_NETWORKMODE_LIVE == Engine.GetLobbyNetworkMode()
      and lobbyModule == Enum.LobbyModule.LOBBY_MODULE_CLIENT
      and lobbyType == Enum.LobbyType.LOBBY_TYPE_PRIVATE
      and Engine.IsLocalClient(xuid)
      and not Engine.IsInGame()
  then
    local controller = Engine.GetControllerForXuid(xuid)
    if Engine.NotifyPsPlusAsyncMultiplay ~= nil then
      Engine.NotifyPsPlusAsyncMultiplay(controller)
    end
  end
  clientData.lobbyID = Engine.GetLobbyLobbyID(lobbyType)
  LuaUtils.LogQoS("client_added", clientData)
end

LobbyVM.OnClientRemoved = function(clientData)
  local lobbyModule = clientData.lobbyModule
  local lobbyType = clientData.lobbyType
  local lobbyMode = clientData.lobbyMode
  local xuid = clientData.xuid
  if lobbyModule == Enum.LobbyModule.LOBBY_MODULE_HOST then
    LuaUtils.ForceLobbyButtonUpdate()
  end
  Lobby.Timer.OnClientRemoved(clientData)
  Lobby.Pregame.OnClientRemoved(clientData)
  Lobby.Steam.OnClientRemoved(clientData)
  Lobby.Matchmaking.OnClientRemoved(clientData)
  Engine.QoSProbeListenerUpdate(lobbyType)
  if lobbyModule == Enum.LobbyModule.LOBBY_MODULE_CLIENT then
    Engine.RemoveLobbyClientExtracam(clientData.xuid)
  end
  if
      lobbyModule == Enum.LobbyModule.LOBBY_MODULE_CLIENT
      and not Engine.IsLocalClient(xuid)
      and not Engine.IsInGame()
  then
    LobbyVM.PlaySound(LobbyData.Sounds.ClientsRemovedFromLobby, 500)
  end
  clientData.lobbyID = Engine.GetLobbyLobbyID(lobbyType)
  LuaUtils.LogQoS("client_removed", clientData)
end

LobbyVM.OnMatchStart = function(matchData)
  local lobbyModule = matchData.lobbyModule
  local lobbyType = matchData.lobbyType
  local lobbyMode = matchData.lobbyMode
  Lobby.Timer.OnMatchStart(matchData)
  Lobby.Stats.OnMatchStart(matchData)
  Lobby.Matchmaking.OnMatchStart(matchData)
  Lobby.Platform.OnMatchStart(matchData)
  Lobby.Debug.OnMatchStart(matchData)
  Lobby.Steam.OnMatchStart(matchData)
  Engine.AllowVoiceChatForAll(Enum.LobbyType.LOBBY_TYPE_GAME)
  LuaUtils.CycleContracts()
  LuaUtils.ValidateBlackjackPurchase()
  LuaUtils.UI_ClearErrorMessageDialog()
end

LobbyVM.OnMatchLaunchClient = function(matchData)
  local lobbyModule = matchData.lobbyModule
  local lobbyType = matchData.lobbyType
  local lobbyMode = matchData.lobbyMode
  Lobby.Stats.OnMatchLaunchClient(matchData)
  LuaUtils.LogQoS("match_launch_client", matchData)
end

LobbyVM.OnMatchChangeGameType = function(matchData)
  Lobby.Steam.OnMatchChangeGameType(matchData)
end

LobbyVM.OnMatchChangeMap = function(matchData)
  local lobbyModule = matchData.lobbyModule
  local lobbyType = matchData.lobbyType
  local lobbyMode = matchData.lobbyMode
  Lobby.Stats.OnMatchChangeMap(matchData)
  Lobby.Matchmaking.OnMatchChangeMap(matchData)
  Lobby.Steam.OnMatchChangeMap(matchData)
  LuaUtils.LogQoS("match_change_map", matchData)
end

LobbyVM.OnMatchEnd = function(matchData)
  local lobbyModule = matchData.lobbyModule
  local lobbyType = matchData.lobbyType
  local lobbyMode = matchData.lobbyMode
  local currentScreen = Engine.GetLobbyUIScreen()
  if currentScreen == LobbyData.UITargets.UI_CPLOBBYLANCUSTOMGAME.id then
    LobbyVM.UpdateUI({
      toTarget = LobbyData.UITargets.UI_CPLOBBYLANGAME,
    })
  elseif currentScreen == LobbyData.UITargets.UI_CPLOBBYONLINECUSTOMGAME.id then
    LobbyVM.UpdateUI({
      toTarget = LobbyData.UITargets.UI_CPLOBBYONLINE,
    })
  elseif currentScreen == LobbyData.UITargets.UI_CPLOBBYONLINEPUBLICGAME.id then
    LobbyVM.UpdateUI({
      toTarget = LobbyData.UITargets.UI_CPLOBBYONLINE,
    })
  elseif currentScreen == LobbyData.UITargets.UI_CP2LOBBYLANCUSTOMGAME.id then
    LobbyVM.UpdateUI({
      toTarget = LobbyData.UITargets.UI_CP2LOBBYLANGAME,
    })
  elseif currentScreen == LobbyData.UITargets.UI_CP2LOBBYONLINECUSTOMGAME.id then
    LobbyVM.UpdateUI({
      toTarget = LobbyData.UITargets.UI_CP2LOBBYONLINE,
    })
  elseif currentScreen == LobbyData.UITargets.UI_CP2LOBBYONLINEPUBLICGAME.id then
    LobbyVM.UpdateUI({
      toTarget = LobbyData.UITargets.UI_CP2LOBBYONLINE,
    })
  elseif currentScreen == LobbyData.UITargets.UI_DOALOBBYLANCUSTOMGAME.id then
    LobbyVM.UpdateUI({
      toTarget = LobbyData.UITargets.UI_DOALOBBYLANGAME,
    })
  elseif currentScreen == LobbyData.UITargets.UI_DOALOBBYONLINEPUBLICGAME.id then
    LobbyVM.UpdateUI({
      toTarget = LobbyData.UITargets.UI_DOALOBBYONLINE,
    })
  end
  Lobby.TeamSelection.OnMatchEnd(matchData)
  Lobby.Timer.OnMatchEnd(matchData)
  Lobby.Analytics.OnMatchEnd(matchData)
  Lobby.Matchmaking.OnMatchEnd(matchData)
  Lobby.Platform.OnMatchEnd(matchData)
  Lobby.Debug.OnMatchEnd(matchData)
  Lobby.Steam.OnMatchEnd(matchData)
  LuaUtils.BuyWeaponContractIfNeededAfterMatch()
  LuaUtils.BuySpecialContractRewardsIfNeededAfterMatch()
  LuaUtils.SaveAfterMatchContractStats()
  LuaUtils.CycleContracts()
  LuaUtils.ResetToLastSelectedSpecialistIfNeededAfterMatch()
end

LobbyVM.OnMatchRecordStart = function(matchData)
  LuaUtils.LogQoS("match_record", matchData)
end

LobbyVM.OnLobbyServerListRetrieved = function(serverListData)
  Lobby.Steam.OnLobbyServerListRetrieved(serverListData)
end

LobbyVM.OnLobbyServerCountUpdated = function(serverCountData)
  Lobby.Steam.OnLobbyServerCountUpdated(serverCountData)
end

LobbyVM.OnLobbyServerListSorted = function(serverListData)
  Lobby.Steam.OnLobbyServerListSorted(serverListData)
end

LobbyVM.ShouldShowContentChangedMessage = function(controller, lobbyType)
  if LobbyVM.CheckDLCBit(Engine.GetDLCBits(), Engine.GetDLCBitForMapName(Dvar.ui_mapname:get())) then
    return true
  else
    return false
  end
end

LobbyVM.OnDisconnect = function(disconnectData)
  local lobbyModule = disconnectData.lobbyModule
  local lobbyType = disconnectData.lobbyType
  local lobbyMode = disconnectData.lobbyMode
  local disconnectClientXuid = disconnectData.disconnectClientXuid
  local disconnectClient = disconnectData.disconnectClient
  local controller = Engine.GetPrimaryController()
  local showAsError = true
  local title = nil
  local message = disconnectData.errorMsg
  if not message then
    message = Engine.Localize("EXE_DISCONNECTED_FROM_SERVER")
  end
  local recoverProcess = Lobby.Process.Recover(controller)
  if disconnectClient == Enum.LobbyDisconnectClient.LOBBY_DISCONNECT_CLIENT_INVALID then
  elseif disconnectClient == Enum.LobbyDisconnectClient.LOBBY_DISCONNECT_CLIENT_DROP then
  elseif disconnectClient == Enum.LobbyDisconnectClient.LOBBY_DISCONNECT_CLIENT_NOPARTYCHAT then
    showAsError = false
    title = Engine.Localize("MENU_NOTICE")
    message = Engine.Localize("PLATFORM_PARTYCHATNOTALLOWED")
    Dvar.partyChatDisallowed:set(false)
  elseif disconnectClient == Enum.LobbyDisconnectClient.LOBBY_DISCONNECT_CLIENT_KICK then
    showAsError = false
    title = Engine.Localize("EXE_KICKED_FROM_PARTY_WARNING_TITLE")
    message = Engine.Localize("EXE_KICKED_FROM_PARTY_WARNING")
  elseif disconnectClient == Enum.LobbyDisconnectClient.LOBBY_DISCONNECT_CLIENT_MISSING_CONTENT then
    Engine.ComError(Enum.errorCode.ERROR_DROP, message)
  elseif disconnectClient == Enum.LobbyDisconnectClient.LOBBY_DISCONNECT_CLIENT_BADDLC then
    if LobbyVM.ShouldShowContentChangedMessage(controller, lobbyType) then
      Engine.ComError(Enum.errorCode.ERROR_DROP, "MENU_NEW_DLC_AVAILABLE")
    else
      Engine.ComError(Enum.errorCode.ERROR_DROP, "PLATFORM_MISSINGMAP")
    end
  elseif disconnectClient == Enum.LobbyDisconnectClient.LOBBY_DISCONNECT_CLIENT_HOSTRELOAD then
    if
        Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_PRIVATE)
        and not Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_GAME)
    then
      Lobby.ProcessQueue.AddToQueue(
        "ReloadGameLobby",
        Lobby.ProcessNavigate.ReloadGameLobby(
          controller,
          math.random(
            Engine.DvarInt(0, "arena_lobbyReloadSearchDelayMin"),
            Engine.DvarInt(0, "arena_lobbyReloadSearchDelayMax")
          )
        )
      )
    end
    return
  elseif disconnectClient == Enum.LobbyDisconnectClient.LOBBY_DISCONNECT_CLIENT_ERROR then
    message = Engine.Localize("EXE_DISCONNECTED_FROM_SERVER_ERROR")
  end
  if showAsError == true then
    recoverProcess.tail.success = Lobby.Actions.ErrorPopupMsg(message, title)
  else
    recoverProcess.tail.success = Lobby.Actions.WarningPopupMsg(message, title)
  end
  Lobby.ProcessQueue.AddToQueue("DisconnectFromHost", recoverProcess)
end

LobbyVM.OnLobbyOnlineUpdate = function(updateData)
  Lobby.Matchmaking.OnLobbyOnlineUpdate(updateData)
end

LobbyVM.OnLeaveWithParty = function(leaveData)
  local lobbyModule = leaveData.lobbyModule
  local lobbyType = leaveData.lobbyType
  local lobbyMode = leaveData.lobbyMode
  LobbyVM.LobbyLeaveWithParty(leaveData)
end

LobbyVM.OnGametypeSettingsChange = function(settingsData)
  local lobbyModule = settingsData.lobbyModule
  local lobbyType = settingsData.lobbyType
  local lobbyMode = settingsData.lobbyMode
  Lobby.TeamSelection.OnGametypeSettingsChange(settingsData)
  LuaUtils.ForceLobbyButtonUpdate()
end

LobbyVM.OnPartyPrivacyChange = function(privacyData)
  local lobbyType = privacyData.lobbyType
  local partyPrivacy = privacyData.partyPrivacy
  Lobby.Steam.OnPartyPrivacyChange(privacyData)
end

LobbyVM.OnSessionModeChanged = function(sessionModeData)
  Lobby.Steam.OnSessionModeChanged(sessionModeData)
end

LobbyVM.OnMaxClientsChanged = function(maxClientsData)
  Lobby.Steam.OnMaxClientsChanged(maxClientsData)
end

LobbyVM.OnPlatformSessionDataUpdate = function(sessionData)
  Lobby.Platform.OnPlatformSessionDataUpdate(sessionData)
end

LobbyVM.ComErrorCodeToString = function(errorCode)
  if errorCode == Enum.errorCode.ERROR_NONE then
    return "ERR_NONE"
  elseif errorCode == Enum.errorCode.ERROR_FATAL then
    return "ERR_FATAL"
  elseif errorCode == Enum.errorCode.ERROR_DROP then
    return "ERR_DROP"
  elseif errorCode == Enum.errorCode.ERROR_FROM_STARTUP then
    return "ERR_FROM_STARTUP"
  elseif errorCode == Enum.errorCode.ERROR_SERVERDISCONNECT then
    return "ERR_SERVERDISCONNECT"
  elseif errorCode == Enum.errorCode.ERROR_DISCONNECT then
    return "ERR_DISCONNECT"
  elseif errorCode == Enum.errorCode.ERROR_SCRIPT then
    return "ERR_SCRIPT"
  elseif errorCode == Enum.errorCode.ERROR_SCRIPT_DROP then
    return "ERR_SCRIPT_DROP"
  elseif errorCode == Enum.errorCode.ERROR_LOCALIZATION then
    return "ERR_LOCALIZATION"
  elseif errorCode == Enum.errorCode.ERROR_UI then
    return "ERR_UI"
  elseif errorCode == Enum.errorCode.ERROR_LUA then
    return "ERR_LUA"
  elseif errorCode == Enum.errorCode.ERROR_SOFTRESTART then
    return "ERR_SOFTRESTART"
  elseif errorCode == Enum.errorCode.ERROR_SOFTRESTART_KEEPDW then
    return "ERR_SOFTRESTART_KEEPDW"
  elseif errorCode == Enum.errorCode.ERROR_SOFTRESTART_SILENT then
    return "ERR_SOFTRESTART_SILENT"
  else
    return tostring(errorCode)
  end
end

LobbyVM.OnComError = function(comErrorData)
  Lobby.Platform.OnComError(comErrorData)
  if comErrorData.isInCleanup then
    LobbyVM.OnComErrorCleanup(comErrorData)
    return
  end
  local controller = comErrorData.controller
  local errorCode = comErrorData.errorCode
  local errorMsg = comErrorData.errorMsg
  local signoutUsers = comErrorData.signoutUsers
  local comErrorInProgress = comErrorData.comErrorInProgress
  Lobby.Debug.OnComError(comErrorData)
  Engine.PrintError(
    Enum.consoleLabel.LABEL_LOBBYVM,
    "COM ERROR - errorCode: " .. LobbyVM.ComErrorCodeToString(errorCode) .. ", errorMsg: " .. errorMsg .. ".\n"
  )
  Engine.SetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.hideMenusForGameStart"), 0)
  LuaUtils.LogQoS("lobby_error", comErrorData)
  if
      errorCode == Enum.errorCode.ERROR_SOFTRESTART
      or errorCode == Enum.errorCode.ERROR_SOFTRESTART_KEEPDW
      or errorCode == Enum.errorCode.ERROR_SOFTRESTART_SILENT
  then
    return
  elseif
      Engine.GetLobbyNetworkMode() == Enum.LobbyNetworkMode.LOBBY_NETWORKMODE_LIVE
      and not Engine.IsSignedInToDemonware(controller)
  then
    LobbyVM.ErrorShutdown(comErrorData)
  end
  if errorCode == Enum.errorCode.ERROR_DROP then
    Lobby.Stats.OnServerTimeout(controller)
  end
  if
      errorCode == Enum.errorCode.ERROR_UI
      or errorCode == Enum.errorCode.ERROR_DISCONNECT
      or errorCode == Enum.errorCode.ERROR_FATAL
      or errorCode == Enum.errorCode.ERROR_DROP
      or errorCode == Enum.errorCode.ERROR_SOFTRESTART_KEEPDW
      or errorCode == Enum.errorCode.ERROR_SOFTRESTART_SILENT
  then
    return
  elseif
      errorCode == Enum.errorCode.ERROR_NONE
      or errorCode == Enum.errorCode.ERROR_LUA
      or errorCode == Enum.errorCode.ERROR_FROM_STARTUP
      or errorCode == Enum.errorCode.ERROR_LOCALIZATION
      or errorCode == Enum.errorCode.ERROR_SERVERDISCONNECT
  then
    LuaUtils.UI_ShowErrorMessageDialog(controller, errorMsg)
    return
  elseif errorCode == Enum.errorCode.ERROR_SCRIPT or errorCode == Enum.errorCode.ERROR_SCRIPT_DROP then
    LuaUtils.UI_ShowErrorMessageDialog(controller, errorMsg)
    Lobby.ProcessQueue.AddToQueue("ErrorNonFatal", Lobby.Process.NonFatalError(errorMsg))
    return
  end
  error(
    "LobbyVM.OnError - Unhandled COM_ERROR: "
    .. LobbyVM.ComErrorCodeToString(errorCode)
    .. ", Message: "
    .. errorMsg
    .. ".\n"
  )
end

LobbyVM.ShutdownCleanupMP = function(controller) end

LobbyVM.ShutdownCleanupZM = function(controller) end

LobbyVM.ShutdownCleanupCP = function(controller) end

LobbyVM.ShutdownCleanup = function(controller)
  Dvar.ui_gametype:set("")
  Dvar.ui_mapname:set("")
  LobbyVM.ShutdownCleanupCP(controller)
  LobbyVM.ShutdownCleanupMP(controller)
  LobbyVM.ShutdownCleanupZM(controller)
end

LobbyVM.OnComErrorCleanup = function(comErrorData)
  local controller = comErrorData.controller
  local errorCode = comErrorData.errorCode
  local errorMsg = comErrorData.errorMsg
  local errorShutdown = comErrorData.errorShutdown
  Engine.LobbyLaunchClear()
  if errorShutdown ~= nil and errorShutdown == true then
    LobbyVM.ErrorShutdown(comErrorData)
    LobbyVM.ShutdownCleanup(comErrorData)
  end
  if errorCode == Enum.errorCode.ERROR_FATAL then
    LuaUtils.UI_ShowErrorMessageDialog(controller, errorMsg)
    local recoverProcess = Lobby.Process.Recover(controller)
    if recoverProcess ~= nil then
      Lobby.ProcessQueue.AddToQueue("ErrorFatal", recoverProcess)
    end
  elseif errorCode == Enum.errorCode.ERROR_DROP then
    local recoverProcess = Lobby.Process.Recover(controller)
    if recoverProcess then
      local errorPopup = Lobby.Actions.ErrorPopupMsg(errorMsg)
      Lobby.Process.AddActions(recoverProcess.tail, errorPopup, errorPopup, errorPopup)
      Lobby.ProcessQueue.AddToQueue("ERROR_DROP", recoverProcess)
    else
      LuaUtils.UI_ShowErrorMessageDialog(controller, errorMsg)
    end
  elseif errorCode == 1026 then
    Lobby.ProcessQueue.ClearQueue()
    LuaUtils.UI_ShowErrorMessageDialog(controller, errorMsg)
  elseif errorCode == 2050 then
    if string.len(errorMsg) > 0 then
      LuaUtils.UI_ShowErrorMessageDialog(controller, errorMsg)
    end
  elseif errorCode == 4098 and string.len(errorMsg) > 0 then
    LuaUtils.UI_ShowInfoMessageDialog(controller, errorMsg)
  end
end

LobbyVM.OnErrorShutdown = function(shutdownData)
  LobbyVM.ErrorShutdown(shutdownData)
end

LobbyVM.OnStorageRead = function(storageData)
  if storageData.result ~= Enum.StorageResult.STORAGE_SUCCESS then
    LuaUtils.LogQoS("storage", storageData)
  elseif storageData.fileType == Enum.StorageFileType.STORAGE_MP_STATS_ONLINE then
    Lobby.Stats.OnMPOnlineStatsDownloaded(storageData.controller)
  elseif storageData.fileType == Enum.StorageFileType.STORAGE_MP_LOADOUTS then
    Lobby.Stats.ValidateLoadouts(storageData.controller)
  elseif storageData.fileType == Enum.StorageFileType.STORAGE_MP_CLASS_SETS then
    Lobby.Stats.ValidateLoadouts(storageData.controller)
  elseif storageData.fileType == Enum.StorageFileType.STORAGE_COMMON_SETTINGS then
    Lobby.Stats.OnProfileCommonStatsDownloaded(storageData.controller)
  elseif storageData.fileType == Enum.StorageFileType.STORAGE_ZM_STATS_ONLINE then
    Lobby.Stats.OnZMOnlineStatsDownloaded(storageData.controller)
  end
end

LobbyVM.OnStorageWrite = function(storageData)
  if storageData.result ~= Enum.StorageResult.STORAGE_SUCCESS then
    LuaUtils.LogQoS("storage", storageData)
  elseif storageData.fileType == Enum.StorageFileType.STORAGE_MP_LOADOUTS then
    Lobby.Stats.ValidateLoadouts(storageData.controller)
  elseif storageData.fileType == Enum.StorageFileType.STORAGE_MP_STATS_ONLINE then
    Lobby.Stats.ValidateLoadouts(storageData.controller)
    Lobby.Stats.OnMPStatsUploaded(storageData.controller)
  end
end

LobbyVM.OnStorageWriteDispatch = function(storageData)
  local controller = storageData.controller
end

LobbyVM.OnRecordComScoreEvent = function(eventData)
  local controller = eventData.controller
  local eventCategoryName = eventData.eventCategoryName
  local eventParamsFlat = {}
  for key, value in pairs(eventData.EventParams) do
    table.insert(eventParamsFlat, key)
    table.insert(eventParamsFlat, value)
  end
  if #eventParamsFlat > 0 and #eventParamsFlat % 2 == 0 then
    Engine.RecordComScoreEvent(controller, eventCategoryName, unpack(eventParamsFlat))
  end
end

LobbyVM.OnKVSFlush = function()
  LobbyVM.LogGlobalData()
end

LobbyVM.OnPreExecFFOTD = function()
  Lobby.Platform.OnPreExecFFOTD()
  local shutdownParams = {
    controller = Engine.GetPrimaryController(),
    signoutUsers = false,
  }
  local ffotdShutdownModel = Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.FFOTDShutdown")
  Engine.SetModelValue(ffotdShutdownModel, true)
  LobbyVM.ErrorShutdown(shutdownParams)
  Engine.SetModelValue(ffotdShutdownModel, false)
end

LobbyVM.OnPostExecFFOTD = function()
  LobbyVM.LogGlobalData()
  Lobby.Platform.OnPostExecFFOTD()
end

LobbyVM.OnDWDisconnect = function(disconnectData)
  if not Engine.IsDedicatedServer() and disconnectData.controller == Engine.GetPrimaryController() then
    Engine.LobbyOnlineCancel(true)
  end
end

LobbyVM.DemoEndFinished = function(demoData)
  if LobbyVM.AutoJoin.data ~= nil then
    local joinProcess = nil
    if LobbyVM.AutoJoin.data.platform == true then
      if Lobby.Platform.PlatformSessionOrbisEnabled() then
        if LobbyVM.AutoJoin.data.playTogether == true then
          recoverProcess = Lobby.Process.Recover(controller, LobbyData.UITargets.UI_MODESELECT.id)
          joinProcess = Lobby.Platform.PS4ProcessPlayTogetherEvent(LobbyVM.AutoJoin.data.platformData)
        else
          joinProcess = Lobby.Platform.InGamePlatformJoinOrbis(LobbyVM.AutoJoin.data.platformData)
        end
      elseif Lobby.Platform.PlatformSessionDurangoEnabled() then
        joinProcess = Lobby.Platform.InGamePlatformJoinDurango(LobbyVM.AutoJoin.data.platformData)
      end
    else
      joinProcess = LobbyVM.GetJoinProcess(LobbyVM.AutoJoin.data)
    end
    if joinProcess ~= nil then
      Lobby.ProcessQueue.AddToQueue("DemoEnd", joinProcess)
    end
    LobbyVM.AutoJoin.data = nil
  end
end

LobbyVM.OnLimitedItemPromoUpdate = function(promoData)
  local itemCountModel =
      Engine.CreateModel(Engine.CreateModel(Engine.GetGlobalModel(), "LimitedItemPromo"), "itemCount")
  local itemCount = promoData.count
  local overrideCount = Engine.DvarInt(nil, "loot_limitedItemPromo_override_count")
  if overrideCount > -1 then
    itemCount = overrideCount
  end
  Engine.SetModelValue(itemCountModel, promoData.count)
end

LobbyVM.Gunsmith = {}
LobbyVM.Gunsmith.InitializeBuffer = function(buffer)
  local variantCount = #buffer.variant
  for i = 0, variantCount - 1, 1 do
    local variant = buffer.variant[i]
    variant.variantIndex:set(i)
    variant.paintjobSlot:set(Enum.CustomizationPaintjobInvalidID.CUSTOMIZATION_INVALID_PAINTJOB_SLOT)
    variant.paintjobIndex:set(Enum.CustomizationPaintjobInvalidID.CUSTOMIZATION_INVALID_PAINTJOB_INDEX)
  end
end

LobbyVM.Gunsmith.ClearBuffer = function(buffer)
  local variantCount = #buffer.variant
  for i = 0, variantCount - 1, 1 do
    local variant = buffer.variant[i]
    variant.variantName:set("")
    local attachmentCount = #variant.attachment
    for attachmentIndex = 0, attachmentCount - 1, 1 do
      variant.attachment[attachmentIndex]:set(0)
    end
    local attachmentVariantCount = #variant.attachmentVariant
    for attachmentVariantIndex = 0, attachmentVariantCount - 1, 1 do
      variant.attachmentVariant[attachmentVariantIndex]:set(0)
    end
    variant.reticleIndex:set(0)
    variant.camoIndex:set(0)
    variant.paintjobSlot:set(0)
    variant.paintjobIndex:set(0)
    variant.weaponIndex:set(0)
    variant.variantIndex:set(0)
    variant.sortIndex:set(0)
    variant.readOnly:set(0)
    variant.createTime:set(0)
  end
end

LobbyVM.InitilizeGunsmithBuffer = function(initData)
  local controller = initData.controller
  Engine.StorageReset(controller, Enum.StorageFileType.STORAGE_GUNSMITH)
  LobbyVM.Gunsmith.InitializeBuffer(Engine.StorageGetBuffer(controller, Enum.StorageFileType.STORAGE_GUNSMITH))
  Engine.StorageWrite(controller, Enum.StorageFileType.STORAGE_GUNSMITH)
end

LobbyVM.InitilizeZMLoadoutBuffer = function(loadoutData)
  local buffer = Engine.StorageGetBuffer(loadoutData.controller, loadoutData.storageFileType)
  if buffer then
    local cacLoadouts = buffer.cacLoadouts
    if cacLoadouts then
      LobbyVM.Gunsmith.ClearBuffer(cacLoadouts)
      LobbyVM.Gunsmith.InitializeBuffer(cacLoadouts)
    end
  end
end

LobbyVM.OnInitializeStats = function(statsData)
  local controller = statsData.controller
  local storageFileType = statsData.storageFileType
  if storageFileType == Enum.StorageFileType.STORAGE_MP_STATS_ONLINE then
    local itemStats = Engine.StorageGetBuffer(controller, Enum.StorageFileType.STORAGE_MP_STATS_ONLINE)
    itemStats = itemStats.itemStats
    if itemStats then
      for itemIndex = 1, 255, 1 do
        if
            Engine.ItemIndexValid(itemIndex, Enum.eModes.MODE_MULTIPLAYER)
            and Engine.GetItemUnlockLevel(itemIndex, Enum.eModes.MODE_MULTIPLAYER) == 0
            and not Engine.ItemIndexIsBlackMarket(itemIndex, Enum.eModes.MODE_MULTIPLAYER)
        then
          itemStats[itemIndex].markedOld:set(1)
        end
        itemStats[itemIndex].attachmentMarkedOld.gmod6:set(1)
        itemStats[itemIndex].attachmentMarkedOld.gmod7:set(1)
      end
    end
  elseif storageFileType == Enum.StorageFileType.STORAGE_ZM_STATS_ONLINE then
    local itemStats = Engine.StorageGetBuffer(controller, Enum.StorageFileType.STORAGE_ZM_STATS_ONLINE)
    itemStats = itemStats.itemStats
    if itemStats then
      for itemIndex = 1, 255, 1 do
        if
            Engine.ItemIndexValid(itemIndex, Enum.eModes.MODE_ZOMBIES)
            and Engine.GetItemUnlockLevel(itemIndex, Enum.eModes.MODE_ZOMBIES) == 0
            and Engine.GetDLCNameForItem(itemIndex, Enum.eModes.MODE_ZOMBIES) == nil
        then
          itemStats[itemIndex].markedOld:set(1)
        end
      end
    end
  end
end

LobbyVM.SetDefaultShowcaseWeapon = function(buffer, mode)
  local weaponIndex = Engine.GetItemIndexFromReference("ar_standard", mode)
  for i = 0, #buffer.characters - 1, 1 do
    local showcaseWeapon = buffer.characters[i].showcaseWeapon
    showcaseWeapon.weaponIndex:set(weaponIndex)
    showcaseWeapon.paintjobSlot:set(Enum.CustomizationPaintjobInvalidID.CUSTOMIZATION_INVALID_PAINTJOB_SLOT)
    showcaseWeapon.paintjobIndex:set(Enum.CustomizationPaintjobInvalidID.CUSTOMIZATION_INVALID_PAINTJOB_INDEX)
  end
end

LobbyVM.OnInitializeLoadouts = function(loadoutData)
  local controller = loadoutData.controller
  local storageFileType = loadoutData.storageFileType
  local statsNetworkType = loadoutData.statsNetworkType
  if storageFileType == Enum.StorageFileType.STORAGE_ZM_LOADOUTS then
    local cacLoadouts = Engine.StorageGetBuffer(controller, Enum.StorageFileType.STORAGE_ZM_LOADOUTS)
    cacLoadouts = cacLoadouts.cacLoadouts
    LobbyVM.Gunsmith.InitializeBuffer(cacLoadouts)
    cacLoadouts.characterContext.characterIndex:set(6)
    LobbyVM.SetDefaultShowcaseWeapon(cacLoadouts, Enum.eModes.MODE_ZOMBIES)
  elseif storageFileType == Enum.StorageFileType.STORAGE_ZM_LOADOUTS_OFFLINE then
    local cacLoadouts = Engine.StorageGetBuffer(controller, Enum.StorageFileType.STORAGE_ZM_LOADOUTS_OFFLINE)
    cacLoadouts = cacLoadouts.cacLoadouts
    LobbyVM.Gunsmith.InitializeBuffer(cacLoadouts)
    cacLoadouts.characterContext.characterIndex:set(6)
    LobbyVM.SetDefaultShowcaseWeapon(cacLoadouts, Enum.eModes.MODE_ZOMBIES)
  elseif storageFileType == Enum.StorageFileType.STORAGE_MP_LOADOUTS_OFFLINE then
    local cacLoadouts = Engine.StorageGetBuffer(controller, Enum.StorageFileType.STORAGE_MP_LOADOUTS_OFFLINE)
    cacLoadouts = cacLoadouts.cacLoadouts
    LobbyVM.SetDefaultShowcaseWeapon(cacLoadouts, Enum.eModes.MODE_MULTIPLAYER)
    local equippedHero =
        Engine.GetEquippedHeroForCACType(controller, Enum.eModes.MODE_MULTIPLAYER, Enum.CACType.CAC_TYPE_MP_OFFLINE)
    if equippedHero then
      local heroWeaponIndex = Engine.GetLoadoutItemIndexForHero(
        Enum.eModes.MODE_MULTIPLAYER,
        equippedHero,
        Enum.heroLoadoutTypes_e.HERO_LOADOUT_TYPE_WEAPON
      )
      if cacLoadouts and cacLoadouts.heroweapon then
        cacLoadouts.heroweapon:set(heroWeaponIndex)
      end
    end
  elseif storageFileType == Enum.StorageFileType.STORAGE_MP_LOADOUTS then
    local cacLoadouts = Engine.StorageGetBuffer(controller, Enum.StorageFileType.STORAGE_MP_LOADOUTS)
    cacLoadouts = cacLoadouts.cacLoadouts
    local customMatchCacLoadouts = Engine.StorageGetBuffer(controller, Enum.StorageFileType.STORAGE_MP_LOADOUTS)
    customMatchCacLoadouts = customMatchCacLoadouts.customMatchCacLoadouts
    local leagueCacLoadouts = Engine.StorageGetBuffer(controller, Enum.StorageFileType.STORAGE_MP_LOADOUTS)
    leagueCacLoadouts = leagueCacLoadouts.leagueCacLoadouts
    LobbyVM.SetDefaultShowcaseWeapon(cacLoadouts, Enum.eModes.MODE_MULTIPLAYER)
    LobbyVM.SetDefaultShowcaseWeapon(customMatchCacLoadouts, Enum.eModes.MODE_MULTIPLAYER)
    LobbyVM.SetDefaultShowcaseWeapon(leagueCacLoadouts, Enum.eModes.MODE_MULTIPLAYER)
    local equippedHero =
        Engine.GetEquippedHeroForCACType(controller, Enum.eModes.MODE_MULTIPLAYER, Enum.CACType.CAC_TYPE_MP_PUBLIC)
    if equippedHero then
      local heroWeaponIndex = Engine.GetLoadoutItemIndexForHero(
        Enum.eModes.MODE_MULTIPLAYER,
        equippedHero,
        Enum.heroLoadoutTypes_e.HERO_LOADOUT_TYPE_WEAPON
      )
      if cacLoadouts and cacLoadouts.heroweapon then
        cacLoadouts.heroweapon:set(heroWeaponIndex)
      end
      if customMatchCacLoadouts and customMatchCacLoadouts.heroweapon then
        customMatchCacLoadouts.heroweapon:set(heroWeaponIndex)
      end
    end
    if cacLoadouts then
      local heroList = Engine.GetHeroList(Enum.eModes.MODE_MULTIPLAYER)
      for i = 0, #heroList - 1, 1 do
        if cacLoadouts.characters[i].itemList then
          cacLoadouts.characters[i].itemList.body.items[0].markedOld:set(1)
          cacLoadouts.characters[i].itemList.helmet.items[0].markedOld:set(1)
        end
        if
            cacLoadouts.characters[i].tauntMarkedOld
            and cacLoadouts.characters[i].tauntMarkedOld.first_place
            and cacLoadouts.characters[i].tauntMarkedOld.first_place.markedOld
        then
          cacLoadouts.characters[i].tauntMarkedOld.first_place.markedOld[0]:set(1)
        end
      end
    end
  elseif storageFileType == Enum.StorageFileType.STORAGE_CP_LOADOUTS then
    local cacLoadouts = Engine.StorageGetBuffer(controller, Enum.StorageFileType.STORAGE_CP_LOADOUTS)
    cacLoadouts = cacLoadouts.cacLoadouts
    LobbyVM.SetDefaultShowcaseWeapon(cacLoadouts, Enum.eModes.MODE_CAMPAIGN)
    if cacLoadouts then
      local heroList = Engine.GetHeroList(Enum.eModes.MODE_CAMPAIGN)
      for i = 0, #heroList - 1, 1 do
        if cacLoadouts.characters[i].itemList then
          cacLoadouts.characters[i].itemList.body.items[0].markedOld:set(1)
          cacLoadouts.characters[i].itemList.helmet.items[0].markedOld:set(1)
          cacLoadouts.characters[i].itemList.helmet.items[1].markedOld:set(1)
        end
      end
    end
  elseif storageFileType == Enum.StorageFileType.STORAGE_CP_LOADOUTS_OFFLINE then
    local cacLoadouts = Engine.StorageGetBuffer(controller, Enum.StorageFileType.STORAGE_CP_LOADOUTS_OFFLINE)
    LobbyVM.SetDefaultShowcaseWeapon(cacLoadouts.cacLoadouts, Enum.eModes.MODE_CAMPAIGN)
  end
end

LobbyVM.LobbyHostLeftNoMigration = function(lobbyData)
  local controller = lobbyData.controller
  local lobbyType = lobbyData.lobbyType
  local lobbyMainMode = lobbyData.lobbyMainMode
  local lobbyNetworkMode = lobbyData.lobbyNetworkMode
  local maxClients = lobbyData.maxClients
  local isGameLobbyActive = lobbyData.isGameLobbyActive
  local isPrivateHost = lobbyData.isPrivateHost
  if not LobbyVM.CanClientLaunch(lobbyData) then
    return
  elseif Engine.IsLobbyInRecovery(lobbyType) then
    return
  elseif isGameLobbyActive == true and lobbyType == Enum.LobbyType.LOBBY_TYPE_GAME then
    if Lobby.ProcessQueue.GetCurrentRunningProcessName() ~= "HostLeftNoMigrationGame" then
      Lobby.ProcessQueue.AddToQueue("HostLeftNoMigrationGame", Lobby.Process.Recover(controller))
    end
  else
    Lobby.ProcessQueue.AddToQueue(
      "HostLeftNoMigration",
      Lobby.Process.HostLeftNoMigrationCreatePrivateLobby(controller, lobbyMainMode, maxClients)
    )
  end
  Engine.PlaySound(LobbyData.Sounds.LobbyClosed)
  LuaUtils.LogQoS("host_left_no_migration", lobbyData)
end

LobbyVM.LobbyHostLeft_InGameMigrateFinished = function(lobbyData)
  local controller = lobbyData.controller
  local recoverProcess = Lobby.Process.Recover(controller)
  if LobbyVM.AutoJoin.data ~= nil then
    local joinProcess = nil
    if LobbyVM.AutoJoin.data.platform == true then
      if Lobby.Platform.PlatformSessionOrbisEnabled() then
        if LobbyVM.AutoJoin.data.playTogether == true then
          recoverProcess = Lobby.Process.Recover(controller, LobbyData.UITargets.UI_MODESELECT.id)
          joinProcess = Lobby.Platform.PS4ProcessPlayTogetherEvent(LobbyVM.AutoJoin.data.platformData)
        else
          joinProcess = Lobby.Platform.InGamePlatformJoinOrbis(LobbyVM.AutoJoin.data.platformData)
        end
      elseif Lobby.Platform.PlatformSessionDurangoEnabled() then
        joinProcess = Lobby.Platform.InGamePlatformJoinDurango(LobbyVM.AutoJoin.data.platformData)
      end
    else
      joinProcess = LobbyVM.GetJoinProcess(LobbyVM.AutoJoin.data)
    end
    if joinProcess ~= nil then
      Lobby.Process.AppendProcess(recoverProcess, joinProcess)
    end
    LobbyVM.AutoJoin.data = nil
  end
  Lobby.ProcessQueue.AddToQueue("LobbyHostLeft_InGameMigrateFinished", recoverProcess)
  Engine.PlaySound(LobbyData.Sounds.LobbyClosed)
  LuaUtils.LogQoS("ingame_migrate_finished", lobbyData)
end
LobbyVM.LobbyClientLeftEvent = function(event)
  local controller = event.controller
  local lobbyType = event.lobbyType
  local lobbyMainMode = event.lobbyMainMode
  local lobbyNetworkMode = event.lobbyNetworkMode
  local maxClients = event.maxClients
  local recoverProcess = Lobby.Process.Recover(controller)
  if LobbyVM.AutoJoin.data ~= nil then
    local joinProcess = nil
    if LobbyVM.AutoJoin.data.platform == true then
      if Lobby.Platform.PlatformSessionOrbisEnabled() then
        if LobbyVM.AutoJoin.data.playTogether == true then
          recoverProcess = Lobby.Process.Recover(controller, LobbyData.UITargets.UI_MODESELECT.id)
          joinProcess = Lobby.Platform.PS4ProcessPlayTogetherEvent(LobbyVM.AutoJoin.data.platformData)
        else
          joinProcess = Lobby.Platform.InGamePlatformJoinOrbis(LobbyVM.AutoJoin.data.platformData)
        end
      elseif Lobby.Platform.PlatformSessionDurangoEnabled() then
        joinProcess = Lobby.Platform.InGamePlatformJoinDurango(LobbyVM.AutoJoin.data.platformData)
      end
    else
      joinProcess = LobbyVM.GetJoinProcess(LobbyVM.AutoJoin.data)
    end
    if joinProcess ~= nil then
      Lobby.Process.AppendProcess(recoverProcess, joinProcess)
    end
    LobbyVM.AutoJoin.data = nil
  end
  Lobby.ProcessQueue.AddToQueue("LobbyClientLeft", recoverProcess)
end

LobbyVM.Devmap = function(event)
  Lobby.ProcessQueue.AddToQueue("Devmap", Lobby.Process.Devmap(event.controller, event.mainMode))
end

LobbyVM.DevmapClient = function(event)
  Lobby.ProcessQueue.AddToQueue("DevmapClient", Lobby.Process.DevmapClient(event.controller))
end

LobbyVM.MatchmakingPriorityQuit = function(event)
  local uiTarget = LobbyData:UITargetFromId(LobbyData.GetLobbyNav())
  if
      uiTarget.id == LobbyData.UITargets.UI_MPLOBBYONLINEPUBLICGAME.id
      or uiTarget.id == LobbyData.UITargets.UI_DOALOBBYONLINEPUBLICGAME.id
  then
    Lobby.MatchmakingPriority.AddHost(event.hostSecId, Enum.JoinType.JOIN_TYPE_PLAYLIST)
  end
end

LobbyVM.NetworkModeChanged = function(event)
  Lobby.ProcessQueue.AddToQueue(
    "NetworkModeChanged",
    Lobby.Process.ReloadPrivateLobby(event.controller, event.networkMode)
  )
end

LobbyVM.GoForward = function(params)
  if LobbyVM.IsHostLaunching() then
    return
  end
  local controller = Engine.GetPrimaryController()
  local navToMenu = params.navToMenu
  local withParty = params.withParty
  local disbandParty = params.disbandParty
  local currentTarget = LobbyData:UITargetFromId(LobbyData.GetLobbyNav())
  local nextTarget = LobbyData:UITargetFromName(navToMenu)
  if currentTarget == nil then
    error("LobbyVM: No menu called '" .. currentMenu .. "' found.")
  end
  if nextTarget == nil then
    error("LobbyVM: No menu called '" .. navToMenu .. "' found.")
  end
  local forwardFunc = Lobby.Process.GetForwardFunc(currentTarget, nextTarget)
  if forwardFunc == nil then
    error(
      "LobbyVM: No forward process function found to move from '"
      .. currentTarget.name
      .. "' to '"
      .. navToMenu
      .. "'."
    )
  end
  local forwardProcess = forwardFunc(controller, currentTarget, nextTarget)
  if forwardProcess ~= nil and disbandParty == true then
    if Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_PRIVATE) == true then
      local managePartyLeaveProcess = Lobby.Process.ManagePartyLeave(controller)
      Lobby.Process.AppendProcess(managePartyLeaveProcess, forwardProcess)
      Lobby.ProcessQueue.AddToQueue(
        "GoForward from '"
        .. currentTarget.name
        .. "' menu to '"
        .. nextTarget.name
        .. "' menu AND ManagePartyLeave",
        managePartyLeaveProcess
      )
    else
      local reloadPrivateLobbyProcess = Lobby.Process.ReloadPrivateLobby(controller, Engine.GetLobbyNetworkMode())
      Lobby.Process.AppendProcess(reloadPrivateLobbyProcess, forwardProcess)
      Lobby.ProcessQueue.AddToQueue(
        "GoForward from '"
        .. currentTarget.name
        .. "' menu to '"
        .. nextTarget.name
        .. "' menu AND PartyMemberLeave",
        reloadPrivateLobbyProcess
      )
    end
    return
  end
  Lobby.ProcessQueue.AddToQueue(
    "GoForward from '" .. currentTarget.name .. "' menu to '" .. nextTarget.name .. "' menu.",
    forwardProcess
  )
end

LobbyVM.GoBack = function(params)
  if LobbyVM.IsHostLaunching() then
    return
  end
  local controller = Engine.GetPrimaryController()
  local withParty = params.withParty
  local currentTarget = LobbyData:UITargetFromId(LobbyData.GetLobbyNav())
  if currentTarget == nil then
    error("LobbyVM: No menu called '" .. currentMenu .. "' found.")
  end
  local backTarget, backProcessFunc = Lobby.Process.GetBackFunc(currentTarget)
  if backTarget == nil or backProcessFunc == nil then
    error("LobbyVM: No back process function found for '" .. currentMenu("'."))
  end
  Lobby.ProcessQueue.AddToQueue(
    "GoBackFrom" .. currentTarget.name,
    backProcessFunc(controller, currentTarget, backTarget, withParty)
  )
end

LobbyVM.ManagePartyLeave = function(params)
  if LobbyVM.IsHostLaunching() then
    return
  end
  local controller = params.controller
  if Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_PRIVATE) == true then
    Lobby.ProcessQueue.AddToQueue("ManagePartyLeave", Lobby.Process.ManagePartyLeave(controller))
  else
    Lobby.ProcessQueue.AddToQueue(
      "PartyMemberLeave",
      Lobby.Process.ReloadPrivateLobby(controller, Engine.GetLobbyNetworkMode())
    )
  end
end

LobbyVM.InGameJoin = function(joinData, leaveImmediately)
  LobbyVM.AutoJoin.data = joinData
  if
      joinData.migrating ~= nil
      and joinData.migrating == false
      and Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_GAME) == true
  then
    if leaveImmediately == true then
      LobbyVM.AutoJoin.leaveServerImmediately = true
      return
    elseif not LobbyVM.IsInTheaterLobby() then
      Engine.LeaveServerImmediately()
    end
  end
end

-- params = {
-- 	controller = f190_arg0,
-- 	xuid = f190_local2,
-- 	joinType = Enum.JoinType.JOIN_TYPE_FRIEND,
-- 	goBack = true
-- },
LobbyVM.GetJoinProcess = function(params)
  local lobbyType = Enum.LobbyType.LOBBY_TYPE_PRIVATE
  if Engine.IsLobbyActive(Enum.LobbyType.LOBBY_TYPE_GAME) then
    lobbyType = Enum.LobbyType.LOBBY_TYPE_GAME
  end
  if Engine.LobbyIsPlayerInLobby(lobbyType, params.xuid) then
    Engine.PrintInfo(Enum.consoleLabel.LABEL_LOBBY, "LobbyVM: Trying to Join lobby we are already part of\n")
    return nil
  elseif
      not LuaUtils.isPC
      and (Engine.IsCpStillDownloading() or Engine.IsMpStillDownloading() or Engine.IsZmStillDownloading())
  then
    Engine.PrintInfo(
      Enum.consoleLabel.LABEL_LOBBY,
      "LobbyVM: Cannot process a join when the game is still being installed.\n"
    )
    return nil
  else
    return Lobby.Process.Join(params.controller, params.xuid, params.joinType, LuaEnums.LEAVE_WITH_PARTY.WITH)
  end
end

-- params = {
-- 	controller = f190_arg0,
-- 	xuid = f190_local2,
-- 	joinType = Enum.JoinType.JOIN_TYPE_FRIEND,
-- 	goBack = true
-- },
LobbyVM.Join = function(params)
  local joinProcess = LobbyVM.GetJoinProcess(params)
  if joinProcess ~= nil then
    Lobby.ProcessQueue.AddToQueue("Join", joinProcess)
  end
end

LobbyVM.OnJoinComplete = function(event)
  Lobby.Matchmaking.OnJoinComplete(event)
  Lobby.Debug.OnJoinComplete(event)
end

LobbyVM.JoinSystemlink = function(params)
  Lobby.ProcessQueue.AddToQueue(
    "JoinSystemLink",
    Lobby.Process.JoinSystemlink(
      params.controller,
      params.lobbyMainMode,
      params.lobbyNetworkMode,
      params.hostXuid,
      params.hostInfo,
      params.ugcInfo
    )
  )
end

LobbyVM.CreateDedicatedLANLobby = function()
  Lobby.ProcessQueue.AddToQueue(
    "CreateDedicatedLANLobby",
    Lobby.Process.CreateDedicatedLANLobby(0, LobbyData.UITargets.UI_MPLOBBYLANGAME)
  )
end

LobbyVM.CreateDedicatedLobby = function(params)
  local unusedTable = {}
  local uiTarget = LobbyData.UITargets.UI_MPLOBBYONLINEPUBLICGAME
  if Engine.IsDedicatedServerMods() then
    uiTarget = LobbyData.UITargets.UI_MPLOBBYONLINEMODGAME
  end
  Lobby.ProcessQueue.AddToQueue("CreateDedicatedLobby", Lobby.Process.CreateDedicatedLobby(0, uiTarget))
end

LobbyVM.CreateDedicatedModsLobby = function(params)
  Lobby.ProcessQueue.AddToQueue(
    "CreateDedicatedModsLobby",
    Lobby.Process.CreateDedicatedModsLobby(0, LobbyData.UITargets.UI_MPLOBBYONLINEMODGAME)
  )
end

LobbyVM.ForceToMenu = function(params)
  Lobby.ProcessQueue.AddToQueue(
    "ForceToMenu",
    Lobby.Process.ForceToMenu(params.controller, LobbyData:UITargetFromName(params.menuName), params.msg)
  )
end

LobbyVM.LobbyClientPromoteToHost = function(params)
  Lobby.ProcessQueue.AddToQueue(
    "PromoteClientToHost",
    Lobby.Process.PromoteClientToHost(
      params.controller,
      params.lobbyMainMode,
      params.lobbyType,
      params.lobbyMode,
      params.maxClients,
      params.hostInfo,
      params.isAdvertised,
      params.isInGame,
      params.newMigrateIndex
    )
  )
end

LobbyVM.LobbyLeaveWithParty = function(params)
  local leaveProcess = Lobby.Process.LeaveWithParty(params.lobbyModule, params.lobbyType, params.lobbyMode)
  if leaveProcess == nil then
    return
  else
    Lobby.ProcessQueue.AddToQueue("LeaveWithParty", leaveProcess)
  end
end

LobbyVM.CanLobbyCanMigrate = function(params)
  local lobbyType = params.lobbyType
  local lobbyMode = params.lobbyMode
  if lobbyMode == Enum.LobbyMode.LOBBY_MODE_CUSTOM then
    return false
  elseif lobbyMode == Enum.LobbyMode.LOBBY_MODE_THEATER then
    return false
  elseif Engine.IsLobbyInRecovery ~= nil and Engine.IsLobbyInRecovery(lobbyType) then
    return false
  else
    return true
  end
end

LobbyVM.LobbyLocalClientLeave = function(params)
  if not Lobby.ProcessQueue.IsQueueEmpty() then
    return false
  else
    local leaveProcess = Lobby.Process.LocalClientLeave(params.controller, params.xuid)
    if leaveProcess == nil then
      return false
    else
      Lobby.ProcessQueue.AddToQueue("LobbyLocalClientLeave", leaveProcess)
      return true
    end
  end
end

LobbyVM.SetMaxLocalPlayers = function(params)
  Dvar.lobby_maxLocalPlayers:set(math.min(params.maxLocalClients, Engine.GetMaxLocalControllers()))
end

LobbyVM.LobbySettings = function(params)
  local controller = params.controller
  local target = params.toTarget
  local skipSwitchMode = params.skipSwitchMode
  local isDevMap = params.isDevMap
  if type(target) == "number" then
    target = LobbyData:UITargetFromId(target)
  end
  if target == nil then
    Engine.PrintInfo(Enum.consoleLabel.LABEL_LOBBYHOST, "Tried to apply settings for an invalid target.\n")
    return
  end
  Engine.PrintInfo(Enum.consoleLabel.LABEL_LOBBYHOST, "Applying settings for menu: " .. target.name .. ".\n")
  if not skipSwitchMode then
    local currentMainMode = Engine.GetLobbyMainMode()
    local targetMainMode = target.mainMode
    if LuaUtils.isPC then
      if targetMainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_CP and Engine.IsCpStillDownloading() then
        Engine.ComError(Enum.errorCode.ERROR_SOFTRESTART, "MENU_JOIN_RESULT_CP_REQUIRED")
      end
      if targetMainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_MP and Engine.IsMpStillDownloading() then
        Engine.ComError(Enum.errorCode.ERROR_SOFTRESTART, "MENU_JOIN_RESULT_MP_REQUIRED")
      end
      if targetMainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_ZM and Engine.IsZmStillDownloading() then
        Engine.ComError(Enum.errorCode.ERROR_SOFTRESTART, "MENU_JOIN_RESULT_ZM_REQUIRED")
      end
    end
    if Engine.SwitchCampaignMode then
      if targetMainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_CP then
        if LuaUtils.IsCPZMTarget(target.id) then
          Engine.SwitchCampaignMode(Enum.CampaignMode.CAMPAIGNMODE_ZOMBIES)
        else
          Engine.SwitchCampaignMode(Enum.CampaignMode.CAMPAIGNMODE_DEFAULT)
        end
      else
        Engine.SwitchCampaignMode(Enum.CampaignMode.CAMPAIGNMODE_DEFAULT)
      end
    end
    if currentMainMode ~= targetMainMode then
      if targetMainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_MP then
        Engine.SwitchMode(controller, "mp")
      elseif targetMainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_CP then
        Engine.SwitchMode(controller, "cp")
      elseif targetMainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_ZM then
        Engine.SwitchMode(controller, "zm")
      else
        Engine.SwitchMode(controller, "")
      end
    end
  end
  if isDevMap ~= true and target.eGameModes == Enum.eGameModes.MODE_GAME_INVALID then
    Engine.ResetGametypeSettings()
  end
  LobbyVM.SetMaxLocalPlayers(target)
  Engine.SetLobbyMode(target.lobbyType, target.lobbyMode)
  Engine.TEMPGameModeSetMode(target.eGameModes)
end

LobbyVM.UpdateUI = function(params)
  local target = params.toTarget
  local controller = params.controller
  if type(target) == "number" then
    target = LobbyData:UITargetFromId(target)
  end
  local function roomNameMapping(roomName)
    if roomName == "auto" then
      return "room2"
    else
      return roomName
    end
  end

  LobbyData.SetLobbyNav(target)
  Engine.SetLobbyUIScreen(target.id)
  Engine.SetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.lobbyMode", true), target.lobbyMode)
  Engine.SetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.lobbyMainMode", true), target.mainMode)
  Engine.SetModelValue(
    Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.lobbyGameMode", true),
    target.eGameModes
  )
  Engine.SetModelValue(
    Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.room", true),
    roomNameMapping(target.room)
  )
end

LobbyVM.Leaderboard_PopulateCustomList = function(params)
  return Lobby.Leaderboard.PopulateCustomList(params)
end

LobbyVM.Leaderboard_CalculateLBColValue = function(params)
  return Lobby.Leaderboard.CalculateLBColValue(params)
end

LobbyVM.OnShouldWriteLeaderboard = function(params)
  return Lobby.Leaderboard.OnShouldWriteLeaderboard(params)
end

LobbyVM.CheckDLCBit = function(dlcBits, requiredBit)
  if Engine.DvarInt(nil, "ui_freeDLC1") == 1 then
    dlcBits = dlcBits | Enum.ContentFlagBits.CONTENT_DLC1
  end
  if
      Engine.DvarInt(nil, "ui_freeDLCPC") == 1
      and dlcBits & Enum.ContentFlagBits.CONTENT_DLCPC == Enum.ContentFlagBits.CONTENT_DLCPC
  then
    return true
  else
    return dlcBits & requiredBit == requiredBit
  end
end

LobbyVM.LaunchGameExec = function(controller, lobbyType)
  if lobbyType ~= Enum.LobbyType.LOBBY_TYPE_GAME then
    Engine.PrintError(
      Enum.consoleLabel.LABEL_LOBBYHOST,
      "Launch game not supported for this lobby type(" .. lobbyType .. ")\n"
    )
    return false
  elseif Lobby.ProcessQueue.IsQueueEmpty() then
    local mapName = Dvar.ui_mapname:get()
    local gameType = Dvar.ui_gametype:get()
    Engine.PrintInfo(Enum.consoleLabel.LABEL_LOBBYHOST, "** Launching game... **\n")
    Engine.Exec(controller, "lobbyLaunchGame " .. lobbyType .. " " .. mapName .. " " .. gameType)
    return true
  else
    local queueHead = Lobby.ProcessQueue.GetQueueHead()
    Engine.PrintError(
      Enum.consoleLabel.LABEL_LOBBYHOST,
      "Warning: Could not launch game. Lobby process '" .. queueHead.name .. "' in progress.\n"
    )
    return false
  end
end

LobbyVM.LaunchDemoExec = function(controller, lobbyType)
  if lobbyType ~= Enum.LobbyType.LOBBY_TYPE_GAME then
    Engine.PrintError(
      Enum.consoleLabel.LABEL_LOBBYHOST,
      "Launch demo not supported for this lobby type(" .. lobbyType .. ")\n"
    )
    return
  elseif Lobby.ProcessQueue.IsQueueEmpty() then
    Engine.PrintInfo(Enum.consoleLabel.LABEL_LOBBYHOST, "** Launching demo... **\n")
    Engine.Exec(controller, "lobbyLaunchDemo")
    LuaUtils.UI_ClearErrorMessageDialog()
  else
    local queueHead = Lobby.ProcessQueue.GetQueueHead()
    Engine.PrintError(
      Enum.consoleLabel.LABEL_LOBBYHOST,
      "Warning: Could not launch damo. Lobby process '" .. queueHead.name .. "' in progress.\n"
    )
  end
end

LobbyVM.HostLaunchInit = function()
  Engine.ForceNotifyModelSubscriptions(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.closePopups"))
  LobbyVM.hostLaunch = {}
  LobbyVM.hostLaunch.fadeToBlack = false
  local currentTime = Engine.milliseconds()
  LobbyVM.hostLaunch.startTime = currentTime
  LobbyVM.hostLaunch.fadeToBlackTime = currentTime + Dvar.lobbyLaunch_fadeToBlackDelay:get()
  LobbyVM.hostLaunch.launchTime = currentTime + Dvar.lobbyLaunch_gameLaunchDelay:get()
  LobbyVM.hostLaunch.waitForClientAckTime = currentTime + Dvar.lobbyLaunch_waitForClientAckDelay:get()
  Lobby.Timer.ResetUIModel()
  Engine.SetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.spinnerActive"), true)
  return true
end

LobbyVM.CheckNeedInstallUGC = function(ugcInfo, caller)
  if ugcInfo then
    if ugcInfo.ugcType == LuaEnums.MODS_BASE_PATH then
      if LobbyVM.OnModUpdate({
            ugcInfo = ugcInfo,
          }) then
        Engine.PrintWarning(
          Enum.consoleLabel.LABEL_LOBBY,
          "Caller: "
          .. caller
          .. ", ugcType: "
          .. ugcInfo.ugcType
          .. ": "
          .. ugcInfo.ugcName
          .. " not installed yet.\n"
        )
        return true
      end
    elseif
        ugcInfo.ugcType == LuaEnums.USERMAP_BASE_PATH
        and LobbyVM.OnUsermapUpdate({
          ugcInfo = ugcInfo,
        }, true)
    then
      Engine.PrintWarning(
        Enum.consoleLabel.LABEL_LOBBY,
        "Caller: "
        .. caller
        .. ", ugcType: "
        .. ugcInfo.ugcType
        .. ": "
        .. ugcInfo.ugcName
        .. " not installed yet.\n"
      )
      return true
    end
  end
  return false
end

LobbyVM.HostLaunchPump = function(params)
  local lobbyUIScreen = Engine.GetLobbyUIScreen()
  if lobbyUIScreen == LobbyData.UITargets.UI_MPLOBBYONLINEPUBLICGAME.id then
    local shouldRestartTimer = false
    if Dvar.tu9_hostLaunchCheckMinPlayer:get() == true then
      local clientCount = Engine.GetLobbyClientCount(
        Enum.LobbyModule.LOBBY_MODULE_HOST,
        Enum.LobbyType.LOBBY_TYPE_GAME,
        Enum.LobbyClientType.LOBBY_CLIENT_TYPE_ALL
      )
      if clientCount < Dvar.party_minplayers:get() then
        Engine.PrintWarning(
          Enum.consoleLabel.LABEL_LOBBYHOST,
          "LobbyVM.LaunchGameExec: Not enough players ("
          .. tostring(clientCount)
          .. "/"
          .. tostring(Dvar.party_minplayers:get())
          .. "), restart timer.\n"
        )
        shouldRestartTimer = true
      end
    end
    if Dvar.tu9_hostLaunchCheckUnbalancedTeams:get() == true and true == CoDShared.IsGametypeTeamBased() then
      local sessionClients =
          Engine.LobbyGetSessionClients(Enum.LobbyModule.LOBBY_MODULE_HOST, Enum.LobbyType.LOBBY_TYPE_GAME)
      if sessionClients ~= nil then
        local alliesCount = 0
        local axisCount = 0
        for index, sessionClient in ipairs(sessionClients.sessionClients) do
          if sessionClient.team == Enum.team_t.TEAM_ALLIES then
            alliesCount = alliesCount + 1
          end
          if sessionClient.team == Enum.team_t.TEAM_AXIS then
            axisCount = axisCount + 1
          end
        end
        if math.abs(alliesCount - axisCount) > 1 then
          Engine.PrintWarning(
            Enum.consoleLabel.LABEL_LOBBYHOST,
            "LobbyVM.LaunchGameExec: Teams are not balanced (teamAlliesCount: "
            .. tostring(alliesCount)
            .. "/teamAxisCount: "
            .. tostring(axisCount)
            .. "), restart timer.\n"
          )
          shouldRestartTimer = true
        end
      end
    end
    if shouldRestartTimer == true then
      Engine.LobbyLaunchClear()
      LobbyVM.HostLaunchClear()
      local uiTarget = LobbyData:UITargetFromId(lobbyUIScreen)
      Lobby.Timer.HostingLobby({
        controller = Engine.GetPrimaryController(),
        lobbyType = Enum.LobbyType.LOBBY_TYPE_GAME,
        mainMode = uiTarget.mainMode,
        lobbyTimerType = uiTarget.lobbyTimerType,
        matchEnded = true,
        status = Lobby.Timer.LOBBY_STATUS.RESET_TO_NEED,
      })
      return false
    end
  end
  if Engine.GetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.spinnerActive")) == false then
    Engine.SetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.spinnerActive"), true)
  end
  if Engine.DvarBool(nil, "ffotdtempfixHostLaunchPump") then
    if not Engine.IsDedicatedServer() then
      if
          LobbyVM.hostLaunch.fadeToBlack == false
          and nil ~= LobbyVM.hostLaunch.fadeToBlackTime
          and Engine.milliseconds() > LobbyVM.hostLaunch.fadeToBlackTime
      then
        LobbyVM.hostLaunch.fadeToBlack = true
        Engine.SetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.hideMenusForGameStart"), 1)
      end
      if nil ~= LobbyVM.hostLaunch.launchTime and Engine.milliseconds() < LobbyVM.hostLaunch.launchTime then
        return false
      end
    end
  elseif not Engine.IsDedicatedServer() then
    if LobbyVM.hostLaunch.fadeToBlack == false and Engine.milliseconds() > LobbyVM.hostLaunch.fadeToBlackTime then
      LobbyVM.hostLaunch.fadeToBlack = true
      Engine.SetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.hideMenusForGameStart"), 1)
    end
    if nil ~= LobbyVM.hostLaunch.launchTime and Engine.milliseconds() < LobbyVM.hostLaunch.launchTime then
      return false
    end
  end
  if Engine.IsWaitingForDatabase and Engine.IsWaitingForDatabase() then
    return false
  elseif params.ugcInfo and LobbyVM.CheckNeedInstallUGC(params.ugcInfo, "LobbyVM.HostLaunchPump") then
    return false
  elseif
      not params.hasAllClientsGotLatestStateMsg or Engine.milliseconds() < LobbyVM.hostLaunch.waitForClientAckTime
  then
    return false
  elseif not Engine.IsDedicatedServer() then
    if Engine.IsWaitingForDatabase() then
      return false
    elseif Engine.IsCommonFastFileLoaded and not Engine.IsCommonFastFileLoaded() then
      return false
    end
  end
  return true
end

LobbyVM.HostLaunchClear = function()
  Engine.SetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.hideMenusForGameStart"), 0)
  LobbyVM.hostLaunch.startTime = nil
  Engine.SetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.spinnerActive"), false)
end

LobbyVM.IsHostLaunching = function()
  if LobbyVM.hostLaunch.startTime ~= nil then
    return true
  elseif Engine.LobbyHostLaunchTaskInProgress and Engine.LobbyHostLaunchTaskInProgress() then
    return true
  else
    return false
  end
end

LobbyVM.HostLaunch = function(launchState)
  if launchState.stage == Enum.LaunchGameState.LAUNCH_GAME_STATE_START then
    return LobbyVM.HostLaunchInit()
  elseif launchState.stage == Enum.LaunchGameState.LAUNCH_GAME_STATE_PUMP then
    return LobbyVM.HostLaunchPump(launchState)
  else
    LobbyVM.HostLaunchClear()
  end
end

LobbyVM.CheckAdvertizeSteamServer = function()
  if Engine.IsDedicatedServer() == true and LuaUtils.Mods_IsUsingMods() then
    return true
  elseif Engine.SessionMode_IsPublicOnlineGame() then
    return false
  elseif Engine.GetLobbyNetworkMode() ~= Enum.LobbyNetworkMode.LOBBY_NETWORKMODE_LIVE then
    return false
  else
    local currentMenuTarget = LobbyData.GetCurrentMenuTarget()
    if
        currentMenuTarget.mainMode ~= Enum.LobbyMainMode.LOBBY_MAINMODE_MP
        and currentMenuTarget.mainMode ~= Enum.LobbyMainMode.LOBBY_MAINMODE_ZM
    then
      return false
    elseif currentMenuTarget.lobbyType ~= Enum.LobbyType.LOBBY_TYPE_GAME then
      return false
    elseif currentMenuTarget.lobbyMode ~= Enum.LobbyMode.LOBBY_MODE_CUSTOM then
      return false
    else
      local lobbyUIScreen = Engine.GetLobbyUIScreen()
      if
          lobbyUIScreen == LobbyData.UITargets.UI_ZMLOBBYONLINEPUBLICGAME.id
          or lobbyUIScreen == LobbyData.UITargets.UI_ZMLOBBYONLINE.id
          or lobbyUIScreen == LobbyData.UITargets.UI_MPLOBBYMAIN.id
          or lobbyUIScreen == LobbyData.UITargets.UI_MPLOBBYONLINEPUBLICGAME.id
          or lobbyUIScreen == LobbyData.UITargets.UI_MPLOBBYONLINEARENAGAME.id
          or lobbyUIScreen == LobbyData.UITargets.UI_MODESELECT.id
          or lobbyUIScreen == LobbyData.UITargets.UI_FRLOBBYONLINEGAME.id
      then
        return false
      elseif
          Engine.GetProfileVarInt(Engine.GetPrimaryController(), "party_privacyStatus")
          == Enum.PartyPrivacy.PARTY_PRIVACY_CLOSED
      then
        return false
      else
        return true
      end
    end
  end
end

LobbyVM.CheckInitSteamServer = function()
  if Engine.IsInGame() then
    return true
  elseif Engine.IsDedicatedServer() == true then
    return true
  else
    local currentMenuTarget = LobbyData.GetCurrentMenuTarget()
    if
        currentMenuTarget.mainMode ~= Enum.LobbyMainMode.LOBBY_MAINMODE_MP
        and currentMenuTarget.mainMode ~= Enum.LobbyMainMode.LOBBY_MAINMODE_ZM
    then
      return false
    elseif currentMenuTarget.lobbyType ~= Enum.LobbyType.LOBBY_TYPE_GAME then
      return false
    elseif currentMenuTarget.lobbyMode ~= Enum.LobbyMode.LOBBY_MODE_CUSTOM then
      return false
    else
      local lobbyUIScreen = Engine.GetLobbyUIScreen()
      if
          lobbyUIScreen == LobbyData.UITargets.UI_ZMLOBBYONLINEPUBLICGAME.id
          or lobbyUIScreen == LobbyData.UITargets.UI_ZMLOBBYONLINE.id
          or lobbyUIScreen == LobbyData.UITargets.UI_MPLOBBYMAIN.id
          or lobbyUIScreen == LobbyData.UITargets.UI_MPLOBBYONLINEPUBLICGAME.id
          or lobbyUIScreen == LobbyData.UITargets.UI_MPLOBBYONLINEARENAGAME.id
          or lobbyUIScreen == LobbyData.UITargets.UI_MODESELECT.id
          or lobbyUIScreen == LobbyData.UITargets.UI_FRLOBBYONLINEGAME.id
      then
        return false
      else
        return true
      end
    end
  end
end

LobbyVM.GetNeededDLCBits = function()
  local lobbyUIScreen = Engine.GetLobbyUIScreen()
  if
      lobbyUIScreen == LobbyData.UITargets.UI_ZMLOBBYONLINEPUBLICGAME.id
      or lobbyUIScreen == LobbyData.UITargets.UI_MPLOBBYONLINEPUBLICGAME.id
      or lobbyUIScreen == LobbyData.UITargets.UI_MPLOBBYONLINEARENAGAME.id
  then
    return Lobby.Matchmaking.GetMapPackBits(
      Engine.GetDLCBitsForLobby(Enum.LobbyType.LOBBY_TYPE_GAME),
      Engine.GetPlaylistID()
    )
  elseif
      lobbyUIScreen == LobbyData.UITargets.UI_MODESELECT.id
      or lobbyUIScreen == LobbyData.UITargets.UI_MPLOBBYMAIN.id
      or lobbyUIScreen == LobbyData.UITargets.UI_MPLOBBYONLINE.id
      or lobbyUIScreen == LobbyData.UITargets.UI_MPLOBBYONLINEARENA.id
      or lobbyUIScreen == LobbyData.UITargets.UI_ZMLOBBYONLINE.id
  then
    return 0
  end
  local mapName = Dvar.ui_mapname:get()
  if mapName == nil or mapName == "" or mapName == "mp_array" then
    mapName = Engine.GetCurrentMap()
  end
  return Engine.GetDLCBitForMapName(mapName)
end

LobbyVM.DLCMapCheck = function(params)
  local mapName = Dvar.ui_mapname:get()
  local dlcBits = Engine.GetDLCBits()
  local neededDLCBits = LobbyVM.GetNeededDLCBits()
  if
      neededDLCBits == Enum.dlcIndex_t.CONTENT_DEV_MAP_INDEX
      or neededDLCBits == Enum.dlcIndex_t.CONTENT_USERMAP_INDEX
  then
    return true
  elseif not LobbyVM.CheckDLCBit(dlcBits, neededDLCBits) then
    Engine.PrintWarning(
      Enum.consoleLabel.LABEL_LOBBYCLIENT,
      "Leaving game because map '"
      .. mapName
      .. "' is in contentpack '"
      .. neededDLCBits
      .. "' but our dlcbits is '"
      .. dlcBits
      .. "'.\n"
    )
    local controller = Engine.GetPrimaryController()
    local lobbyClientEndAction = Lobby.Actions.LobbyClientEnd(Enum.LobbyType.LOBBY_TYPE_GAME)
    local reloadPrivateLobbyProcess = Lobby.Process.ReloadPrivateLobby(controller, Engine.GetLobbyNetworkMode())
    local lobbySettingsAction = Lobby.Actions.LobbySettings(controller, LobbyData.UITargets.UI_MODESELECT)
    local updateUIAction = Lobby.Actions.UpdateUI(controller, LobbyData.UITargets.UI_MODESELECT)
    local errorMsg = "PLATFORM_MISSINGMAP"
    if LobbyVM.CheckDLCBit(neededDLCBits, Enum.ContentFlagBits.CONTENT_DLC6) then
      errorMsg = "PLATFORM_MISSINGBO4PREORDER"
    end
    local errorPopupAction = Lobby.Actions.ErrorPopupMsg(errorMsg)
    Lobby.Process.ForceAction(lobbyClientEndAction, reloadPrivateLobbyProcess.tail)
    Lobby.Process.ForceAction(reloadPrivateLobbyProcess.tail, lobbySettingsAction)
    Lobby.Process.ForceAction(lobbySettingsAction, updateUIAction)
    Lobby.Process.ForceAction(updateUIAction, errorPopupAction)
    Lobby.ProcessQueue.AddToQueue("MissingMap", reloadPrivateLobbyProcess)
    return false
  else
    return true
  end
end

LobbyVM.CanClientLaunch = function(params)
  if params.modInfo and LobbyVM.CheckNeedInstallUGC(params.modInfo.ugcInfo, "LobbyVM.CanClientLaunch") then
    return false
  elseif params.usermapInfo then
    if LobbyVM.CheckNeedInstallUGC(params.usermapInfo.ugcInfo, "LobbyVM.CanClientLaunch") then
      return false
    else
      return true
    end
  elseif LobbyVM.DLCMapCheck(params) == false then
    Engine.LobbyLaunchClear()
    return false
  else
    return true
  end
end

LobbyVM.ClientLaunchInit = function(params)
  local justConnected = params.justConnected
  if not LobbyVM.CanClientLaunch(params) then
    return false
  end
  Engine.ForceNotifyModelSubscriptions(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.closePopups"))
  LobbyVM.clientLaunch = {}
  LobbyVM.clientLaunch.fadeToBlack = false
  local fadeToBlackDelay = Dvar.lobbyLaunch_fadeToBlackDelay:get()
  local gameLaunchDelay = Dvar.lobbyLaunch_gameLaunchDelay:get()
  if justConnected then
    fadeToBlackDelay = Dvar.lobbyLaunch_fadeToBlackDelayOnConnect:get()
    gameLaunchDelay = Dvar.lobbyLaunch_gameLaunchDelayOnConnect:get()
  end
  local currentTime = Engine.milliseconds()
  LobbyVM.clientLaunch.startTime = currentTime
  LobbyVM.clientLaunch.fadeToBlackTime = currentTime + fadeToBlackDelay
  LobbyVM.clientLaunch.launchTime = currentTime + gameLaunchDelay
  Lobby.Timer.ResetUIModel()
  Engine.SetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.spinnerActive"), true)
  return true
end

LobbyVM.ClientLaunchPump = function(params)
  if Engine.GetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.spinnerActive")) == false then
    Engine.SetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.spinnerActive"), true)
  end
  if false == LobbyVM.clientLaunch.fadeToBlack and Engine.milliseconds() > LobbyVM.clientLaunch.fadeToBlackTime then
    LobbyVM.clientLaunch.fadeToBlack = true
    Engine.SetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.hideMenusForGameStart"), 1)
  end
  if Engine.IsWaitingForDatabase and Engine.IsWaitingForDatabase() then
    return false
  elseif Engine.milliseconds() < LobbyVM.clientLaunch.launchTime then
    return false
  elseif params.modInfo and LobbyVM.CheckNeedInstallUGC(params.modInfo.ugcInfo, "LobbyVM.ClientLaunchPump") then
    return false
  elseif
      params.usermapInfo and LobbyVM.CheckNeedInstallUGC(params.usermapInfo.ugcInfo, "LobbyVM.ClientLaunchPump")
  then
    return false
  elseif Engine.IsWaitingForDatabase() then
    return false
  elseif Engine.IsCommonFastFileLoaded and not Engine.IsCommonFastFileLoaded() then
    return false
  else
    return true
  end
end

LobbyVM.ClientLaunchClear = function()
  Engine.SetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.hideMenusForGameStart"), 0)
  Engine.SetModelValue(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.spinnerActive"), false)
end

LobbyVM.ClientLaunch = function(launchState)
  if launchState.stage == Enum.LaunchGameState.LAUNCH_GAME_STATE_START then
    return LobbyVM.ClientLaunchInit(launchState)
  elseif launchState.stage == Enum.LaunchGameState.LAUNCH_GAME_STATE_PUMP then
    return LobbyVM.ClientLaunchPump(launchState)
  else
    LobbyVM.ClientLaunchClear()
  end
end

LobbyVM.LaunchGame = function(params)
  Lobby.Timer.LaunchGame(params)
end

LobbyVM.LaunchDemo = function(params)
  Lobby.Timer.LaunchGame(params)
end

LobbyVM.UpdateLobbyStatusInfo = function(params)
  Lobby.Timer.UpdateLobbyStatusInfo(params)
  Lobby.Pregame.UpdateLobbyStatusInfo(params)
end

LobbyVM.OnClientSelectionReceived = function(event)
  Lobby.Pregame.OnClientSelectionReceived(event)
end

LobbyVM.LobbyHost_ClientSelectionReceived = function(event)
  Lobby.Pregame.OnClientSelectionReceived(event)
end

LobbyVM.SwitchTeam = function(params)
  Lobby.TeamSelection.SwitchTeam(params)
end

LobbyVM.GameModeChanged = function(event)
  Lobby.TeamSelection.GameModeChanged(event)
end

LobbyVM.OnJoinPartyPrivacyCheck = function(params)
  return Lobby.PartyPrivacy.OnJoinPartyPrivacyCheck(params)
end

LobbyVM.OnCanFitLobbys = function(params)
  return Lobby.TeamSelection.OnCanFitLobbys(params)
end

LobbyVM.OnCanBroadcastHostInfo = function(event)
  local uiTarget = LobbyData:UITargetFromId(LobbyData.GetLobbyNav())
  if
      uiTarget.id == LobbyData.UITargets.UI_FRLOBBYONLINEGAME.id
      or uiTarget.id == LobbyData.UITargets.UI_FRLOBBYLANGAME.id
  then
    return false
  else
    return true
  end
end

LobbyVM.OnUpdateAdvertising = function(event)
  if event.lobbyType == Enum.LobbyType.LOBBY_TYPE_GAME then
    Lobby.Matchmaking.UpdateAdvertising("force update from code")
  end
end

LobbyVM.UGCOffensiveEmblemAdd = function(params)
  Lobby.UGC.OffensiveEmblemAdd(params)
end

LobbyVM.OnPlatformJoin = function(event)
  Lobby.Platform.OnPlatformJoin(event)
end

LobbyVM.Invite = function(params)
  Lobby.Platform.Invite(params)
end

LobbyVM.OnPlatformSessionMultiplayerSubscriptionLost = function(event)
  Lobby.Platform.OnPlatformSessionMultiplayerSubscriptionLost(event)
end

LobbyVM.OnPlatformSessionMultiplayerSessionChanged = function(event)
  Lobby.Platform.OnPlatformSessionMultiplayerSessionChanged(event)
end

LobbyVM.OnPlatformSuspend = function(event)
  Lobby.Platform.OnPlatformSuspend(event)
end

LobbyVM.OnPlatformResume = function(event)
  Lobby.Platform.OnPlatformResume(event)
end

LobbyVM.OnPlatformPlayTogether = function(event)
  Lobby.Platform.OnPlatformPlayTogether(event)
end

LobbyVM.SessionSQJRefreshInfo = function(event)
  Lobby.Debug.SessionSQJRefreshInfo()
end

LobbyVM.StopLobbyTimer = function(params)
  Lobby.Timer.HostingLobbyEnd(params)
end

LobbyVM.ChangeSigninState = function(event)
  local signInProcess = nil
  if event.onlineState == Enum.LiveUserState.LIVE_USER_SIGNED_OUT then
    Lobby.Anticheat.OnControllerSignedOut(event.controller)
    if event.isPrimary then
      signInProcess = Lobby.Process.PrimaryControllerSignedOut(event.controller)
    end
  elseif event.onlineState == Enum.LiveUserState.LIVE_USER_SIGNED_IN then
    if event.isPrimary then
      signInProcess = Lobby.Process.PrimaryControllerSignedIn(event.controller)
    end
  else
    DebugPrint("Warning unknown signin State [" .. event.onlineState .. "]")
  end
  if signInProcess ~= nil then
    Lobby.ProcessQueue.AddToQueue("ChangeSignInState", signInProcess)
  end
end
LobbyVM.JoinResultToString = function(joinResult, includeDebugPrefix)
  local result = {
    debug = "",
    errorMsg = "",
  }
  if includeDebugPrefix == true then
    result.debug = "Enum.JoinResult.JOIN_RESULT_"
  end
  if joinResult == Enum.JoinResult.JOIN_RESULT_INVALID then
    result.debug = result.debug .. "INVALID"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_SUCCESS then
    result.debug = result.debug .. "SUCCESS"
    result.errorMsg = ""
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_CONNECT_TO_HOST_FAILURE then
    result.debug = result.debug .. "CONNECT_TO_HOST_FAILURE"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_PROBE_SEND_FAILURE then
    result.debug = result.debug .. "PROBE_SEND_FAILURE"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_PROBE_TIMEOUT then
    result.debug = result.debug .. "PROBE_TIMEOUT"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_PROBE_INVALID_LOBBY then
    result.debug = result.debug .. "PROBE_INVALID_LOBBY"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_PROBE_INVALID_INFO then
    result.debug = result.debug .. "PROBE_INVALID_INFO"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_PROBE_RESULT_INVALID then
    result.debug = result.debug .. "PROBE_RESULT_INVALID"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_INVALID_LOBBY then
    result.debug = result.debug .. "INVALID_LOBBY"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_SEND_AGREEMENT_REQUEST_FAILED then
    result.debug = result.debug .. "SEND_AGREEMENT_REQUEST_FAILED"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_JOIN_DISABLED then
    result.debug = result.debug .. "JOIN_DISABLED"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_NOT_JOINABLE_OFFLINE_MODE then
    result.debug = result.debug .. "HOST_OFFLINE_MODE"
    result.errorMsg = "MENU_JOIN_RESULT_HOST_OFFLINE_MODE"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_JOIN_ALREADY_IN_PROGRESS then
    result.debug = result.debug .. "JOIN_ALREADY_IN_PROGRESS"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_NOT_JOINABLE_CLOSED then
    result.debug = result.debug .. "NOT_JOINABLE_CLOSED"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE_CLOSED"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_NOT_JOINABLE_INVITE_ONLY then
    result.debug = result.debug .. "NOT_JOINABLE_INVITE_ONLY"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE_INVITE_ONLY"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_NOT_JOINABLE_FRIENDS_ONLY then
    result.debug = result.debug .. "NOT_JOINABLE_FRIENDS_ONLY"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE_FRIENDS_ONLY"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_NOT_JOINABLE_SOLO_MODE then
    result.debug = result.debug .. "NOT_JOINABLE_SOLO_MODE"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE_SOLO_MODE"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_OVER_MAX_PARTY_LIMIT then
    result.debug = result.debug .. "OVER_MAX_PARTY_LIMIT"
    result.errorMsg = "EXE_TO_MANY_LOCAL_PLAYERS"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_UNDER_MIN_PARTY_LIMIT then
    result.debug = result.debug .. "UNDER_MIN_PARTY_LIMIT"
    result.errorMsg = "PLATFORM_JOIN_RESULT_UNDER_MIN_PARTY_LIMIT"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_NO_PARTIES then
    result.debug = result.debug .. "NO_PARTIES"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE_ALREADY_IN_PROGRESS"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_LOBBY_FULL then
    result.debug = result.debug .. "LOBBY_FULL"
    result.errorMsg = "MENU_JOIN_RESULT_LOBBY_FULL"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_NETWORK_MODE_MISMATCH then
    result.debug = result.debug .. "NETWORK_MODE_MISMATCH"
    result.errorMsg = "MENU_JOIN_RESULT_NETWORK_MODE_MISMATCH"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_MISMATCH_PLAYLISTID then
    result.debug = result.debug .. "MISMATCH_PLAYLISTID"
    result.errorMsg = "MENU_JOIN_RESULT_MISMATCH_PLAYLISTID"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_MISMATCH_PLAYLIST_VERSION_TO_NEW then
    result.debug = result.debug .. "MISMATCH_PLAYLIST_VERSION_TO_NEW"
    result.errorMsg = "MENU_JOIN_RESULT_MISMATCH_PLAYLIST_VERSION_TO_NEW"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_MISMATCH_PLAYLIST_VERSION_TO_OLD then
    result.debug = result.debug .. "MISMATCH_PLAYLIST_VERSION_TO_OLD"
    result.errorMsg = "MENU_JOIN_RESULT_MISMATCH_PLAYLIST_VERSION_TO_OLD"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_MISMATCH_PROTOCOL_VERSION then
    result.debug = result.debug .. "MISMATCH_PROTOCOL_VERSION"
    result.errorMsg = "MENU_JOIN_RESULT_MISMATCH_PROTOCOL_VERSION"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_MISMATCH_NETFIELD_CHECKSUM then
    result.debug = result.debug .. "MISMATCH_NETFIELD_CHECKSUM"
    result.errorMsg = "MENU_JOIN_RESULT_MISMATCH_NETFIELD_CHECKSUM"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_MISMATCH_FFOTD_VERSION_TO_NEW then
    result.debug = result.debug .. "MISMATCH_FFOTD_VERSION_TO_NEW"
    result.errorMsg = "MENU_JOIN_RESULT_MISMATCH_PLAYLIST_VERSION_TO_NEW"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_MISMATCH_FFOTD_VERSION_TO_OLD then
    result.debug = result.debug .. "MISMATCH_FFOTD_VERSION_TO_OLD"
    result.errorMsg = "MENU_JOIN_RESULT_MISMATCH_PLAYLIST_VERSION_TO_OLD"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_MIGRATE_IN_PROGRESS then
    result.debug = result.debug .. "MIGRATE_IN_PROGRESS"
    result.errorMsg = "MENU_JOIN_RESULT_MIGRATE_IN_PROGRESS"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_COULD_NOT_RESERVE then
    result.debug = result.debug .. "COULD_NOT_RESERVE"
    result.errorMsg = "MENU_JOIN_RESULT_COULD_NOT_RESERVE"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_HANDSHAKE_WINDOW_EXPIRED then
    result.debug = result.debug .. "HANDSHAKE_WINDOW_EXPIRED"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_AGREEMENT_WINDOW_EXPIRED then
    result.debug = result.debug .. "AGREEMENT_WINDOW_EXPIRED"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_NOT_JOINABLE_NOT_IDLE then
    result.debug = result.debug .. "NOT_JOINABLE_NOT_IDLE"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_NO_JOIN_IN_PROGRESS then
    result.debug = result.debug .. "NO_JOIN_IN_PROGRESS"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_GAME_PAUSED then
    result.debug = result.debug .. "GAME_PAUSED"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE_PAUSED"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_CHUNK_MP_REQUIRED then
    result.debug = result.debug .. "CHUNK_MP_REQUIRED"
    result.errorMsg = "MENU_JOIN_RESULT_MP_REQUIRED"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_CHUNK_ZM_REQUIRED then
    result.debug = result.debug .. "CHUNK_ZM_REQUIRED"
    result.errorMsg = "MENU_JOIN_RESULT_ZM_REQUIRED"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_CHUNK_CP_REQUIRED then
    result.debug = result.debug .. "CHUNK_CP_REQUIRED"
    result.errorMsg = "MENU_JOIN_RESULT_CP_REQUIRED"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_CHUNK_MP_REQUIRED_HOST then
    result.debug = result.debug .. "CHUNK_MP_REQUIRED_HOST"
    result.errorMsg = "MENU_JOIN_RESULT_MP_REQUIRED_HOST"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_CHUNK_ZM_REQUIRED_HOST then
    result.debug = result.debug .. "CHUNK_ZM_REQUIRED_HOST"
    result.errorMsg = "MENU_JOIN_RESULT_ZM_REQUIRED_HOST"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_CHUNK_CP_REQUIRED_HOST then
    result.debug = result.debug .. "CHUNK_CP_REQUIRED_HOST"
    result.errorMsg = "MENU_JOIN_RESULT_CP_REQUIRED_HOST"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_SPLITSCREEN_NOT_ALLOWED then
    result.debug = result.debug .. "JOIN_RESULT_SPLITSCREEN_NOT_ALLOWED"
    result.errorMsg = "MENU_SPLITSCREEN_NOT_ALLOWED"
  elseif
      Engine.DvarBool(0, "probation_public_enabled")
      and joinResult == Enum.JoinResult.JOIN_RESULT_IN_PUBLIC_PROBATION
  then
    result.debug = result.debug .. "JOIN_RESULT_IN_PUBLIC_PROBATION"
    result.errorMsg = "MENU_JOIN_DENIED_PUBLIC_PROBATION"
  elseif
      Engine.DvarBool(0, "probation_league_enabled")
      and joinResult == Enum.JoinResult.JOIN_RESULT_IN_ARENA_PROBATION
  then
    result.debug = result.debug .. "JOIN_RESULT_IN_ARENA_PROBATION"
    result.errorMsg = "MENU_JOIN_DENIED_ARENA_PROBATION"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_BAD_DLC_BITS then
    result.debug = result.debug .. "JOIN_RESULT_BAD_DLC_BITS"
    if Engine.GetLobbyClientCount(Enum.LobbyType.LOBBY_TYPE_PRIVATE) > 1 then
      result.errorMsg = "PLATFORM_MEMBER_MISSING_MAP"
    else
      result.errorMsg = "PLATFORM_MISSINGMAP"
    end
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_BAD_MPHD_BITS then
    result.debug = result.debug .. "JOIN_RESULT_BAD_DLC_BITS"
    result.errorMsg = "PLATFORM_MISSINGBO4PREORDER"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_VM_FAILURE_1 then
    result.debug = result.debug .. "JOIN_RESULT_VM_FAILURE_1"
    result.errorMsg = "EXE_HOSTUNREACH_TOO_MANY_JOINS"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_STARTER_PACK_RESTRICT then
    result.debug = result.debug .. "JOIN_RESULT_STARTER_PACK_RESTRICT"
    if LuaUtils.IsStarterPack() then
      result.errorMsg = "PLATFORM_STARTER_PACK_JOIN_ERROR"
    else
      result.errorMsg = "PLATFORM_STARTER_PACK_MEMBER_MISSING_CONTENT"
    end
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_INVALID_PASSWORD then
    result.debug = result.debug .. "JOIN_RESULT_INVALID_PASSWORD"
    result.errorMsg = "GAME_INVALIDPASSWORD"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_BANNED then
    result.debug = result.debug .. "BANNED"
    result.errorMsg = "PLATFORM_BANNED_FROM_SERVER"
  elseif joinResult == Enum.JoinResult.JOIN_RESULT_BANNED_MEMBER then
    result.debug = result.debug .. "BANNED_MEMBER"
    result.errorMsg = "PLATFORM_BANNED_MEMBER_FROM_SERVER"
  else
    result.debug = result.debug .. "UNHANDLED JOINRESULT ENUM(" .. tostring(joinResult) .. ")"
    result.errorMsg = "MENU_JOIN_RESULT_NOT_JOINABLE"
  end
  return result
end

LobbyVM.ClearLobbyStatus = function()
  if LobbyVM.lobbyStatus.cleared == true then
    return
  elseif LobbyVM.lobbyStatus.clearedTime > Engine.milliseconds() then
    return
  else
    LobbyVM.lobbyStatus.cleared = true
    local lobbyRootModel = Engine.GetModel(Engine.GetGlobalModel(), "lobbyRoot")
    local statusString1Model = Engine.CreateModel(lobbyRootModel, "lobbyStatusString1")
    local statusString2Model = Engine.CreateModel(lobbyRootModel, "lobbyStatusString2")
    local statusString3Model = Engine.CreateModel(lobbyRootModel, "lobbyStatusString3")
    Engine.SetModelValue(statusString1Model, "")
    Engine.SetModelValue(statusString2Model, "")
    Engine.SetModelValue(statusString3Model, "")
  end
end

LobbyVM.LobbyStatusUpdate = function(searchStatus)
  if not Engine.IsMainThreadOrProxy() then
    return
  end
  local lobbyRootModel = Engine.GetModel(Engine.GetGlobalModel(), "lobbyRoot")
  local statusString1Model = Engine.CreateModel(lobbyRootModel, "lobbyStatusString1")
  local statusString2Model = Engine.CreateModel(lobbyRootModel, "lobbyStatusString2")
  local statusString3Model = Engine.CreateModel(lobbyRootModel, "lobbyStatusString3")
  LobbyVM.lobbyStatus.cleared = false
  LobbyVM.lobbyStatus.clearedTime = Engine.milliseconds() + 10000
  local searchStage = searchStatus.searchStage
  local statusText = ""
  if searchStage == 1 then
    local resultCount = searchStatus.numResults
    if resultCount == 1 then
      statusText = Engine.Localize("MENU_SESSIONS_SEARCH_STAGE_SINGLE")
    else
      statusText = Engine.Localize("MENU_SESSIONS_SEARCH_STAGE", resultCount)
    end
    Engine.SetModelValue(statusString1Model, statusText)
    Engine.SetModelValue(statusString2Model, "")
    Engine.SetModelValue(statusString3Model, "")
  elseif searchStage == 2 then
    Engine.SetModelValue(
      statusString2Model,
      Engine.Localize("MENU_SESSIONS_QOS_PROGRESS", searchStatus.contactedResults, searchStatus.numResults)
    )
  elseif searchStage == 3 then
    Engine.SetModelValue(
      statusString3Model,
      Engine.Localize("MENU_SESSIONS_JOINING", searchStatus.joiningCurHost, searchStatus.joiningNumHosts)
    )
  end
end

LobbyVM.CheckStarterPack = function(lobbyInfo)
  if lobbyInfo.isStarterPack == 1 then
    if LuaUtils.Mods_IsUsingMods() then
      return true
    end
    local currentMenuTarget = LobbyData.GetCurrentMenuTarget()
    if
        Engine.GetLobbyNetworkMode() ~= Enum.LobbyNetworkMode.LOBBY_NETWORKMODE_LIVE
        and currentMenuTarget.id ~= LobbyData.UITargets.UI_FRLOBBYLANGAME.id
    then
      return true
    elseif
        currentMenuTarget.mainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_CP
        or currentMenuTarget.mainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_ZM
    then
      return true
    elseif
        currentMenuTarget.lobbyType == Enum.LobbyType.LOBBY_TYPE_GAME
        and currentMenuTarget.lobbyMode == Enum.LobbyMode.LOBBY_MODE_CUSTOM
    then
      return true
    end
  end
  return false
end

LobbyVM.LoadMod = function(modInfo)
  local loadModProcess = Lobby.Process.LoadMod(Engine.GetPrimaryController(), modInfo)
  if loadModProcess ~= nil then
    Lobby.ProcessQueue.AddToQueue("LoadMod", loadModProcess)
  end
end

LobbyVM.OnModUpdate = function(modUpdate)
  if LuaUtils.Mods_IsModLoaded(modUpdate.ugcInfo) then
    return false
  elseif Lobby.ProcessQueue.IsQueueEmpty() then
    modUpdate.ugcInfo.waitForInstall = true
    local installProcess = Lobby.Process.NeedInstallUGC(Engine.GetPrimaryController(), modUpdate.ugcInfo)
    if installProcess ~= nil then
      Lobby.ProcessQueue.AddToQueue("NeedInstallMod", installProcess)
    end
  end
  return true
end

LobbyVM.OnUsermapUpdate = function(usermapInfo, waitForInstall)
  if not LuaUtils.Mods_IsUGC(usermapInfo.ugcInfo) then
    return false
  elseif LuaUtils.Mods_Lists_IsInstalled(usermapInfo.ugcInfo) then
    return false
  elseif waitForInstall ~= true and Engine.Mods_IsSubscribedItem(usermapInfo.ugcInfo.ugcName) then
    if Engine.Mods_InstalledUGC(usermapInfo.ugcInfo.ugcName) then
      Engine.Mods_Lists_UpdateUsermaps()
    end
    return true
  elseif Lobby.ProcessQueue.IsQueueEmpty() then
    usermapInfo.ugcInfo.waitForInstall = waitForInstall
    local installProcess = Lobby.Process.NeedInstallUGC(Engine.GetPrimaryController(), usermapInfo.ugcInfo)
    if installProcess ~= nil then
      Lobby.ProcessQueue.AddToQueue("NeedInstallUsermap", installProcess)
    end
  end
  return true
end

LobbyVM.DoChunksAllowJoin = function(lobbyInfo, mainMode)
  if mainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_MP then
    if lobbyInfo.chunkMP ~= 3 then
      return Enum.JoinResult.JOIN_RESULT_CHUNK_MP_REQUIRED
    elseif Engine.IsMpStillDownloading() then
      return Enum.JoinResult.JOIN_RESULT_CHUNK_MP_REQUIRED_HOST
    end
  elseif mainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_ZM then
    if lobbyInfo.chunkZM ~= 3 then
      return Enum.JoinResult.JOIN_RESULT_CHUNK_ZM_REQUIRED
    elseif Engine.IsZmStillDownloading() then
      return Enum.JoinResult.JOIN_RESULT_CHUNK_ZM_REQUIRED_HOST
    end
  elseif mainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_CP then
    if lobbyInfo.chunkCP ~= 3 then
      return Enum.JoinResult.JOIN_RESULT_CHUNK_CP_REQUIRED
    elseif Engine.IsCpStillDownloading() then
      return Enum.JoinResult.JOIN_RESULT_CHUNK_CP_REQUIRED_HOST
    end
  end
  return Enum.JoinResult.JOIN_RESULT_SUCCESS
end

LobbyVM.IsInTheaterLobby = function()
  local uiTarget = LobbyData:UITargetFromId(Engine.GetLobbyUIScreen())
  if uiTarget.lobbyMode == Enum.LobbyMode.LOBBY_MODE_THEATER then
    return true
  else
    return false
  end
end

LobbyVM.JoinableCheck = function(joinData)
  local joinRequest = joinData.joinRequest
  local joinResponse = joinData.joinResponse
  local currentMenuTarget = LobbyData.GetCurrentMenuTarget()
  if
      Engine.IsDedicatedServer() == false
      and Engine.IsProcessingJoin() == true
      and currentMenuTarget.lobbyType == Enum.LobbyType.LOBBY_TYPE_GAME
      and (
        currentMenuTarget.lobbyMode == Enum.LobbyMode.LOBBY_MODE_PUBLIC
        or currentMenuTarget.lobbyMode == Enum.LobbyMode.LOBBY_MODE_ARENA
      )
  then
    if
        currentMenuTarget.mainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_CP
        and Engine.DvarBool(nil, "cpProcessingJoinCheck")
    then
      return Enum.JoinResult.JOIN_RESULT_VM_FAILURE_1
    elseif
        currentMenuTarget.mainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_MP
        and Engine.DvarBool(nil, "mpProcessingJoinCheck")
    then
      return Enum.JoinResult.JOIN_RESULT_VM_FAILURE_1
    elseif
        currentMenuTarget.mainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_ZM
        and Engine.DvarBool(nil, "zmProcessingJoinCheck")
    then
      return Enum.JoinResult.JOIN_RESULT_VM_FAILURE_1
    end
  end
  if
      Engine.IsInGame()
      or LobbyVM.IsHostLaunching()
      or Engine.GetCurrentMap() ~= "core_frontend"
      or Engine.SessionMode_IsPublicOnlineGame()
  then
    local sessionClients =
        Engine.LobbyGetSessionClients(Enum.LobbyModule.LOBBY_MODULE_HOST, Enum.LobbyType.LOBBY_TYPE_GAME)
    local neededDLCBits = LobbyVM.GetNeededDLCBits()
    Engine.PrintInfo(
      Enum.consoleLabel.LABEL_LOBBY,
      "Incoming client dlc bits are "
      .. joinRequest.dlcBits
      .. " while our needed bits are "
      .. neededDLCBits
      .. "\n"
    )
    if Engine.IsZombiesGame() then
      joinRequest.dlcBits = LuaUtils.UpdateZMDLCBits(joinRequest.dlcBits)
    end
    if not LobbyVM.CheckDLCBit(joinRequest.dlcBits, neededDLCBits) then
      Engine.PrintWarning(Enum.consoleLabel.LABEL_LOBBY, "Rejecting client due to incompatible dlc bits.\n")
      if LobbyVM.CheckDLCBit(neededDLCBits, Enum.ContentFlagBits.CONTENT_DLC6) then
        return Enum.JoinResult.JOIN_RESULT_BAD_MPHD_BITS
      else
        return Enum.JoinResult.JOIN_RESULT_BAD_DLC_BITS
      end
    end
  end
  if joinResponse.response == Enum.JoinResult.JOIN_RESULT_SUCCESS then
    if Engine.IsMultiplayerGame() then
      if LuaUtils.IsArenaMode() then
        if
            (
              (Engine.GetGametypeSetting("pregameItemVoteEnabled") == 1)
              or Engine.GetGametypeSetting("pregameDraftEnabled") == 1
            )
            and Engine.IsInGame()
            and Engine.SessionMode_IsPublicOnlineGame()
        then
          return Enum.JoinResult.JOIN_RESULT_NO_JOIN_IN_PROGRESS
        elseif
            joinData.joinRequest.splitscreenClients ~= nil and joinData.joinRequest.splitscreenClients > 0
        then
          return Enum.JoinResult.JOIN_RESULT_SPLITSCREEN_NOT_ALLOWED
        elseif Lobby.Timer.LobbyIsLocked() then
          return Enum.JoinResult.JOIN_RESULT_NO_JOIN_IN_PROGRESS
        elseif Engine.GetLobbyPregameState() ~= Enum.LobbyPregameState.LOBBY_PREGAME_STATE_IDLE then
          return Enum.JoinResult.JOIN_RESULT_NO_JOIN_IN_PROGRESS
        end
        local timerStatus = Lobby.Timer.GetTimerStatus()
        if
            timerStatus == Lobby.Timer.LOBBY_STATUS.POST_GAME
            or timerStatus == Lobby.Timer.LOBBY_STATUS.FIND_NEW_LOBBY
        then
          return Enum.JoinResult.JOIN_RESULT_NO_JOIN_IN_PROGRESS
        elseif
            Engine.DvarBool(0, "probation_league_enabled")
            and currentMenuTarget.lobbyType == Enum.LobbyType.LOBBY_TYPE_GAME
        then
          for memberIndex, member in pairs(joinRequest.members) do
            if member.arenaProbation > 0 then
              return Enum.JoinResult.JOIN_RESULT_IN_ARENA_PROBATION
            end
          end
        end
      elseif
          Engine.DvarBool(0, "probation_public_enabled")
          and currentMenuTarget.lobbyType == Enum.LobbyType.LOBBY_TYPE_GAME
      then
        for memberIndex, member in pairs(joinRequest.members) do
          if member.publicProbation > 0 then
            return Enum.JoinResult.JOIN_RESULT_IN_PUBLIC_PROBATION
          end
        end
      end
    end
    if Engine.IsZombiesGame() then
      local privateRankedMatch = false
      if Dvar.zm_private_rankedmatch:get() then
        privateRankedMatch = true
      end
      if LobbyVM.IsInTheaterLobby() then
        return Enum.JoinResult.JOIN_RESULT_JOIN_DISABLED
      elseif
          LobbyData.UITargets.UI_ZMLOBBYONLINE.id == Engine.GetLobbyUIScreen()
          and (Engine.IsInGame() or LobbyVM.IsHostLaunching() or Engine.GetCurrentMap() ~= "core_frontend")
      then
        return Enum.JoinResult.JOIN_RESULT_NOT_JOINABLE_SOLO_MODE
      elseif
          (Engine.IsInGame() or LobbyVM.IsHostLaunching())
          and Engine.SessionMode_IsOnlineGame()
          and privateRankedMatch
      then
        return Enum.JoinResult.JOIN_RESULT_NO_JOIN_IN_PROGRESS
      elseif (Engine.IsInGame() or LobbyVM.IsHostLaunching()) and Engine.SessionMode_IsPublicOnlineGame() then
        return Enum.JoinResult.JOIN_RESULT_NO_JOIN_IN_PROGRESS
      end
    end
    if LuaUtils.isPC and LobbyVM.CheckStarterPack(joinRequest) then
      return Enum.JoinResult.JOIN_RESULT_STARTER_PACK_RESTRICT
    elseif LuaUtils.isPC then
      if not joinData.isLocalRequest and Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_GAME) then
        local serverPassword = Engine.LiveSteamServer_GetServerPassword()
        if
            serverPassword
            and string.len(serverPassword) > 0
            and joinData.joinRequest.password ~= serverPassword
        then
          return Enum.JoinResult.JOIN_RESULT_INVALID_PASSWORD
        end
      end
    end
    local mainMode = Engine.GetLobbyMainMode()
    if Engine.GetLobbyNetworkMode() == Enum.LobbyNetworkMode.LOBBY_NETWORKMODE_LIVE then
      if not joinData.isLocalRequest then
        if joinResponse.lobbyType == Enum.LobbyType.LOBBY_TYPE_GAME then
          local chunkJoinResult = LobbyVM.DoChunksAllowJoin(joinRequest, mainMode)
          if chunkJoinResult ~= Enum.JoinResult.JOIN_RESULT_SUCCESS then
            return chunkJoinResult
          end
        elseif joinResponse.lobbyType == Enum.LobbyType.LOBBY_TYPE_PRIVATE then
          if LuaUtils.isPC then
            local chunkJoinResult = LobbyVM.DoChunksAllowJoin(joinRequest, mainMode)
            if chunkJoinResult ~= Enum.JoinResult.JOIN_RESULT_SUCCESS then
              return chunkJoinResult
            end
          end
          for mainModeIndex = 0, Enum.LobbyMainMode.LOBBY_MAINMODE_COUNT - 1, 1 do
            local mainModeChunkJoinResult = LobbyVM.DoChunksAllowJoin(joinRequest, mainModeIndex)
            if mainModeChunkJoinResult ~= Enum.JoinResult.JOIN_RESULT_SUCCESS then
              return mainModeChunkJoinResult
            end
          end
        end
      end
    elseif not joinData.isLocalRequest then
      local chunkJoinResult = LobbyVM.DoChunksAllowJoin(joinRequest, mainMode)
      if chunkJoinResult ~= Enum.JoinResult.JOIN_RESULT_SUCCESS then
        return chunkJoinResult
      end
    end
  end
  return joinResponse.response
end

LobbyVM.ProcessCompleteSuccess = function(processResult)
  Lobby.ProcessQueue.Success(processResult)
end

LobbyVM.ProcessCompleteFailure = function(processResult)
  Lobby.ProcessQueue.Failure(processResult)
end

LobbyVM.ProcessCompleteError = function(processResult)
  Lobby.ProcessQueue.Error(processResult)
end

LobbyVM.ProcessUpdate = function(processResult)
  Lobby.ProcessQueue.Update(processResult)
end

LobbyVM.GameLobbyGameServerDataUpdate = function(serverData)
  if Lobby.ProcessQueue.IsQueueEmpty() == true then
    local lobbyUIScreen = Engine.GetLobbyUIScreen()
    if
        (
          lobbyUIScreen == LobbyData.UITargets.UI_ZMLOBBYONLINEPUBLICGAME.id
          or lobbyUIScreen == LobbyData.UITargets.UI_MPLOBBYONLINEPUBLICGAME.id
          or lobbyUIScreen == LobbyData.UITargets.UI_MPLOBBYONLINEARENAGAME.id
        ) and LobbyVM.DLCMapCheck(serverData) == false
    then
      return
    end
  end
  local playlistID = Engine.GetPlaylistID()
  local playlistName = Engine.GetPlaylistName(0, playlistID)
  local lobbyPlaylistModel = Engine.CreateModel(Engine.GetGlobalModel(), "lobbyPlaylist")
  Engine.SetModelValue(Engine.CreateModel(lobbyPlaylistModel, "name"), Engine.ToUpper(playlistName))
  local kickerText = ""
  local playlistInfo = Engine.GetPlaylistInfoByID(playlistID)
  if
      playlistInfo
      and playlistInfo.playlist
      and playlistInfo.playlist.category
      and playlistInfo.playlist.category == Engine.GetPlaylistCategoryIdByName("hardcore")
  then
    kickerText = "MENU_RULES_HARDCORE"
  end
  Engine.SetModelValue(Engine.CreateModel(lobbyPlaylistModel, "kickerText"), kickerText)
  Lobby.MapVote.GameLobbyGameServerDataUpdate(serverData)
end

LobbyVM.PrivateLobbyServerDataUpdate = function(serverData)
  local playlistID = Engine.GetPlaylistID()
  local playlistName = Engine.GetPlaylistName(0, playlistID)
  local lobbyPlaylistModel = Engine.CreateModel(Engine.GetGlobalModel(), "lobbyPlaylist")
  Engine.SetModelValue(Engine.CreateModel(lobbyPlaylistModel, "name"), Engine.ToUpper(playlistName))
  local kickerText = ""
  local playlistInfo = Engine.GetPlaylistInfoByID(playlistID)
  if
      playlistInfo
      and playlistInfo.playlist
      and playlistInfo.playlist.category
      and playlistInfo.playlist.category == Engine.GetPlaylistCategoryIdByName("hardcore")
  then
    kickerText = "MENU_RULES_HARDCORE"
  end
  Engine.SetModelValue(Engine.CreateModel(lobbyPlaylistModel, "kickerText"), kickerText)
end

LobbyVM.GameLobbyClientDataUpdate = function(clientData)
  Lobby.MapVote.GameLobbyClientDataUpdate(clientData)
  Lobby.TeamSelection.GameLobbyClientDataUpdate(clientData)
  Lobby.Pregame.GameLobbyClientDataUpdate(clientData)
  LuaUtils.ForceLobbyButtonUpdate()
  Engine.ForceNotifyModelSubscriptions(Engine.CreateModel(Engine.GetGlobalModel(), "lobbyRoot.gameClientDataUpdate"))
end

LobbyVM.HopperClientJoin = function(clientJoinData)
  return Lobby.Hopper.ClientJoin(clientJoinData)
end

LobbyVM.HopperIsParked = function()
  return Lobby.Hopper.IsParked()
end

LobbyVM.GetMatchmakingExperimentActive = function(experiment)
  return Lobby.Matchmaking.IsExperimentActive(experiment)
end

LobbyVM.OnExperimentReset = function(experimentData)
  Lobby.Matchmaking.ResetExperiment(experimentData.controller)
end

LobbyVM.OnDediQosReady = function()
  local qosResults = Engine.GetDediQosResultsByType(Lobby.Matchmaking.DatacenterType.ANY)
  if qosResults.numResults == 0 then
    Engine.PrintWarning(
      Enum.consoleLabel.LABEL_LOBBY,
      "Engine told us results were ready, but no results available\n"
    )
  else
    servers = qosResults.pingResults
    for serverIndex, server in ipairs(servers) do
      LuaUtils.LogQoS("dediqos", server)
    end
  end
end

LobbyVM.PopulateMutableClientDDLBuff = function(ddlBuffData)
  local controller = ddlBuffData.controller
  local ddlData = ddlBuffData.ddlData
end

LobbyVM.ReceiveMutableClientDDLBuff = function(ddlBuffData)
  local xuid = ddlBuffData.xuid
  local ddlData = ddlBuffData.ddlData
end

LobbyVM.LobbyPumpList = {}
LobbyVM.LobbyPumpList.sequence = 0
LobbyVM.LobbyPumpList.funcs = {
  LobbyVM.ClearLobbyStatus,
  Lobby.Anticheat.Pump,
  Lobby.Debug.Pump,
  Lobby.Hopper.Pump,
  Lobby.MapVote.Pump,
  Lobby.Matchmaking.Pump,
  Lobby.Merge.Pump,
  Lobby.Platform.Pump,
  Lobby.PartyPrivacy.Pump,
  Lobby.Pregame.Pump,
  Lobby.ProcessQueue.Pump,
  Lobby.TeamSelection.Pump,
  Lobby.Theater.Pump,
  Lobby.Timer.Pump,
  Lobby.Events.Pump,
}
LobbyVM.Pump = function()
  LobbyVM.LobbyPumpList.sequence = math.fmod(LobbyVM.LobbyPumpList.sequence, #LobbyVM.LobbyPumpList.funcs)
  LobbyVM.LobbyPumpList.sequence = LobbyVM.LobbyPumpList.sequence + 1
  LobbyVM.LobbyPumpList.funcs[LobbyVM.LobbyPumpList.sequence]()
  if LobbyVM.AutoJoin.leaveServerImmediately == true then
    LobbyVM.AutoJoin.leaveServerImmediately = false
    if not LobbyVM.IsInTheaterLobby() then
      Engine.LeaveServerImmediately()
    end
  end
end

LobbyVM.OnEnableJoins = function(joinState)
  if joinState.enable then
    Engine.SetSessionStatus(Enum.LobbyType.LOBBY_TYPE_GAME, Enum.SessionStatus.SESSION_STATUS_IDLE)
  else
    Engine.SetSessionStatus(Enum.LobbyType.LOBBY_TYPE_GAME, Enum.SessionStatus.SESSION_STATUS_PAUSED)
  end
end

LobbyVM.OnStorageOperationReadResult = function(readResult)
  Lobby.Anticheat.OnStorageOperationReadResult(readResult)
end

LobbyVM.OnIsFeatureBanned = function(featureBanData)
  local isBanned, banInfo =
      Lobby.Anticheat.CheckIsFeatureBannedForIndex(featureBanData.controller, featureBanData.feature)
  return isBanned
end

LobbyVM.OnPlayerBanned = function(banData)
  Lobby.Anticheat.OnPlayerBanned(banData)
end

LobbyVM.OnFeatureBan = function(banData)
  Lobby.Anticheat.OnFeatureBan(banData)
end

LobbyVM.OnCheckPrestigeFeatureBan = function(banData)
  Lobby.Anticheat.OnCheckPrestigeFeatureBan(banData)
end

LobbyVM.OnIsPermaBanned = function(banData)
  return Lobby.Anticheat.OnIsPermaBanned(banData)
end

LobbyVM.OnGetBanTimeRemaining = function(banData)
  return Lobby.Anticheat.OnGetBanTimeRemaining(banData)
end

LobbyVM.OnPushAnticheatMessageToUI = function(messageData)
  return Lobby.Anticheat.OnPushAnticheatMessageToUI(messageData)
end

LobbyVM.OnPopAnticheatMessage = function(messageData)
  Lobby.Anticheat.OnPopAnticheatMessage(messageData)
end

LobbyVM.OnGetAnticheatReputation = function(reputationData)
  return Lobby.Anticheat.OnGetAnticheatReputation(reputationData)
end

LobbyVM.GetLootItemCategory = function(itemId)
  local lootTable = "gamedata/loot/mplootitems.csv"
  local itemIdColumn = 1
  local categoryColumn = 2
  local rows = Engine.TableFindRows(lootTable, itemIdColumn, itemId)
  if rows == nil or #rows == 0 then
    return -1
  else
    local category = Engine.TableLookupGetColumnValueForRow(lootTable, rows[1], categoryColumn)
    if category == nil then
      return -1
    else
      return category
    end
  end
end

LobbyVM.GetRecentItemTags = function(controller, itemCategory)
  local lootItems = Engine.GetLootItems(
    controller,
    Enum.eModes.MODE_MULTIPLAYER,
    0,
    Engine.GetLootItemCount(controller, Enum.eModes.MODE_MULTIPLAYER)
  )
  local lootVersion = LuaUtils.GetCurrentLootVersion()
  local lootTable = "gamedata/loot/mplootitems.csv"
  local rows = Engine.TableFindRows(lootTable, 2, itemCategory)
  local versionColumn = 6
  local possibleCount = 0
  for _, row in ipairs(rows) do
    local itemVersion = Engine.TableLookupGetColumnValueForRow(lootTable, row, versionColumn)
    if itemVersion == nil or itemVersion == "" then
      itemVersion = -1
    else
      itemVersion = tonumber(itemVersion)
    end
    if itemVersion ~= -1 and itemVersion <= lootVersion then
      possibleCount = possibleCount + 1
    end
  end
  if lootItems == nil then
    return ""
  end
  recentItems = {}
  for _, lootItem in ipairs(lootItems) do
    if
        CoDShared.GetLootItemCategory(lootItem.id) == itemCategory
        and CoDShared.GetLootItemVersion(lootItem.id) <= lootVersion
    then
      table.insert(recentItems, lootItem.id)
    end
    if possibleCount <= #recentItems then
      return ""
    end
  end
  if #recentItems == 0 then
    return ""
  end
  tagList = ""
  for index, itemId in ipairs(recentItems) do
    tagList = tagList .. "[ 203, " .. itemId .. "]"
    if index < #recentItems then
      tagList = tagList .. " , "
    end
  end
  return tagList
end

LobbyVM.GetLootPossibleCount = function(itemCategory)
  local lootVersion = LuaUtils.GetCurrentLootVersion()
  local lootTable = "gamedata/loot/mplootitems.csv"
  local rows = Engine.TableFindRows(lootTable, 2, itemCategory)
  local versionColumn = 6
  local possibleCount = 0
  for _, row in ipairs(rows) do
    local itemVersion = tonumber(Engine.TableLookupGetColumnValueForRow(lootTable, row, versionColumn)) or -1
    if itemVersion ~= -1 and itemVersion <= lootVersion then
      possibleCount = possibleCount + 1
    end
  end
  return possibleCount
end

LobbyVM.GetLootItemList = function(controller, itemCategory)
  local lootVersion = LuaUtils.GetCurrentLootVersion()
  local lootItems = Engine.GetLootItems(
    controller,
    Enum.eModes.MODE_MULTIPLAYER,
    0,
    Engine.GetLootItemCount(controller, Enum.eModes.MODE_MULTIPLAYER)
  )
  local itemList = {}
  if lootItems == nil then
    return itemList
  end
  for _, lootItem in ipairs(lootItems) do
    if
        CoDShared.GetLootItemCategory(lootItem.id) == itemCategory
        and CoDShared.GetLootItemVersion(lootItem.id) <= lootVersion
    then
      table.insert(itemList, lootItem.id)
    end
  end
  return itemList
end

LobbyVM.OnBuyCrate = function(crateData)
  if crateData.crateDWID == 48 then
    local possibleCount = LobbyVM.GetLootPossibleCount("melee_weapon")
    local itemList = LobbyVM.GetLootItemList(crateData.controller, "melee_weapon")
    if possibleCount ~= nil and itemList ~= nil and possibleCount - 1 <= #itemList then
      crateData.crateDWID = 50
    end
  end
  local supplyDropId = ' "SupplyDropID": ' .. crateData.crateDWID .. ", "
  local purchaseWith = ' "PurchaseWith": "' .. crateData.currency .. '", '
  local rank = ' "Rank": [], '
  local excludeTag = ' "ExcludeTag": [], '
  local inventoryVersion = ' "InventoryVersion": [ '
      .. Dvar.loot_mpItemVersions:get()
      .. Dvar.loot_limitedTimeItemVersions:get()
      .. " ] "
  if Engine.DvarString(nil, "loot_useClientDupes") == "1" then
    local useExcludeTag = false
    local excludeCategory = ""
    if crateData.crateDWID == 32 then
      useExcludeTag = true
      excludeCategory = "weapon"
    elseif crateData.crateDWID == 31 then
      useExcludeTag = true
      excludeCategory = "melee_weapon"
    end
    if useExcludeTag == true then
      local recentItemTags = LobbyVM.GetRecentItemTags(crateData.controller, excludeCategory)
      excludeTag = ' "ExcludeTag": [ '
      if recentItemTags ~= "" then
        excludeTag = excludeTag .. recentItemTags
      end
      excludeTag = excludeTag .. " ], "
      Engine.PrintInfo(Enum.consoleLabel.LABEL_DEFAULT, "Excluding: " .. excludeTag .. ".\n")
    end
  end
  return supplyDropId .. purchaseWith .. rank .. excludeTag .. inventoryVersion
end

LobbyVM.DLCInfo = {
  {
    dlcIndex = Enum.dlcIndex_t.CONTENT_DLC1_INDEX,
    dlcPackName = "dlc1",
    inventoryVersion = 101,
  },
  {
    dlcIndex = Enum.dlcIndex_t.CONTENT_DLC2_INDEX,
    dlcPackName = "dlc2",
    inventoryVersion = 102,
  },
  {
    dlcIndex = Enum.dlcIndex_t.CONTENT_DLC3_INDEX,
    dlcPackName = "dlc3",
    inventoryVersion = 103,
  },
  {
    dlcIndex = Enum.dlcIndex_t.CONTENT_DLC4_INDEX,
    dlcPackName = "dlc4",
    inventoryVersion = 104,
  },
  {
    dlcIndex = Enum.dlcIndex_t.CONTENT_DLC5_INDEX,
    dlcPackName = "dlc5",
    inventoryVersion = 105,
  },
}
LobbyVM.OnSpendVials = function(vialData)
  local numVials = ' "NumVials": ' .. vialData.vialCount .. ", "
  local inventoryVersion = ' "InventoryVersion": [' .. Dvar.loot_zmItemVersions:get() .. " "
  local highestAvailableDLC = Engine.DvarInt(nil, "tu9_highestAvailableDLC")
  for _, dlcInfo in ipairs(LobbyVM.DLCInfo) do
    if
        dlcInfo.dlcIndex <= highestAvailableDLC
        or dlcInfo.dlcPackName == "dlc5"
        and (Dvar.loot_forceEnableZCGumsInFactory:get() == 1 or Dvar.loot_forceEnableZCGumsInFactory:get() == "1")
    then
      inventoryVersion = inventoryVersion .. ", " .. dlcInfo.inventoryVersion .. " "
    end
  end
  return numVials .. inventoryVersion .. "]"
end

LobbyVM.OnCookGobbleGumRecipe = function(recipeData)
  return ' "Recipe": ' .. recipeData.recipeIndex + recipeData.itemOffset
end

LobbyVM.OnInventoryFetched = function(inventoryData)
  local teamMatrixItems = {
    "na_team_mtx",
    "eu_team_mtx",
    "anz_team_mtx",
    "cwl_mtx",
    "cwl_mtx_v2",
  }
  local inventoryTable = "gamedata/tables/common/inventory_items.csv"
  local nameColumn = 1
  local quantityColumn = 2
  local hasTeamMatrix = false
  for _, itemName in ipairs(teamMatrixItems) do
    if
        Engine.GetInventoryItemQuantity(
          inventoryData.controller,
          tonumber(Engine.TableLookup(nil, inventoryTable, nameColumn, itemName, quantityColumn))
        ) > 0
    then
      Engine.SetProfileVar(inventoryData.controller, itemName, "1")
      hasTeamMatrix = true
    end
  end
  if hasTeamMatrix then
    Engine.CommitProfileChanges(inventoryData.controller)
  end
end

LobbyVM.CheckSpecialPlaylistRules = function(controller)
  if Dvar.partyChatDisallowed:get() == true then
    if Engine.IsLocalClientInPlatformPartyChat() and not Engine.IsInComError() then
      Dvar.partyChatDisallowed:set(false)
      Engine.ComError(Enum.errorCode.ERROR_DROP, "PLATFORM_PARTYCHATNOTALLOWED")
    end
    local sessionClients =
        Engine.LobbyGetSessionClients(Enum.LobbyModule.LOBBY_MODULE_HOST, Enum.LobbyType.LOBBY_TYPE_GAME)
    for _, client in ipairs(sessionClients.sessionClients) do
      if client.isInPlatformPartyChat == true then
        Engine.KickClient(
          controller,
          Enum.LobbyType.LOBBY_TYPE_GAME,
          client.xuid,
          Enum.LobbyDisconnectClient.LOBBY_DISCONNECT_CLIENT_NOPARTYCHAT,
          "PLATFORM_PARTYCHATNOTALLOWED"
        )
      end
    end
  end
end

LobbyVM.LobbyMonitor = function()
  if not Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_GAME) or Engine.IsDedicatedServer() or Engine.IsInGame() then
    return
  elseif Engine.GetLobbyUIScreen() ~= LobbyData.UITargets.UI_MPLOBBYONLINEPUBLICGAME.id then
    return
  elseif Engine.IsLobbyActive(Enum.LobbyType.LOBBY_TYPE_GAME) == false then
    return
  elseif false == Dvar.tu16_lobbyMonitor:exists() or Dvar.tu16_lobbyMonitor:get() == 0 then
    return
  end
  local monitorMode = Dvar.tu16_lobbyMonitor:get()
  if monitorMode == 0 then
    return
  end
  local clientCount = Engine.GetLobbyClientCount(Enum.LobbyType.LOBBY_TYPE_GAME)
  local maxClients = Engine.GetLobbyMaxClients(Enum.LobbyType.LOBBY_TYPE_GAME)
  local maxPlayers = Dvar.party_maxplayers:get()
  if maxClients == maxPlayers or clientCount <= maxPlayers then
    return
  end
  local primaryController = Engine.GetPrimaryController()
  if false == LobbyVM.lobbyMonitor.eventRecorded then
    LobbyVM.lobbyMonitor.eventRecorded = true
    Engine.RecordComScoreEvent(
      primaryController,
      "lobby_monitor",
      "game_client_count",
      clientCount,
      "game_max_clients",
      maxClients,
      "party_max_players",
      maxPlayers
    )
  end
  if monitorMode == 1 then
  elseif monitorMode == 2 then
    Engine.SetSessionStatus(Enum.LobbyType.LOBBY_TYPE_PRIVATE, Enum.SessionStatus.SESSION_STATUS_ERROR)
    local sessionClients =
        Engine.LobbyGetSessionClients(Enum.LobbyModule.LOBBY_MODULE_HOST, Enum.LobbyType.LOBBY_TYPE_GAME)
    for _, client in ipairs(sessionClients.sessionClients) do
      if client.isHost == false and client.isLocal == false then
        Engine.KickClient(
          primaryController,
          Enum.LobbyType.LOBBY_TYPE_GAME,
          client.xuid,
          Enum.LobbyDisconnectClient.LOBBY_DISCONNECT_CLIENT_ERROR,
          "LobbyVM.LobbyMonitor() found an issue with the lobby"
        )
      end
    end
    Engine.ComError(Enum.errorCode.ERROR_DROP, "EXE_DISCONNECTED_FROM_SERVER_ERROR")
  end
end

LobbyVM.IngameMonitor = function()
  if not Engine.IsInGame() then
    return
  elseif
      (Engine.IsDedicatedServer() or CoDShared.IsLobbyMode(Enum.LobbyMode.LOBBY_MODE_PUBLIC))
      and Dvar.partyChatDisallowed:get()
      and Engine.IsLobbyHostActive(Enum.LobbyType.LOBBY_TYPE_GAME)
  then
    local sessionClients =
        Engine.LobbyGetSessionClients(Enum.LobbyModule.LOBBY_MODULE_HOST, Enum.LobbyType.LOBBY_TYPE_GAME)
    local hostControllerIndex = Engine.GetLobbyHostControllerIndex(Enum.LobbyType.LOBBY_TYPE_GAME)
    for _, client in ipairs(sessionClients.sessionClients) do
      if client.isInPlatformPartyChat == true then
        Engine.KickClient(
          hostControllerIndex,
          Enum.LobbyType.LOBBY_TYPE_GAME,
          client.xuid,
          Enum.LobbyDisconnectClient.LOBBY_DISCONNECT_CLIENT_NOPARTYCHAT,
          "PLATFORM_PARTYCHATNOTALLOWED"
        )
      end
    end
  end
end
