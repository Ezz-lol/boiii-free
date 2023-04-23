DataSources.StartMenuTabs = ListHelper_SetupDataSource("StartMenuTabs", function(f44_arg0)
	local f44_local0 = {}
	table.insert(f44_local0, {
		models = {
			tabIcon = CoD.buttonStrings.shoulderl
		},
		properties = {
			m_mouseDisabled = true
		}
	})
	if Engine.IsDemoPlaying() then
		local f44_local1 = "CoD.StartMenu_GameOptions"
		if Engine.IsZombiesGame() then
			f44_local1 = "CoD.StartMenu_GameOptions_ZM"
		end
		table.insert(f44_local0, {
			models = {
				tabName = Engine.Localize("MENU_THEATER_CAPS"),
				tabWidget = f44_local1,
				tabIcon = ""
			},
			properties = {
				tabId = "gameOptions"
			}
		})
	elseif Engine.IsInGame() then
		if IsGameTypeDOA() and not InSafehouse() then
			table.insert(f44_local0, {
				models = {
					tabName = "DOA",
					tabWidget = "CoD.StartMenu_GameOptions_DOA",
					tabIcon = ""
				},
				properties = {
					tabId = "gameOptions"
				}
			})
		elseif CoD.isCampaign then
			table.insert(f44_local0, {
				models = {
					tabName = SessionModeToUnlocalizedSessionModeCaps(Engine.CurrentSessionMode()),
					tabWidget = "CoD.StartMenu_GameOptions_CP",
					tabIcon = ""
				},
				properties = {
					tabId = "gameOptions"
				}
			})
			if not Engine.IsCampaignModeZombies() then
				if CoD.isSafehouse and CoD.isOnlineGame() and not IsInTrainingSim(f44_arg0) and Dvar.ui_safehousebarracks:get() and not IsPlayerAGuest(f44_arg0) then
					table.insert(f44_local0, {
						models = {
							tabName = "CPUI_BARRACKS_CAPS",
							tabWidget = "CoD.CombatRecordCP_Contents",
							tabIcon = ""
						},
						properties = {
							tabId = "combatRecord"
						}
					})
				end
				if HighestMapReachedGreaterThan(f44_arg0, 1) or LUI.DEV ~= nil then
					table.insert(f44_local0, {
						models = {
							tabName = "CPUI_TACTICAL_MODE_CAPS",
							tabWidget = "CoD.StartMenu_TacticalMode",
							tabIcon = ""
						},
						properties = {
							tabId = "tacticalMode"
						}
					})
				end
				if not CoD.isSafehouse and not IsPlayerAGuest(f44_arg0) then
					table.insert(f44_local0, {
						models = {
							tabName = "CPUI_ACCOLADES",
							tabWidget = "CoD.MissionRecordVault_Challenges",
							tabIcon = ""
						},
						properties = {
							tabId = "accolades"
						}
					})
				end
			end
		elseif Engine.IsZombiesGame() then
			table.insert(f44_local0, {
				models = {
					tabName = SessionModeToUnlocalizedSessionModeCaps(Engine.CurrentSessionMode()),
					tabWidget = "CoD.StartMenu_GameOptions_ZM",
					tabIcon = ""
				},
				properties = {
					tabId = "gameOptions"
				}
			})
		else
			table.insert(f44_local0, {
				models = {
					tabName = SessionModeToUnlocalizedSessionModeCaps(Engine.CurrentSessionMode()),
					tabWidget = "CoD.StartMenu_GameOptions",
					tabIcon = ""
				},
				properties = {
					tabId = "gameOptions"
				}
			})
		end
	else
		if not IsPlayerAGuest(f44_arg0) then
			table.insert(f44_local0, {
				models = {
					tabName = "MENU_TAB_IDENTITY_CAPS",
					tabWidget = "CoD.StartMenu_Identity",
					tabIcon = ""
				},
				properties = {
					tabId = "identity",
					disabled = Dvar.ui_execdemo_gamescom:get()
				}
			})
		end
		if not IsLobbyNetworkModeLAN() and not Dvar.ui_execdemo:get() and not Engine.IsCampaignModeZombies() and not IsPlayerAGuest(f44_arg0) then
			table.insert(f44_local0, {
				models = {
					tabName = "MENU_TAB_CHALLENGES_CAPS",
					tabWidget = "CoD.StartMenu_Challenges",
					tabIcon = ""
				},
				properties = {
					tabId = "challenges"
				}
			})
			local f44_local1 = CoD.isPC
			if f44_local1 then
				f44_local1 = false --Mods_IsUsingMods()
			end
			table.insert(f44_local0, {
				models = {
					tabName = "MENU_TAB_BARRACKS_CAPS",
					tabWidget = "CoD.StartMenu_Barracks",
					tabIcon = "",
					disabled = f44_local1
				},
				properties = {
					tabId = "barracks"
				}
			})
			if CommunityOptionsEnabled() then
				local f44_local2 = CoD.perController[f44_arg0].openMediaTabAfterClosingGroups
				CoD.perController[f44_arg0].openMediaTabAfterClosingGroups = false
				table.insert(f44_local0, {
					models = {
						tabName = "MENU_TAB_MEDIA_CAPS",
						tabWidget = "CoD.StartMenu_Media",
						tabIcon = ""
					},
					properties = {
						tabId = "media",
						selectIndex = f44_local2
					}
				})
			end
		end
	end
	if IsGameTypeDOA() and Engine.IsInGame() and not InSafehouse() then
		local f44_local1 = table.insert
		local f44_local2 = f44_local0
		local f44_local3 = {
			models = {
				tabName = "MENU_TAB_OPTIONS_CAPS",
				tabWidget = "CoD.StartMenu_Options_DOA",
				tabIcon = ""
			}
		}
		local f44_local4 = {
			tabId = "options"
		}
		local f44_local5 = Dvar.ui_execdemo:get()
		if f44_local5 then
			f44_local5 = not Engine.IsInGame()
		end
		f44_local4.selectIndex = f44_local5
		f44_local3.properties = f44_local4
		f44_local1(f44_local2, f44_local3)
	else
		local f44_local1 = table.insert
		local f44_local2 = f44_local0
		local f44_local3 = {
			models = {
				tabName = "MENU_TAB_OPTIONS_CAPS",
				tabWidget = "CoD.StartMenu_Options",
				tabIcon = ""
			}
		}
		local f44_local4 = {
			tabId = "options"
		}
		local f44_local5 = Dvar.ui_execdemo_gamescom:get()
		if f44_local5 then
			f44_local5 = not Engine.IsInGame()
		end
		f44_local4.selectIndex = f44_local5
		f44_local3.properties = f44_local4
		f44_local1(f44_local2, f44_local3)
	end
	table.insert(f44_local0, {
		models = {
			tabIcon = CoD.buttonStrings.shoulderr
		},
		properties = {
			m_mouseDisabled = true
		}
	})
	return f44_local0
end, true)
