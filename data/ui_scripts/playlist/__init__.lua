if Engine.GetCurrentMap() ~= "core_frontend" then
	return
end

if CoD.LobbyMember then
	local oldLobbyMember = CoD.LobbyMember.new
	function CoD.LobbyMember.new(menu, controller)
		local self = oldLobbyMember(menu, controller)
	
		-- Hide the playlist count text
		if self.SearchingForPlayer then
			self.SearchingForPlayer:setAlpha(0)
		end
		if self.FEMemberBlurPanelContainer0 then
			self.FEMemberBlurPanelContainer0:setAlpha(0)
		end
	
		return self
	end
end

function IsLobbyStatusVisible()
	return false
end

Engine.SetDvar("lobbyMigrate_Enabled", 0)
Engine.SetDvar("lobbyTimerStatusVotingInterval", 11000)
Engine.SetDvar("lobbyTimerStatusBeginInterval", 10)
Engine.SetDvar("lobbyTimerStatusStartInterval", 10)
Engine.SetDvar("lobbyTimerStatusPostGameInterval", 10)
Engine.SetDvar("lobbyTimerStatusVotingInterval_Arena", 11000)

require("widget_playlist_match_settings_info")
require("widget_playlist_category_match_settings_info")
