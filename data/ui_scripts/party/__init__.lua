if not Engine.IsInGame() then
	return
end

-- Removed check for public matches to allow team change in ranked matches
CoD.IsTeamChangeAllowed = function()
	if Engine.GetGametypeSetting("allowInGameTeamChange") == 1 then
		return true
	else
		return false
	end
end

local getModeInfo = function()
	local id = Engine.GetLobbyUIScreen()
	return LobbyData:UITargetFromId(id)
end

local getMaxClients = function()
	local modeInfo = getModeInfo()
	return modeInfo.maxClients
end

-- Set com_maxclients InGame so players can join via direct connect (default from lobbydata)
Engine.SetDvar("com_maxclients", getMaxClients())

require("datasources_start_menu_game_options")
