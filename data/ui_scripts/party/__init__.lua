
-- Removed check for public matches to allow team change in ranked matches
CoD.IsTeamChangeAllowed = function()
	if Engine.GetGametypeSetting( "allowInGameTeamChange" ) == 1 then
		return true
	else
		return false
	end
end