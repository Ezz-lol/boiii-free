local getModeInfo = function()
	local id = Engine.GetLobbyUIScreen()
	return LobbyData:UITargetFromId(id)
end

local getMaxClients = function()
	local modeInfo = getModeInfo()
	return modeInfo.maxClients
end

-- Disable setting party privacy in menu. Auto set to open + max.
Engine.SetDvar("partyprivacyenabled", 0)
Engine.SetDvar("tu4_partyprivacyuseglobal", 0)
Engine.SetDvar("tu4_partyprivacyluacheck", 0)

local maxClients = getMaxClients()

-- Fix for invisible bots in custom games
if maxClients >= 1 then
	return Engine.SetDvar("party_maxplayers", maxClients)
end

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

-- Set com_maxclients InGame so players can join via direct connect (default from lobbydata)
Engine.SetDvar("com_maxclients", maxClients)

require("datasources_start_menu_game_options")
