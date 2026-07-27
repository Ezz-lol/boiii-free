local function addFriendsAccessOption(options, controller)
  local connectedToDedicated = false
  pcall(function()
    connectedToDedicated = Dvar.cl_connected_to_dedi and Dvar.cl_connected_to_dedi:get()
  end)

  if connectedToDedicated or not Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_GAME) then
    return
  end

  local friendsCanJoin = Engine.DvarBool(controller, "friends_open")
  table.insert(options, {
    models = {
      displayText = friendsCanJoin and "CLOSE FRIEND JOINING" or "OPEN TO FRIENDS",
      action = function(self, element, actionController, param, menu)
        Engine.Exec(actionController, "friends_open")
        StartMenuGoBack_ListElement(self, element, actionController, param, menu)
      end,
    },
  })
end

local function restartGame(self, element, controller, param, menu)
  Engine.Exec(controller, "boiii_prepare_menu_restart")
  return RestartGame(self, element, controller, param, menu)
end

DataSources.StartMenuGameOptions = ListHelper_SetupDataSource("StartMenuGameOptions", function(controller)
  local options = {}
  local currentMode = Engine.CurrentSessionMode()
  local isMP = currentMode == Enum.eModes.MODE_MULTIPLAYER
  local isZM = currentMode == Enum.eModes.MODE_ZOMBIES
  local isCP = currentMode == Enum.eModes.MODE_CAMPAIGN
  local endGameOption = nil
  local restartGameOption = nil
  local connectedToDedicated = false
  pcall(function()
    connectedToDedicated = Dvar.cl_connected_to_dedi and Dvar.cl_connected_to_dedi:get()
  end)
  local isLocalHost = Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_GAME) and not connectedToDedicated
  if Engine.IsDemoPlaying() then
    if not IsDemoRestrictedBasicMode() then
      table.insert(options, {
        models = {
          displayText = Engine.ToUpper(Engine.Localize("MENU_UPLOAD_CLIP", Engine.GetDemoSegmentCount())),
          action = StartMenuUploadClip,
          disabledFunction = IsUploadClipButtonDisabled,
        },
        properties = { hideHelpItemLabel = true },
      })
    end
    if Engine.IsDemoHighlightReelMode() then
      table.insert(options, {
        models = {
          displayText = Engine.ToUpper(Engine.Localize("MENU_DEMO_CUSTOMIZE_HIGHLIGHT_REEL")),
          action = StartMenuOpenCustomizeHighlightReel,
          disabledFunction = IsCustomizeHighlightReelButtonDisabled,
        },
      })
    end
    table.insert(options, {
      models = {
        displayText = Engine.ToUpper(Engine.ToUpper(Engine.Localize("MENU_JUMP_TO_START"))),
        action = StartMenuJumpToStart,
        disabledFunction = IsJumpToStartButtonDisabled,
      },
      properties = { hideHelpItemLabel = true },
    })
    local endDemoButtonText = nil
    if Engine.IsDemoClipPlaying() then
      endDemoButtonText = Engine.ToUpper(Engine.Localize("MENU_END_CLIP"))
    else
      endDemoButtonText = Engine.ToUpper(Engine.Localize("MENU_END_FILM"))
    end
    table.insert(options, { models = { displayText = Engine.ToUpper(endDemoButtonText), action = StartMenuEndDemo } })
  elseif isCP then
    table.insert(options, { models = { displayText = "MENU_RESUMEGAME_CAPS", action = StartMenuGoBack_ListElement } })
    local inTrainingSim = CoD.SafeGetModelValue(Engine.GetModelForController(controller), "safehouse.inTrainingSim")
    if not inTrainingSim then
      inTrainingSim = 0
    end
    if Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_GAME) then
      if not CoD.isSafehouse and controller == Engine.GetPrimaryController() then
        table.insert(options, { models = { displayText = "MENU_RESTART_MISSION_CAPS", action = RestartMission } })
        if LUI.DEV ~= nil then
          table.insert(
            options,
            { models = { displayText = "MENU_RESTART_CHECKPOINT_CAPS", action = RestartFromCheckpoint } }
          )
        end
      end
      if controller == Engine.GetPrimaryController() then
        table.insert(
          options,
          { models = { displayText = "MENU_CHANGE_DIFFICULTY_CAPS", action = OpenDifficultySelect } }
        )
      end
      if CoD.isSafehouse and inTrainingSim == 1 then
        table.insert(options, { models = { displayText = "MENU_END_TRAINING_SIM", action = EndTrainingSim } })
      elseif controller == Engine.GetPrimaryController() then
        if Engine.DvarBool(0, "ui_blocksaves") then
          table.insert(options, { models = { displayText = "MENU_EXIT_CAPS", action = SaveAndQuitGame } })
        else
          table.insert(options, { models = { displayText = "MENU_SAVE_AND_QUIT_CAPS", action = SaveAndQuitGame } })
        end
      end
    elseif CoD.isSafehouse and inTrainingSim == 1 then
      table.insert(options, { models = { displayText = "MENU_END_TRAINING_SIM", action = EndTrainingSim } })
    else
      table.insert(options, { models = { displayText = "MENU_LEAVE_PARTY_AND_EXIT_CAPS", action = QuitGame } })
    end
  elseif isMP then
    if
      Engine.Team(controller, "name") ~= "TEAM_SPECTATOR" and Engine.GetGametypeSetting("disableClassSelection") ~= 1
    then
      table.insert(options, { models = { displayText = "MPUI_CHOOSE_CLASS_BUTTON_CAPS", action = ChooseClass } })
    end
    if
      not Engine.IsVisibilityBitSet(controller, Enum.UIVisibilityBit.BIT_ROUND_END_KILLCAM)
      and not Engine.IsVisibilityBitSet(controller, Enum.UIVisibilityBit.BIT_FINAL_KILLCAM)
      and CoD.IsTeamChangeAllowed()
    then
      table.insert(options, { models = { displayText = "MPUI_CHANGE_TEAM_BUTTON_CAPS", action = ChooseTeam } })
    end
    local endGameText = isLocalHost and "MAIN MENU" or "MENU_QUIT_GAME_CAPS"
    endGameOption = { models = { displayText = endGameText, action = QuitGame_MP } }
    if isLocalHost then
      restartGameOption = {
        models = {
          displayText = "RESTART GAME",
          action = restartGame,
        },
      }
    end
  elseif isZM then
    table.insert(options, { models = { displayText = "MENU_RESUMEGAME_CAPS", action = StartMenuGoBack_ListElement } })
    local endGameText = isLocalHost and "MAIN MENU" or "MENU_QUIT_GAME_CAPS"
    endGameOption = { models = { displayText = endGameText, action = QuitGame_MP } }
    if isLocalHost then
      restartGameOption = {
        models = {
          displayText = "RESTART GAME",
          action = restartGame,
        },
      }
    end
  end
  addFriendsAccessOption(options, controller)
  local isLocalGame = true
  pcall(function()
    if Dvar.cl_connected_to_dedi and Dvar.cl_connected_to_dedi:get() then
      isLocalGame = false
    end
  end)
  if isLocalGame then
    table.insert(options, {
      models = {
        displayText = "GAME TWEAKS",
        action = function(self, element, controller, param, menu)
          if menu and menu.openPopup then
            menu:openPopup("BoiiiGameTweaksMenu", controller)
          else
            OpenPopup(self, "BoiiiGameTweaksMenu", controller)
          end
        end,
      },
    })
  end
  if restartGameOption then
    table.insert(options, restartGameOption)
  end
  if endGameOption then
    table.insert(options, endGameOption)
  end
  table.insert(options, { models = { displayText = "QUIT TO DESKTOP", action = OpenPCQuit } })
  return options
end, true)
