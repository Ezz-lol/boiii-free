if not CoD.playlistCategoryMatchSettingsInfo then
	return
end

local oldPlaylistCategoryMatchSettingsInfo = CoD.playlistCategoryMatchSettingsInfo.new
function CoD.playlistCategoryMatchSettingsInfo.new(menu, controller)
	local self = oldPlaylistCategoryMatchSettingsInfo(menu, controller)

	-- Hide the playlist count text
	if self.playlistCount then
		self.playlistCount:setAlpha(0)
	end

	return self
end
