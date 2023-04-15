if not CoD.playlistMatchSettingsInfo then
	return
end

local oldPlaylistMatchSettingsInfo = CoD.playlistMatchSettingsInfo.new
function CoD.playlistMatchSettingsInfo.new(menu, controller)
	local self = oldPlaylistMatchSettingsInfo(menu, controller)

	-- Hide the playlist count text
	if self.playlistCount then
		self.playlistCount:setAlpha(0)
	end

	return self
end
