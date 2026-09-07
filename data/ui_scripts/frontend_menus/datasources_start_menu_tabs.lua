DataSources.StartMenuTabs = ListHelper_SetupDataSource("StartMenuTabs", function(controllerIndex)
  local tabs = {}
  table.insert(tabs, {
    models = {
      tabIcon = CoD.buttonStrings.shoulderl,
    },
    properties = {
      m_mouseDisabled = true,
    },
  })
  if Engine.IsDemoPlaying() then
    local gameOptionsWidget = "CoD.StartMenu_GameOptions"
    if Engine.IsZombiesGame() then
      gameOptionsWidget = "CoD.StartMenu_GameOptions_ZM"
    end
    table.insert(tabs, {
      models = {
        tabName = Engine.Localize("MENU_THEATER_CAPS"),
        tabWidget = gameOptionsWidget,
        tabIcon = "",
      },
      properties = {
        tabId = "gameOptions",
      },
    })
  elseif Engine.IsInGame() then
    if IsGameTypeDOA() and not InSafehouse() then
      table.insert(tabs, {
        models = {
          tabName = "DOA",
          tabWidget = "CoD.StartMenu_GameOptions_DOA",
          tabIcon = "",
        },
        properties = {
          tabId = "gameOptions",
        },
      })
    elseif CoD.isCampaign then
      table.insert(tabs, {
        models = {
          tabName = SessionModeToUnlocalizedSessionModeCaps(Engine.CurrentSessionMode()),
          tabWidget = "CoD.StartMenu_GameOptions_CP",
          tabIcon = "",
        },
        properties = {
          tabId = "gameOptions",
        },
      })
      if not Engine.IsCampaignModeZombies() then
        if
          CoD.isSafehouse
          and CoD.isOnlineGame()
          and not IsInTrainingSim(controllerIndex)
          and Dvar.ui_safehousebarracks:get()
          and not IsPlayerAGuest(controllerIndex)
        then
          table.insert(tabs, {
            models = {
              tabName = "CPUI_BARRACKS_CAPS",
              tabWidget = "CoD.CombatRecordCP_Contents",
              tabIcon = "",
            },
            properties = {
              tabId = "combatRecord",
            },
          })
        end
        if HighestMapReachedGreaterThan(controllerIndex, 1) or LUI.DEV ~= nil then
          table.insert(tabs, {
            models = {
              tabName = "CPUI_TACTICAL_MODE_CAPS",
              tabWidget = "CoD.StartMenu_TacticalMode",
              tabIcon = "",
            },
            properties = {
              tabId = "tacticalMode",
            },
          })
        end
        if not CoD.isSafehouse and not IsPlayerAGuest(controllerIndex) then
          table.insert(tabs, {
            models = {
              tabName = "CPUI_ACCOLADES",
              tabWidget = "CoD.MissionRecordVault_Challenges",
              tabIcon = "",
            },
            properties = {
              tabId = "accolades",
            },
          })
        end
      end
    elseif Engine.IsZombiesGame() then
      table.insert(tabs, {
        models = {
          tabName = SessionModeToUnlocalizedSessionModeCaps(Engine.CurrentSessionMode()),
          tabWidget = "CoD.StartMenu_GameOptions_ZM",
          tabIcon = "",
        },
        properties = {
          tabId = "gameOptions",
        },
      })
    else
      table.insert(tabs, {
        models = {
          tabName = SessionModeToUnlocalizedSessionModeCaps(Engine.CurrentSessionMode()),
          tabWidget = "CoD.StartMenu_GameOptions",
          tabIcon = "",
        },
        properties = {
          tabId = "gameOptions",
        },
      })
    end
  else
    if not IsPlayerAGuest(controllerIndex) then
      table.insert(tabs, {
        models = {
          tabName = "MENU_TAB_IDENTITY_CAPS",
          tabWidget = "CoD.StartMenu_Identity",
          tabIcon = "",
        },
        properties = {
          tabId = "identity",
          disabled = Dvar.ui_execdemo_gamescom:get(),
        },
      })
    end
    if
      not IsLobbyNetworkModeLAN()
      and not Dvar.ui_execdemo:get()
      and not Engine.IsCampaignModeZombies()
      and not IsPlayerAGuest(controllerIndex)
    then
      table.insert(tabs, {
        models = {
          tabName = "MENU_TAB_CHALLENGES_CAPS",
          tabWidget = "CoD.StartMenu_Challenges",
          tabIcon = "",
        },
        properties = {
          tabId = "challenges",
        },
      })
      local disableBarracks = CoD.isPC
      if disableBarracks then
        disableBarracks = false --Mods_IsUsingMods()
      end
      table.insert(tabs, {
        models = {
          tabName = "MENU_TAB_BARRACKS_CAPS",
          tabWidget = "CoD.StartMenu_Barracks",
          tabIcon = "",
          disabled = disableBarracks,
        },
        properties = {
          tabId = "barracks",
        },
      })
      if CommunityOptionsEnabled() then
        local openMediaTab = CoD.perController[controllerIndex].openMediaTabAfterClosingGroups
        CoD.perController[controllerIndex].openMediaTabAfterClosingGroups = false
        table.insert(tabs, {
          models = {
            tabName = "MENU_TAB_MEDIA_CAPS",
            tabWidget = "CoD.StartMenu_Media",
            tabIcon = "",
          },
          properties = {
            tabId = "media",
            selectIndex = openMediaTab,
          },
        })
      end
    end
  end
  if IsGameTypeDOA() and Engine.IsInGame() and not InSafehouse() then
    local tableInsert = table.insert
    local tabsRef = tabs
    local optionsTab = {
      models = {
        tabName = "MENU_TAB_OPTIONS_CAPS",
        tabWidget = "CoD.StartMenu_Options_DOA",
        tabIcon = "",
      },
    }
    local optionsProperties = {
      tabId = "options",
    }
    local isDemoSelected = Dvar.ui_execdemo:get()
    if isDemoSelected then
      isDemoSelected = not Engine.IsInGame()
    end
    optionsProperties.selectIndex = isDemoSelected
    optionsTab.properties = optionsProperties
    tableInsert(tabsRef, optionsTab)
  else
    local tableInsert = table.insert
    local tabsRef = tabs
    local optionsTab = {
      models = {
        tabName = "MENU_TAB_OPTIONS_CAPS",
        tabWidget = "CoD.StartMenu_Options",
        tabIcon = "",
      },
    }
    local optionsProperties = {
      tabId = "options",
    }
    local isGamescomDemo = Dvar.ui_execdemo_gamescom:get()
    if isGamescomDemo then
      isGamescomDemo = not Engine.IsInGame()
    end
    optionsProperties.selectIndex = isGamescomDemo
    optionsTab.properties = optionsProperties
    tableInsert(tabsRef, optionsTab)
  end
  table.insert(tabs, {
    models = {
      tabIcon = CoD.buttonStrings.shoulderr,
    },
    properties = {
      m_mouseDisabled = true,
    },
  })
  return tabs
end, true)
