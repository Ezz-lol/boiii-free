if Engine.GetCurrentMap() ~= "core_frontend" then
	return
end

DataSources.MPStatsSettings = DataSourceHelpers.ListSetup( "MPStatsSettings", function ( controller )
  local optionsTable = {}

	local updateDvar = function(f1_arg0, f1_arg1, f1_arg2, dvarName, f1_arg4)
    local oldValue = Engine.DvarInt( nil, dvarName )
    local newValue = f1_arg1.value
	  UpdateInfoModels( f1_arg1 )
    if oldValue == newValue then
      return 
    end
    Engine.SetDvar( dvarName, f1_arg1.value )
		if dvarName == "cg_unlockall_loot" then
    	Engine.SetDvar( "ui_enableAllHeroes", f1_arg1.value )
		end
    end

    table.insert( optionsTable, CoD.OptionsUtility.CreateDvarSettings( controller, "Unlock All Loot", "Whether loot should be locked based on the player's stats or always unlocked.", "MPStatsSettings_unlock_loot", "cg_unlockall_loot", {
		{
			option = "MENU_DISABLED",
			value = 0,
			default = true
		},
		{
			option = "MENU_ENABLED",
			value = 1
		},
	}, nil, updateDvar ))
	if Engine.CurrentSessionMode() == Enum.eModes.MODE_MULTIPLAYER then
		table.insert( optionsTable, CoD.OptionsUtility.CreateDvarSettings( controller, "Unlock All Purchases", "All items that need to be purchased with unlock tokens are unlocked.", "MPStatsSettings_purchase_all", "cg_unlockall_purchases", {
			{
				option = "MENU_DISABLED",
				value = 0,
				default = true
			},
			{
				option = "MENU_ENABLED",
				value = 1
			},
		}, nil, updateDvar ))
	end 
    table.insert( optionsTable, CoD.OptionsUtility.CreateDvarSettings( controller, "Unlock All Attachments", "All attachments on weapons are unlocked.", "MPStatsSettings_unlockall_attachments", "cg_unlockall_attachments", {
		{
			option = "MENU_DISABLED",
			value = 0,
			default = true
		},
		{
			option = "MENU_ENABLED",
			value = 1
		},
	}, nil, updateDvar ))
  table.insert( optionsTable, CoD.OptionsUtility.CreateDvarSettings( controller, "Unlock all Camos and Reticles", "All camos and reticles on weapons are unlocked.", "MPStatsSettings_unlockall_camos_and_reticles", "cg_unlockall_camos_and_reticles", {
		{
			option = "MENU_DISABLED",
			value = 0,
			default = true
		},
		{
			option = "MENU_ENABLED",
			value = 1
		},
	}, nil, updateDvar ))
  table.insert( optionsTable, CoD.OptionsUtility.CreateDvarSettings( controller, "Unlock all Emblems and Backings", "All emblems and backings are unlocked.", "MPStatsSettings_unlockall_emblems_and_backings", "cg_unlockall_emblems_and_backings", {
		{
			option = "MENU_DISABLED",
			value = 0,
			default = true
		},
		{
			option = "MENU_ENABLED",
			value = 1
		},
	}, nil, updateDvar ))

	local rankLevels = {}
	if Engine.CurrentSessionMode() == Enum.eModes.MODE_MULTIPLAYER then
		rankLevels = { 1, 10, 20, 30, 40, 50, 55 }
	elseif Engine.CurrentSessionMode() == Enum.eModes.MODE_ZOMBIES then
		rankLevels = { 1, 10, 20, 30, 35 }
	end
	local rankObjs = {}
	local hasDefault = false
	local currentRank = CoD.BlackMarketUtility.GetCurrentRank( controller ) + 1
	for index, value in ipairs(rankLevels) do
		table.insert( rankObjs, {
			name = value,
			value = value - 1,
			default = value == currentRank,
			title = "Rank Level",
			desc = ""
		})
		if not hasDefault then
			hasDefault = value == currentRank
		end
	end

	if not hasDefault then
		table.insert( rankObjs, {
			name = currentRank,
			value = currentRank - 1,
			default = true,
			title = "Rank Level",
			desc = ""
		})
	end

	local prestigeTable = {}
	for i = 0, 10 do
		table.insert( prestigeTable, {
			name = i == 0 and "None" or i,
			value = i,
			default = i == CoD.PrestigeUtility.GetCurrentPLevel( controller ),
			title = "Prestige",
			desc = ""
		})
	end

	local createSettingsDatasource = function ( controller, datasourceName, optionsTable, currentValue, loopEdges, action )
		if currentValue == nil then
			currentValue = 0
		end
		DataSources[datasourceName] = DataSourceHelpers.ListSetup( datasourceName, function ( f47_arg0 )
			local f47_local0 = {}
			for f47_local4, f47_local5 in ipairs( optionsTable ) do
				table.insert( f47_local0, {
					models = {
						text = optionsTable[f47_local4].name
					},
					properties = {
						title = optionsTable[f47_local4].title,
						desc = optionsTable[f47_local4].desc,
						image = optionsTable[f47_local4].image,
						value = optionsTable[f47_local4].value,
						default = optionsTable[f47_local4].default,
						action = action,
						selectIndex = optionsTable[f47_local4].value == currentValue,
						loopEdges = loopEdges,
						showChangeIndicator = function ( f48_arg0, f48_arg1, f48_arg2 )
							return f48_arg0.default ~= true
						end
					}
				} )
			end
			f47_local0[1].properties.first = true
			f47_local0[#optionsTable].properties.last = true
			return f47_local0
		end, nil, nil, nil )
		return datasourceName
	end
	
	table.insert( optionsTable, {
		models = {
			name = "Rank Level",
			desc = "",
			image = nil,
			optionsDatasource = createSettingsDatasource( controller, "MPStatsSettings_rank_level", rankObjs, CoD.BlackMarketUtility.GetCurrentRank( controller ), false, function(f1_arg0, f1_arg1, f1_arg2, dvarName, f1_arg4)
				UpdateInfoModels( f1_arg1 )
				local rankTable = nil
				if Engine.CurrentSessionMode() == Enum.eModes.MODE_MULTIPLAYER then
					rankTable = "gamedata/tables/mp/mp_ranktable.csv"
				elseif Engine.CurrentSessionMode() == Enum.eModes.MODE_ZOMBIES then
					rankTable = "gamedata/tables/zm/zm_ranktable.csv"
				end
				local skipLines = Engine.CurrentSessionMode() == Enum.eModes.MODE_MULTIPLAYER and 3 or 2
				local maxXp = tonumber(Engine.TableLookupGetColumnValueForRow(rankTable, f1_arg1.value + skipLines, 7))
				if maxXp == nil then
					maxXp = 9999999999
				end
				Engine.ExecNow(f1_arg0, "statsetbyname rankxp " .. maxXp - 1)
				Engine.ExecNow(f1_arg0, "statsetbyname rank " .. f1_arg1.value)
				Engine.Exec( f1_arg0, "uploadstats " .. tostring( Engine.CurrentSessionMode() ) )
			end )
		},
		properties = {
			revert = function ( f50_arg0 ) end
		}
	})

	table.insert( optionsTable, {
		models = {
			name = "Prestige",
			desc = "",
			image = nil,
			optionsDatasource = createSettingsDatasource( controller, "MPStatsSettings_rank_prestige", prestigeTable, CoD.PrestigeUtility.GetCurrentPLevel( controller ), false, function(f1_arg0, f1_arg1, f1_arg2, dvarName, f1_arg4)
				UpdateInfoModels( f1_arg1 )
				local newPrestige = f1_arg1.value
				Engine.ExecNow(f1_arg0, "statsetbyname plevel " .. newPrestige)
				Engine.ExecNow(f1_arg0, "statsetbyname hasprestiged " .. (newPrestige > 0 and 1 or 0))
				Engine.Exec( f1_arg0, "uploadstats " .. tostring( Engine.CurrentSessionMode() ) )
			end )
		},
		properties = {
			revert = function ( f50_arg0 ) end
		}
	})
  
    return optionsTable
end)

if Dvar.cg_unlockall_loot:get() == true then
	Engine.SetDvar( "ui_enableAllHeroes", 1 )
end

LUI.createMenu.BoiiiStatsMenu = function ( controller )
	local self = CoD.Menu.NewForUIEditor( "BoiiiStatsMenu" )
	if PreLoadFunc then
		PreLoadFunc( self, controller )
	end
	self.soundSet = "ChooseDecal"
	self:setOwner( controller )
	self:setLeftRight( true, true, 0, 0 )
	self:setTopBottom( true, true, 0, 0 )
	self:playSound( "menu_open", controller )
	self.buttonModel = Engine.CreateModel( Engine.GetModelForController( controller ), "BoiiiStatsMenu.buttonPrompts" )
	self.anyChildUsesUpdateState = true

    local GameSettingsBackground = CoD.GameSettings_Background.new( self, controller )
	GameSettingsBackground:setLeftRight( true, true, 0, 0 )
	GameSettingsBackground:setTopBottom( true, true, 0, 0 )
	GameSettingsBackground.MenuFrame.titleLabel:setText( Engine.Localize( "STATS SETTINGS" )  )
	GameSettingsBackground.MenuFrame.cac3dTitleIntermediary0.FE3dTitleContainer0.MenuTitle.TextBox1.Label0:setText( Engine.Localize( "STATS SETTINGS" ) )
	GameSettingsBackground.GameSettingsSelectedItemInfo.GameModeInfo:setAlpha( 0 )
	GameSettingsBackground.GameSettingsSelectedItemInfo.GameModeName:setAlpha( 0 )
	self:addElement( GameSettingsBackground )
	self.GameSettingsBackground = GameSettingsBackground

    local Options = CoD.Competitive_SettingsList.new( self, controller )
	Options:setLeftRight( true, false, 26, 741 )
	Options:setTopBottom( true, false, 135, 720 )
	Options.Title.DescTitle:setText( Engine.Localize( "Stats" ) )
	Options.ButtonList:setVerticalCount( 15 )
	Options.ButtonList:setDataSource( "MPStatsSettings" )
	self:addElement( Options )
	self.Options = Options

    self:AddButtonCallbackFunction( self, controller, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE, nil, function ( element, menu, controller, model )
		GoBack( self, controller )
    SetPerControllerTableProperty( controller, "disableGameSettingsOptions", nil )
		return true
	end, function ( element, menu, controller )
		CoD.Menu.SetButtonLabel( menu, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE, "MENU_BACK" )
		return true
	end, false )

    GameSettingsBackground.MenuFrame:setModel( self.buttonModel, controller )
    Options.id = "Options"

    self:processEvent( {
		name = "menu_loaded",
		controller = controller
	} )
	self:processEvent( {
		name = "update_state",
		menu = self
	} )
    if not self:restoreState() then
		self.Options:processEvent( {
			name = "gain_focus",
			controller = controller
		} )
	end

    LUI.OverrideFunction_CallOriginalSecond( self, "close", function ( element )
		element.GameSettingsBackground:close()
		element.Options:close()
		Engine.UnsubscribeAndFreeModel( Engine.GetModel( Engine.GetModelForController( controller ), "BoiiiStatsMenu.buttonPrompts" ) )
	end )

    if PostLoadFunc then
		PostLoadFunc( self, controller )
	end

    return self
end

CoD.LobbyButtons.MP_STATS = {
	stringRef = "STATS",
	action = function ( self, element, controller, param, menu )
    SetPerControllerTableProperty( controller, "disableGameSettingsOptions", true )
    OpenPopup( menu, "BoiiiStatsMenu", controller )
    end,
	customId = "btnMPStats"
}

CoD.LobbyButtons.MP_START_GAME = {
	stringRef = "MENU_START_GAME_CAPS",
	action = function ( self, element, controller, param, menu )
  --Engine.SetDvar( "bot_difficulty", 3 )	
	Engine.SetDvar( "party_minplayers", 1 )
	Engine.Exec( nil, "launchgame" )
    end,
	customId = "btnStartGame"
}

CoD.LobbyButtons.SETTING_UP_BOTS = {
	stringRef = "MENU_SETUP_BOTS_CAPS",
	action = function ( self, element, controller, param, menu )
    SetPerControllerTableProperty( controller, "disableGameSettingsOptions", true )
    OpenPopup( menu, "GameSettings_Bots", controller )
    end,
	customId = "btnSettingUpBots"
}

CoD.LobbyButtons.MP_CUSTOM_SETUP_GAME = {
	stringRef = "MPUI_SETUP_GAME_CAPS",
	action = OpenSetupGameMP,
	customId = "btnSetupGame",
}

local IsGamescomDemo = function ()
	return Dvar.ui_execdemo_gamescom:get()
end

local IsBetaDemo = function ()
	return Dvar.ui_execdemo_beta:get()
end

local SetButtonState = function ( button, state )
	if state == nil then
		return 
	elseif state == CoD.LobbyButtons.DISABLED then
		button.disabled = true
	elseif state == CoD.LobbyButtons.HIDDEN then
		button.hidden = true
	end
end

local AddButton = function ( controller, options, button, isLargeButton )
	button.disabled = false
	button.hidden = false
	button.selected = false
	button.warning = false
	if button.defaultState ~= nil then
		if button.defaultState == CoD.LobbyButtons.DISABLED then
		    button.disabled = true
		elseif button.defaultState == CoD.LobbyButtons.HIDDEN then
			button.hidden = true
		end
	end
	if button.disabledFunc ~= nil then
		button.disabled = button.disabledFunc( controller )
	end
	if button.visibleFunc ~= nil then
		button.hidden = not button.visibleFunc( controller )
	end
	if IsBetaDemo() then
		SetButtonState( button, button.demo_beta )
	elseif IsGamescomDemo() then
		SetButtonState( button, button.demo_gamescom )
	end
	if button.hidden then
		return 
	end
	local lobbyNav = LobbyData.GetLobbyNav()
	if button.selectedFunc ~= nil then
		button.selected = button.selectedFunc( button.selectedParam )
	elseif CoD.LobbyMenus.History[lobbyNav] ~= nil then
		button.selected = CoD.LobbyMenus.History[lobbyNav] == button.customId
	end
	if button.newBreadcrumbFunc then
		local f8_local1 = button.newBreadcrumbFunc
		if type( f8_local1 ) == "string" then
			f8_local1 = LUI.getTableFromPath( f8_local1 )
		end
		if f8_local1 then
			button.isBreadcrumbNew = f8_local1( controller )
		end
	end
	if button.warningFunc ~= nil then
		button.warning = button.warningFunc( controller )
	end
	if button.starterPack == CoD.LobbyButtons.STARTERPACK_UPGRADE then
		button.starterPackUpgrade = true
		if IsStarterPack() then
			button.disabled = false
		end
	end
	table.insert( options, {
		optionDisplay = button.stringRef,
		action = button.action,
		param = button.param,
		customId = button.customId,
		isLargeButton = isLargeButton,
		isLastButtonInGroup = false,
		disabled = button.disabled,
		selected = button.selected,
		isBreadcrumbNew = button.isBreadcrumbNew,
		warning = button.warning,
		requiredChunk = button.selectedParam,
		starterPackUpgrade = button.starterPackUpgrade,
		unloadMod = button.unloadMod
	} )
end

local AddLargeButton = function ( controller, options, button )
	AddButton( controller, options, button, true )
end

local AddSmallButton = function ( controller, options, button )
	AddButton( controller, options, button, false )
end

local AddSpacer = function ( options )
	if 0 < #options then
		options[#options].isLastButtonInGroup = true
	end
end

local MapVote = 0

CoD.LobbyMenus.MPButtonsOnline = function ( f26_arg0, f26_arg1, f26_arg2 )
	if f26_arg2 == 1 then
		AddLargeButton( f26_arg0, f26_arg1, CoD.LobbyButtons.MP_FIND_MATCH )
		AddSpacer( f26_arg1 )
	end
	AddLargeButton( f26_arg0, f26_arg1, CoD.LobbyButtons.MP_CAC_NO_WARNING )
	AddLargeButton( f26_arg0, f26_arg1, CoD.LobbyButtons.MP_SPECIALISTS_NO_WARNING )
	AddLargeButton( f26_arg0, f26_arg1, CoD.LobbyButtons.MP_SCORESTREAKS )
	if (Dvar.ui_execdemo_beta:get() or IsStarterPack()) and IsStoreAvailable() then
		if CoD.isPC then
			AddLargeButton( f26_arg0, f26_arg1, CoD.LobbyButtons.STEAM_STORE )
		else
			AddLargeButton( f26_arg0, f26_arg1, CoD.LobbyButtons.STORE )
		end
	end
	if Engine.DvarBool( nil, "inventory_test_button_visible" ) then
		AddLargeButton( f26_arg0, f26_arg1, CoD.LobbyButtons.MP_INVENTORY_TEST )
	end
	AddSpacer( f26_arg1 )
	if not DisableBlackMarket() then
		AddSmallButton( f26_arg0, f26_arg1, CoD.LobbyButtons.BLACK_MARKET )
	end
    AddSpacer( f26_arg1 )
    AddSmallButton( f26_arg0, f26_arg1, CoD.LobbyButtons.MP_STATS )
    MapVote = 1
end

CoD.LobbyMenus.MPButtonsOnlinePublic = function ( f27_arg0, f27_arg1, f27_arg2 )
	if MapVote == 1 then
	    Engine.Exec(nil, "LobbyStopDemo") -- Enable map vote at start lobby
	    MapVote = 0
    end
	AddLargeButton( f27_arg0, f27_arg1, CoD.LobbyButtons.MP_START_GAME ) --Launch match button
	AddSpacer( f27_arg1 )
	AddLargeButton( f27_arg0, f27_arg1, CoD.LobbyButtons.MP_CAC )
	AddLargeButton( f27_arg0, f27_arg1, CoD.LobbyButtons.MP_SPECIALISTS )
	AddLargeButton( f27_arg0, f27_arg1, CoD.LobbyButtons.MP_SCORESTREAKS )
	if Engine.DvarBool( nil, "inventory_test_button_visible" ) then
		AddLargeButton( f27_arg0, f27_arg1, CoD.LobbyButtons.MP_INVENTORY_TEST )
	end
--[[local f27_local0 = Engine.GetPlaylistInfoByID( Engine.GetPlaylistID() )
	if f27_local0 then
		local f27_local1 = f27_local0.playlist.category
		if f27_local1 == Engine.GetPlaylistCategoryIdByName( "core" ) or f27_local1 == Engine.GetPlaylistCategoryIdByName( "hardcore" ) then
			AddSpacer( f27_arg1 )
			AddSmallButton( f27_arg0, f27_arg1, CoD.LobbyButtons.MP_PUBLIC_LOBBY_LEADERBOARD )
		end
	end
]]  if not DisableBlackMarket() then
        AddSpacer( f27_arg1 )
        AddLargeButton( f27_arg0, f27_arg1, CoD.LobbyButtons.BLACK_MARKET )
	end
    AddSpacer( f27_arg1 )
	AddSmallButton( f27_arg0, f27_arg1, CoD.LobbyButtons.MP_CUSTOM_SETUP_GAME ) --Setup game in public lobby
end

CoD.LobbyMenus.MPButtonsArenaGame = function ( f31_arg0, f31_arg1, f31_arg2 )
	AddLargeButton( f31_arg0, f31_arg1, CoD.LobbyButtons.MP_START_GAME ) --Launch match button
	AddSpacer( f31_arg1 )
	AddLargeButton( f31_arg0, f31_arg1, CoD.LobbyButtons.MP_CAC )
	AddLargeButton( f31_arg0, f31_arg1, CoD.LobbyButtons.MP_SPECIALISTS )
	AddLargeButton( f31_arg0, f31_arg1, CoD.LobbyButtons.MP_SCORESTREAKS )
	if not DisableBlackMarket() then
		AddSpacer( f31_arg1 )
		AddLargeButton( f31_arg0, f31_arg1, CoD.LobbyButtons.BLACK_MARKET )
	end
	AddSpacer( f31_arg1 )
	AddSmallButton( f31_arg0, f31_arg1, CoD.LobbyButtons.SETTING_UP_BOTS ) --Bot setting button in public lobby
end

CoD.LobbyMenus.ZMButtonsOnline = function ( f33_arg0, f33_arg1, f33_arg2 )
	if IsStarterPack() then
		AddSmallButton( f33_arg0, f33_arg1, CoD.LobbyButtons.QUIT )
		return 
	elseif f33_arg2 == 1 then
		AddLargeButton( f33_arg0, f33_arg1, CoD.LobbyButtons.ZM_SOLO_GAME )
		AddLargeButton( f33_arg0, f33_arg1, CoD.LobbyButtons.ZM_FIND_MATCH )
		AddLargeButton( f33_arg0, f33_arg1, CoD.LobbyButtons.ZM_CUSTOM_GAMES )
		AddLargeButton( f33_arg0, f33_arg1, CoD.LobbyButtons.THEATER_ZM )
		AddSpacer( f33_arg1 )
	end
	AddLargeButton( f33_arg0, f33_arg1, CoD.LobbyButtons.ZM_BUBBLEGUM_BUFFS )
	AddLargeButton( f33_arg0, f33_arg1, CoD.LobbyButtons.ZM_MEGACHEW_FACTORY )
	AddLargeButton( f33_arg0, f33_arg1, CoD.LobbyButtons.ZM_GOBBLEGUM_RECIPES )
	AddLargeButton( f33_arg0, f33_arg1, CoD.LobbyButtons.ZM_BUILD_KITS )
    AddSpacer( f33_arg1 )
    AddSmallButton( f33_arg0, f33_arg1, CoD.LobbyButtons.MP_STATS )
end

local targetButtons = {
	[LobbyData.UITargets.UI_MAIN.id] = CoD.LobbyMenus.ModeSelect,
	[LobbyData.UITargets.UI_MODESELECT.id] = CoD.LobbyMenus.ModeSelect,
	[LobbyData.UITargets.UI_CPLOBBYLANGAME.id] = CoD.LobbyMenus.CPButtonsLAN,
	[LobbyData.UITargets.UI_CPLOBBYLANCUSTOMGAME.id] = CoD.LobbyMenus.CPButtonsLANCUSTOM,
	[LobbyData.UITargets.UI_CPLOBBYONLINE.id] = CoD.LobbyMenus.CPButtonsOnline,
	[LobbyData.UITargets.UI_CPLOBBYONLINEPUBLICGAME.id] = CoD.LobbyMenus.CPButtonsPublicGame,
	[LobbyData.UITargets.UI_CPLOBBYONLINECUSTOMGAME.id] = CoD.LobbyMenus.CPButtonsCustomGame,
	[LobbyData.UITargets.UI_CP2LOBBYLANGAME.id] = CoD.LobbyMenus.CPZMButtonsLAN,
	[LobbyData.UITargets.UI_CP2LOBBYLANCUSTOMGAME.id] = CoD.LobbyMenus.CPButtonsLANCUSTOM,
	[LobbyData.UITargets.UI_CP2LOBBYONLINE.id] = CoD.LobbyMenus.CPZMButtonsOnline,
	[LobbyData.UITargets.UI_CP2LOBBYONLINEPUBLICGAME.id] = CoD.LobbyMenus.CPZMButtonsPublicGame,
	[LobbyData.UITargets.UI_CP2LOBBYONLINECUSTOMGAME.id] = CoD.LobbyMenus.CPButtonsCustomGame,
	[LobbyData.UITargets.UI_DOALOBBYLANGAME.id] = CoD.LobbyMenus.DOAButtonsLAN,
	[LobbyData.UITargets.UI_DOALOBBYONLINE.id] = CoD.LobbyMenus.DOAButtonsOnline,
	[LobbyData.UITargets.UI_DOALOBBYONLINEPUBLICGAME.id] = CoD.LobbyMenus.DOAButtonsPublicGame,
	[LobbyData.UITargets.UI_MPLOBBYLANGAME.id] = CoD.LobbyMenus.MPButtonsLAN,
	[LobbyData.UITargets.UI_MPLOBBYMAIN.id] = CoD.LobbyMenus.MPButtonsMain,
	[LobbyData.UITargets.UI_MPLOBBYONLINE.id] = CoD.LobbyMenus.MPButtonsOnline,
	[LobbyData.UITargets.UI_MPLOBBYONLINEPUBLICGAME.id] = CoD.LobbyMenus.MPButtonsOnlinePublic,
	[LobbyData.UITargets.UI_MPLOBBYONLINEMODGAME.id] = CoD.LobbyMenus.MPButtonsModGame,
	[LobbyData.UITargets.UI_MPLOBBYONLINECUSTOMGAME.id] = CoD.LobbyMenus.MPButtonsCustomGame,
	[LobbyData.UITargets.UI_MPLOBBYONLINEARENA.id] = CoD.LobbyMenus.MPButtonsArena,
	[LobbyData.UITargets.UI_MPLOBBYONLINEARENAGAME.id] = CoD.LobbyMenus.MPButtonsArenaGame,
	[LobbyData.UITargets.UI_FRLOBBYONLINEGAME.id] = CoD.LobbyMenus.FRButtonsOnlineGame,
	[LobbyData.UITargets.UI_FRLOBBYLANGAME.id] = CoD.LobbyMenus.FRButtonsLANGame,
	[LobbyData.UITargets.UI_ZMLOBBYLANGAME.id] = CoD.LobbyMenus.ZMButtonsLAN,
	[LobbyData.UITargets.UI_ZMLOBBYONLINE.id] = CoD.LobbyMenus.ZMButtonsOnline,
	[LobbyData.UITargets.UI_ZMLOBBYONLINEPUBLICGAME.id] = CoD.LobbyMenus.ZMButtonsPublicGame,
	[LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME.id] = CoD.LobbyMenus.ZMButtonsCustomGame,
	[LobbyData.UITargets.UI_MPLOBBYONLINETHEATER.id] = CoD.LobbyMenus.ButtonsTheaterGame,
	[LobbyData.UITargets.UI_ZMLOBBYONLINETHEATER.id] = CoD.LobbyMenus.ButtonsTheaterGame
}

CoD.LobbyMenus.AddButtonsForTarget = function ( controller, id )
	local buttonFunc = targetButtons[id]
	local model = nil
	if Engine.IsLobbyActive( Enum.LobbyType.LOBBY_TYPE_GAME ) then
		model = Engine.GetModel( DataSources.LobbyRoot.getModel( controller ), "gameClient.isHost" )
	else
		model = Engine.GetModel( DataSources.LobbyRoot.getModel( controller ), "privateClient.isHost" )
	end
	local isLeader = nil
	if model ~= nil then
		isLeader = Engine.GetModelValue( model )
	else
		isLeader = 1
	end
	local result = {}
	buttonFunc( controller, result, isLeader )
	return result
end

