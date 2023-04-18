
-- Removed check for public matches to allow team change in ranked matches
CoD.IsTeamChangeAllowed = function()
	if Engine.GetGametypeSetting( "allowInGameTeamChange" ) == 1 then
		return true
	else
		return false
	end
end

DataSources.StartMenuGameOptions = ListHelper_SetupDataSource("StartMenuGameOptions", function (controller)
	local options = {}
	if Engine.IsDemoPlaying() then
		if not IsDemoRestrictedBasicMode() then
			table.insert(options, {models = {displayText = Engine.ToUpper(Engine.Localize("MENU_UPLOAD_CLIP", Engine.GetDemoSegmentCount())), action = StartMenuUploadClip, disabledFunction = IsUploadClipButtonDisabled}, properties = {hideHelpItemLabel = true}})
		end
		if Engine.IsDemoHighlightReelMode() then
			table.insert(options, {models = {displayText = Engine.ToUpper(Engine.Localize("MENU_DEMO_CUSTOMIZE_HIGHLIGHT_REEL")), action = StartMenuOpenCustomizeHighlightReel, disabledFunction = IsCustomizeHighlightReelButtonDisabled}})
		end
		table.insert(options, {models = {displayText = Engine.ToUpper(Engine.ToUpper(Engine.Localize("MENU_JUMP_TO_START"))), action = StartMenuJumpToStart, disabledFunction = IsJumpToStartButtonDisabled}, properties = {hideHelpItemLabel = true}})
		local endDemoButtonText = nil
		if Engine.IsDemoClipPlaying() then
			endDemoButtonText = Engine.ToUpper(Engine.Localize("MENU_END_CLIP"))
		else
			endDemoButtonText = Engine.ToUpper(Engine.Localize("MENU_END_FILM"))
		end
		table.insert(options, {models = {displayText = Engine.ToUpper(endDemoButtonText), action = StartMenuEndDemo}})
	elseif CoD.isCampaign then
		table.insert(options, {models = {displayText = "MENU_RESUMEGAME_CAPS", action = StartMenuGoBack_ListElement}})
		local inTrainingSim = CoD.SafeGetModelValue(Engine.GetModelForController(controller), "safehouse.inTrainingSim")
		if not inTrainingSim then
			inTrainingSim = 0
		end
		if Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_GAME) then
			if not CoD.isSafehouse and controller == Engine.GetPrimaryController() then
				table.insert(options, {models = {displayText = "MENU_RESTART_MISSION_CAPS", action = RestartMission}})
				if LUI.DEV ~= nil then
					table.insert(options, {models = {displayText = "MENU_RESTART_CHECKPOINT_CAPS", action = RestartFromCheckpoint}})
				end
			end
			if controller == Engine.GetPrimaryController() then
				table.insert(options, {models = {displayText = "MENU_CHANGE_DIFFICULTY_CAPS", action = OpenDifficultySelect}})
			end
			if CoD.isSafehouse and inTrainingSim == 1 then
				table.insert(options, {models = {displayText = "MENU_END_TRAINING_SIM", action = EndTrainingSim}})
			elseif controller == Engine.GetPrimaryController() then
				if Engine.DvarBool(0, "ui_blocksaves") then
					table.insert(options, {models = {displayText = "MENU_EXIT_CAPS", action = SaveAndQuitGame}})
				else
					table.insert(options, {models = {displayText = "MENU_SAVE_AND_QUIT_CAPS", action = SaveAndQuitGame}})
				end
			end
		elseif CoD.isSafehouse and inTrainingSim == 1 then
			table.insert(options, {models = {displayText = "MENU_END_TRAINING_SIM", action = EndTrainingSim}})
		else
			table.insert(options, {models = {displayText = "MENU_LEAVE_PARTY_AND_EXIT_CAPS", action = QuitGame}})
		end
	elseif CoD.isMultiplayer then
		if Engine.Team(controller, "name") ~= "TEAM_SPECTATOR" and Engine.GetGametypeSetting("disableClassSelection") ~= 1 then
			table.insert(options, {models = {displayText = "MPUI_CHOOSE_CLASS_BUTTON_CAPS", action = ChooseClass}})
		end
		if not Engine.IsVisibilityBitSet(controller, Enum.UIVisibilityBit.BIT_ROUND_END_KILLCAM) and not Engine.IsVisibilityBitSet(controller, Enum.UIVisibilityBit.BIT_FINAL_KILLCAM) and CoD.IsTeamChangeAllowed() then
			table.insert(options, {models = {displayText = "MPUI_CHANGE_TEAM_BUTTON_CAPS", action = ChooseTeam}})
		end
		if controller == 0 then
			local endGameText = "MENU_QUIT_GAME_CAPS"
			if Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_GAME) and not CoD.isOnlineGame() then
				endGameText = "MENU_END_GAME_CAPS"
			end
			table.insert(options, {models = {displayText = endGameText, action = QuitGame_MP}})
		end
	elseif CoD.isZombie then
		table.insert(options, {models = {displayText = "MENU_RESUMEGAME_CAPS", action = StartMenuGoBack_ListElement}})
		if Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_GAME) and (not Engine.SessionModeIsMode(CoD.SESSIONMODE_SYSTEMLINK) or Engine.SessionModeIsMode(CoD.SESSIONMODE_OFFLINE)) then
			table.insert(options, {models = {displayText = "MENU_RESTART_LEVEL_CAPS", action = RestartGame}})
		end
		if Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_GAME) == true then
			table.insert(options, {models = {displayText = "MENU_END_GAME_CAPS", action = QuitGame_MP}})
		else
			table.insert(options, {models = {displayText = "MENU_QUIT_GAME_CAPS", action = QuitGame_MP}})
		end
	end
	return options
end, true)
