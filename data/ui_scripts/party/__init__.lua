-- Removed check for public matches to allow team change in ranked matches
CoD.IsTeamChangeAllowed = function()
	if Engine.GetGametypeSetting( "allowInGameTeamChange" ) == 1 then
		return true
	else
		return false
	end
end

-- Set com_maxclients InGame so players can join via direct connect
local maxclients = function()
	if Engine.IsZombiesGame() then
		return 4
	else
		return 18
	end
end

Engine.SetDvar( "com_maxclients", maxclients() )

require("DataSources_StartMenuGameOptions")
