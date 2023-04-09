if Engine.GetCurrentMap() ~= "core_frontend" then
	return
end

DataSources.MPStatsSettings = DataSourceHelpers.ListSetup("MPStatsSettings", function(controller)
	local optionsTable = {}

	local updateDvar = function(f1_arg0, f1_arg1, f1_arg2, dvarName, f1_arg4)
		local oldValue = Engine.DvarInt(nil, dvarName)
		local newValue = f1_arg1.value
		UpdateInfoModels(f1_arg1)
		if oldValue == newValue then
			return
		end
		Engine.SetDvar(dvarName, f1_arg1.value)
		if dvarName == "cg_unlockall_loot" then
			Engine.SetDvar("ui_enableAllHeroes", f1_arg1.value)
		end
	end

	table.insert(optionsTable,
		CoD.OptionsUtility.CreateDvarSettings(controller, "Unlock All Loot",
			"Whether loot should be locked based on the player's stats or always unlocked.", "MPStatsSettings_unlock_loot",
			"cg_unlockall_loot", {
				{
					option = "MENU_DISABLED",
					value = 0,
					default = true
				},
				{
					option = "MENU_ENABLED",
					value = 1
				},
			}, nil, updateDvar))
	if Engine.CurrentSessionMode() == Enum.eModes.MODE_MULTIPLAYER then
		table.insert(optionsTable,
			CoD.OptionsUtility.CreateDvarSettings(controller, "Unlock All Purchases",
				"All items that need to be purchased with unlock tokens are unlocked.", "MPStatsSettings_purchase_all",
				"cg_unlockall_purchases", {
					{
						option = "MENU_DISABLED",
						value = 0,
						default = true
					},
					{
						option = "MENU_ENABLED",
						value = 1
					},
				}, nil, updateDvar))
	end
	table.insert(optionsTable,
		CoD.OptionsUtility.CreateDvarSettings(controller, "Unlock All Attachments",
			"All attachments on weapons are unlocked.",
			"MPStatsSettings_unlockall_attachments", "cg_unlockall_attachments", {
				{
					option = "MENU_DISABLED",
					value = 0,
					default = true
				},
				{
					option = "MENU_ENABLED",
					value = 1
				},
			}, nil, updateDvar))
	table.insert(optionsTable,
		CoD.OptionsUtility.CreateDvarSettings(controller, "Unlock all Camos and Reticles",
			"All camos and reticles on weapons are unlocked.", "MPStatsSettings_unlockall_camos_and_reticles",
			"cg_unlockall_camos_and_reticles", {
				{
					option = "MENU_DISABLED",
					value = 0,
					default = true
				},
				{
					option = "MENU_ENABLED",
					value = 1
				},
			}, nil, updateDvar))
	table.insert(optionsTable,
		CoD.OptionsUtility.CreateDvarSettings(controller, "Unlock all Calling Cards", "All calling cards are unlocked.",
			"MPStatsSettings_unlockall_calling_cards", "cg_unlockall_calling_cards", {
				{
					option = "MENU_DISABLED",
					value = 0,
					default = true
				},
				{
					option = "MENU_ENABLED",
					value = 1
				},
			}, nil, updateDvar))
	table.insert(optionsTable,
		CoD.OptionsUtility.CreateDvarSettings(controller, "Unlock all Specialists Outfits",
			"All specialists outfits are unlocked.", "MPStatsSettings_unlockall_specialists_outfits",
			"cg_unlockall_specialists_outfits", {
				{
					option = "MENU_DISABLED",
					value = 0,
					default = true
				},
				{
					option = "MENU_ENABLED",
					value = 1
				},
			}, nil, updateDvar))

	local rankLevels = {}
	if Engine.CurrentSessionMode() == Enum.eModes.MODE_MULTIPLAYER then
		rankLevels = { 1, 10, 20, 30, 40, 50, 55 }
	elseif Engine.CurrentSessionMode() == Enum.eModes.MODE_ZOMBIES then
		rankLevels = { 1, 10, 20, 30, 35 }
	end
	local rankObjs = {}
	local hasDefault = false
	local currentRank = CoD.BlackMarketUtility.GetCurrentRank(controller) + 1
	for index, value in ipairs(rankLevels) do
		table.insert(rankObjs, {
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
		table.insert(rankObjs, {
			name = currentRank,
			value = currentRank - 1,
			default = true,
			title = "Rank Level",
			desc = ""
		})
	end

	local prestigeTable = {}
	for i = 0, 10 do
		table.insert(prestigeTable, {
			name = i == 0 and "None" or i,
			value = i,
			default = i == CoD.PrestigeUtility.GetCurrentPLevel(controller),
			title = "Prestige",
			desc = ""
		})
	end

	local createSettingsDatasource = function(controller, datasourceName, optionsTable, currentValue, loopEdges, action)
		if currentValue == nil then
			currentValue = 0
		end
		DataSources[datasourceName] = DataSourceHelpers.ListSetup(datasourceName, function(f47_arg0)
			local f47_local0 = {}
			for f47_local4, f47_local5 in ipairs(optionsTable) do
				table.insert(f47_local0, {
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
						showChangeIndicator = function(f48_arg0, f48_arg1, f48_arg2)
							return f48_arg0.default ~= true
						end
					}
				})
			end
			f47_local0[1].properties.first = true
			f47_local0[#optionsTable].properties.last = true
			return f47_local0
		end, nil, nil, nil)
		return datasourceName
	end

	table.insert(optionsTable, {
		models = {
			name = "Rank Level",
			desc = "",
			image = nil,
			optionsDatasource = createSettingsDatasource(controller, "MPStatsSettings_rank_level", rankObjs,
				CoD.BlackMarketUtility.GetCurrentRank(controller), false, function(f1_arg0, f1_arg1, f1_arg2, dvarName, f1_arg4)
					UpdateInfoModels(f1_arg1)
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
					Engine.Exec(f1_arg0, "uploadstats " .. tostring(Engine.CurrentSessionMode()))
				end)
		},
		properties = {
			revert = function(f50_arg0)
			end
		}
	})

	table.insert(optionsTable, {
		models = {
			name = "Prestige",
			desc = "",
			image = nil,
			optionsDatasource = createSettingsDatasource(controller, "MPStatsSettings_rank_prestige", prestigeTable,
				CoD.PrestigeUtility.GetCurrentPLevel(controller), false, function(f1_arg0, f1_arg1, f1_arg2, dvarName, f1_arg4)
					UpdateInfoModels(f1_arg1)
					local newPrestige = f1_arg1.value
					Engine.ExecNow(f1_arg0, "statsetbyname plevel " .. newPrestige)
					Engine.ExecNow(f1_arg0, "statsetbyname hasprestiged " .. (newPrestige > 0 and 1 or 0))
					Engine.Exec(f1_arg0, "uploadstats " .. tostring(Engine.CurrentSessionMode()))
				end)
		},
		properties = {
			revert = function(f50_arg0)
			end
		}
	})

	return optionsTable
end)

if Dvar.cg_unlockall_loot:get() == true then
	Engine.SetDvar("ui_enableAllHeroes", 1)
end

LUI.createMenu.BoiiiStatsMenu = function(controller)
	local self = CoD.Menu.NewForUIEditor("BoiiiStatsMenu")
	if PreLoadFunc then
		PreLoadFunc(self, controller)
	end
	self.soundSet = "ChooseDecal"
	self:setOwner(controller)
	self:setLeftRight(true, true, 0, 0)
	self:setTopBottom(true, true, 0, 0)
	self:playSound("menu_open", controller)
	self.buttonModel = Engine.CreateModel(Engine.GetModelForController(controller), "BoiiiStatsMenu.buttonPrompts")
	self.anyChildUsesUpdateState = true

	local GameSettingsBackground = CoD.GameSettings_Background.new(self, controller)
	GameSettingsBackground:setLeftRight(true, true, 0, 0)
	GameSettingsBackground:setTopBottom(true, true, 0, 0)
	GameSettingsBackground.MenuFrame.titleLabel:setText(Engine.Localize("STATS SETTINGS"))
	GameSettingsBackground.MenuFrame.cac3dTitleIntermediary0.FE3dTitleContainer0.MenuTitle.TextBox1.Label0:setText(Engine
		.Localize("STATS SETTINGS"))
	GameSettingsBackground.GameSettingsSelectedItemInfo.GameModeInfo:setAlpha(0)
	GameSettingsBackground.GameSettingsSelectedItemInfo.GameModeName:setAlpha(0)
	self:addElement(GameSettingsBackground)
	self.GameSettingsBackground = GameSettingsBackground

	local Options = CoD.Competitive_SettingsList.new(self, controller)
	Options:setLeftRight(true, false, 26, 741)
	Options:setTopBottom(true, false, 135, 720)
	Options.Title.DescTitle:setText(Engine.Localize("Stats"))
	Options.ButtonList:setVerticalCount(15)
	Options.ButtonList:setDataSource("MPStatsSettings")
	self:addElement(Options)
	self.Options = Options

	self:AddButtonCallbackFunction(self, controller, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE, nil,
		function(element, menu, controller, model)
			GoBack(self, controller)
			SetPerControllerTableProperty(controller, "disableGameSettingsOptions", nil)
			return true
		end, function(element, menu, controller)
			CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE, "MENU_BACK")
			return true
		end, false)

	GameSettingsBackground.MenuFrame:setModel(self.buttonModel, controller)
	Options.id = "Options"

	self:processEvent({
		name = "menu_loaded",
		controller = controller
	})
	self:processEvent({
		name = "update_state",
		menu = self
	})
	if not self:restoreState() then
		self.Options:processEvent({
			name = "gain_focus",
			controller = controller
		})
	end

	LUI.OverrideFunction_CallOriginalSecond(self, "close", function(element)
		element.GameSettingsBackground:close()
		element.Options:close()
		Engine.UnsubscribeAndFreeModel(Engine.GetModel(Engine.GetModelForController(controller),
			"BoiiiStatsMenu.buttonPrompts"))
	end)

	if PostLoadFunc then
		PostLoadFunc(self, controller)
	end

	return self
end
