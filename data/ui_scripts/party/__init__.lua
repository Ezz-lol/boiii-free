if not Engine.IsInGame() then
	return
end

-- Removed check for public matches to allow team change in ranked matches
CoD.IsTeamChangeAllowed = function()
	if Engine.GetGametypeSetting( "allowInGameTeamChange" ) == 1 then
		return true
	else
		return false
	end
end

local GetModeInfo = function()
	local id = Engine.GetLobbyUIScreen()
	return  LobbyData:UITargetFromId(id)
end

local GetMaxClients = function()
	local ModeInfo = GetModeInfo()
	return ModeInfo.maxClients
end

-- Set com_maxclients InGame so players can join via direct connect (default from lobbydata)
Engine.SetDvar( "com_maxclients", GetMaxClients() )

require("DataSources_StartMenuGameOptions")
