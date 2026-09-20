if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

local function refreshCurrencyModels(controller)
  local controllerModel = Engine.GetModelForController(controller)
  local currencyModel = Engine.CreateModel(controllerModel, "CryptoKeyProgress")
  Engine.SetModelValue(Engine.CreateModel(currencyModel, "codPoints"), Engine.GetCoDPoints(controller))
  local tokenModel = Engine.CreateModel(controllerModel, "MegaChewTokens")
  Engine.SetModelValue(Engine.CreateModel(tokenModel, "remainingTokens"), Engine.GetZMVials(controller))
  if DataSources.GobbleGumDistills then
    DataSources.GobbleGumDistills.getModel(controller)
  end
end

local easterEggStats = {
  "DARKOPS_ZOD_EE",
  "DARKOPS_ZOD_SUPER_EE",
  "DARKOPS_FACTORY_EE",
  "DARKOPS_FACTORY_SUPER_EE",
  "DARKOPS_CASTLE_EE",
  "DARKOPS_CASTLE_SUPER_EE",
  "DARKOPS_ISLAND_EE",
  "DARKOPS_ISLAND_SUPER_EE",
  "DARKOPS_STALINGRAD_EE",
  "DARKOPS_STALINGRAD_SUPER_EE",
  "DARKOPS_GENESIS_EE",
  "DARKOPS_GENESIS_SUPER_EE",
}

local function allEasterEggsCompleted(controller)
  local stats = Engine.GetPlayerStats(controller, CoD.STATS_LOCATION_NORMAL, Enum.eModes.MODE_ZOMBIES)
  if not stats or not stats.PlayerStatsList then
    return false
  end
  for _, name in ipairs(easterEggStats) do
    local stat = stats.PlayerStatsList[name]
    if not stat or not stat.StatValue or stat.StatValue:get() ~= 1 then
      return false
    end
  end
  return true
end

DataSources.MPStatsSettings = DataSourceHelpers.ListSetup("MPStatsSettings", function(controller)
  local optionsTable = {}

  Engine.SetDvar("all_ee_completed", allEasterEggsCompleted(controller) and 1 or 0)

  local updateDvar = function(element, itemModel, controllerIndex, dvarName, param)
    local oldValue = Engine.DvarInt(nil, dvarName)
    local newValue = itemModel.value
    UpdateInfoModels(itemModel)
    if oldValue == newValue then
      return
    end
    Engine.SetDvar(dvarName, itemModel.value)
    if dvarName == "cg_unlockall_loot" then
      Engine.SetDvar("ui_enableAllHeroes", itemModel.value)
      if itemModel.value == 1 then
        Engine.SetDvar("cg_unlockall_gobblegums", 1)
      end
    end
    if dvarName == "cg_local_currency" then
      refreshCurrencyModels(controllerIndex)
    end
    if dvarName == "all_ee_completed" then
      Engine.ExecNow(controllerIndex, "statsetbyname darkops_zod_ee " .. itemModel.value)
      Engine.ExecNow(controllerIndex, "statsetbyname darkops_zod_super_ee " .. itemModel.value)
      Engine.ExecNow(controllerIndex, "statsetbyname darkops_factory_ee " .. itemModel.value)
      Engine.ExecNow(controllerIndex, "statsetbyname darkops_factory_super_ee " .. itemModel.value)
      Engine.ExecNow(controllerIndex, "statsetbyname darkops_castle_ee " .. itemModel.value)
      Engine.ExecNow(controllerIndex, "statsetbyname darkops_castle_super_ee " .. itemModel.value)
      Engine.ExecNow(controllerIndex, "statsetbyname darkops_island_ee " .. itemModel.value)
      Engine.ExecNow(controllerIndex, "statsetbyname darkops_island_super_ee " .. itemModel.value)
      Engine.ExecNow(controllerIndex, "statsetbyname darkops_stalingrad_ee " .. itemModel.value)
      Engine.ExecNow(controllerIndex, "statsetbyname darkops_stalingrad_super_ee " .. itemModel.value)
      Engine.ExecNow(controllerIndex, "statsetbyname darkops_genesis_ee " .. itemModel.value)
      Engine.ExecNow(controllerIndex, "statsetbyname DARKOPS_GENESIS_SUPER_EE " .. itemModel.value)
      game.savestats(controllerIndex, Enum.eModes.MODE_ZOMBIES)
    end
  end

  local updateMaxCurrencies = function(element, itemModel, controllerIndex, dvarName, param)
    local oldValue = Engine.DvarInt(nil, dvarName)
    local newValue = itemModel.value
    UpdateInfoModels(itemModel)
    if oldValue == newValue then
      return
    end
    if game.setcurrenciesmaxed(controllerIndex, newValue == 1) then
      Engine.SetDvar(dvarName, newValue)
      refreshCurrencyModels(controllerIndex)
      CoD.OverlayUtility.ShowToast(
        "BlackMarketEquipped",
        newValue == 1 and "Local currencies maxed." or "Local currencies cleared.",
        nil,
        "uie_t7_icon_codpoints"
      )
    else
      itemModel.value = oldValue
      UpdateInfoModels(itemModel)
      LuaUtils.UI_ShowErrorMessageDialog(
        controllerIndex,
        "Could not update currencies. Return to the Zombies menu and try again."
      )
    end
  end

  table.insert(
    optionsTable,
    CoD.OptionsUtility.CreateDvarSettings(
      controller,
      "Unlock All Loot",
      "Unlocks all Black Market loot.",
      "MPStatsSettings_unlock_loot",
      "cg_unlockall_loot",
      {
        {
          option = "MENU_DISABLED",
          value = 0,
          default = true,
        },
        {
          option = "MENU_ENABLED",
          value = 1,
        },
      },
      nil,
      updateDvar
    )
  )
  if
    Engine.CurrentSessionMode() == Enum.eModes.MODE_ZOMBIES
    and type(game.resetgobblegums) == "function"
    and type(game.setcurrenciesmaxed) == "function"
  then
    table.insert(
      optionsTable,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Local Currency",
        "Uses locally saved COD Points, Liquid Divinium, GobbleGums, and Cookbook Distills.",
        "MPStatsSettings_local_currency",
        "cg_local_currency",
        {
          { option = "MENU_DISABLED", value = 0 },
          { option = "MENU_ENABLED", value = 1, default = true },
        },
        nil,
        updateDvar
      )
    )
    table.insert(
      optionsTable,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Unlimited GobbleGums",
        "Uses unlimited GobbleGums without replacing your earned inventory.",
        "MPStatsSettings_unlimited_gobblegums",
        "cg_unlockall_gobblegums",
        {
          { option = "MENU_DISABLED", value = 0, default = true },
          { option = "MENU_ENABLED", value = 1 },
        },
        nil,
        updateDvar
      )
    )
    table.insert(
      optionsTable,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Max Local Currencies",
        "Sets COD Points, Liquid Divinium, and Cookbook Distills to maximum or zero.",
        "MPStatsSettings_max_currencies",
        "cg_max_local_currencies",
        {
          { option = "MENU_DISABLED", value = 0, default = true },
          { option = "MENU_ENABLED", value = 1 },
        },
        nil,
        updateMaxCurrencies
      )
    )
  end
  if Engine.CurrentSessionMode() == Enum.eModes.MODE_MULTIPLAYER then
    table.insert(
      optionsTable,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Unlock All Purchases",
        "All items that need to be purchased with unlock tokens are unlocked.",
        "MPStatsSettings_purchase_all",
        "cg_unlockall_purchases",
        {
          {
            option = "MENU_DISABLED",
            value = 0,
            default = true,
          },
          {
            option = "MENU_ENABLED",
            value = 1,
          },
        },
        nil,
        updateDvar
      )
    )
    table.insert(
      optionsTable,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Unlock All Class Slots",
        "Unlocks all create-a-class slots and sets.",
        "MPStatsSettings_unlockall_cac_slots",
        "cg_unlockall_cac_slots",
        {
          {
            option = "MENU_DISABLED",
            value = 0,
            default = true,
          },
          {
            option = "MENU_ENABLED",
            value = 1,
          },
        },
        nil,
        updateDvar
      )
    )
  end
  table.insert(
    optionsTable,
    CoD.OptionsUtility.CreateDvarSettings(
      controller,
      "Unlock All Attachments",
      "All attachments on weapons are unlocked.",
      "MPStatsSettings_unlockall_attachments",
      "cg_unlockall_attachments",
      {
        {
          option = "MENU_DISABLED",
          value = 0,
          default = true,
        },
        {
          option = "MENU_ENABLED",
          value = 1,
        },
      },
      nil,
      updateDvar
    )
  )
  table.insert(
    optionsTable,
    CoD.OptionsUtility.CreateDvarSettings(
      controller,
      "Unlock all Camos and Reticles",
      "All camos and reticles on weapons are unlocked.",
      "MPStatsSettings_unlockall_camos_and_reticles",
      "cg_unlockall_camos_and_reticles",
      {
        {
          option = "MENU_DISABLED",
          value = 0,
          default = true,
        },
        {
          option = "MENU_ENABLED",
          value = 1,
        },
      },
      nil,
      updateDvar
    )
  )
  table.insert(
    optionsTable,
    CoD.OptionsUtility.CreateDvarSettings(
      controller,
      "Unlock all Calling Cards",
      "All calling cards are unlocked.",
      "MPStatsSettings_unlockall_calling_cards",
      "cg_unlockall_calling_cards",
      {
        {
          option = "MENU_DISABLED",
          value = 0,
          default = true,
        },
        {
          option = "MENU_ENABLED",
          value = 1,
        },
      },
      nil,
      updateDvar
    )
  )
  if Engine.CurrentSessionMode() == Enum.eModes.MODE_MULTIPLAYER then
    table.insert(
      optionsTable,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Unlock all Specialists Outfits",
        "All specialists outfits are unlocked.",
        "MPStatsSettings_unlockall_specialists_outfits",
        "cg_unlockall_specialists_outfits",
        {
          {
            option = "MENU_DISABLED",
            value = 0,
            default = true,
          },
          {
            option = "MENU_ENABLED",
            value = 1,
          },
        },
        nil,
        updateDvar
      )
    )
  end
  if Engine.CurrentSessionMode() == Enum.eModes.MODE_ZOMBIES then
    table.insert(
      optionsTable,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Unlock Easter Eggs",
        "Complete all Easter Egg Achievements.",
        "MPStatsSettings_complete_ee",
        "all_ee_completed",
        {
          {
            option = "MENU_DISABLED",
            value = 0,
            default = true,
          },
          {
            option = "MENU_ENABLED",
            value = 1,
          },
        },
        nil,
        updateDvar
      )
    )
  end

  local rankLevels = {}
  local rankObjs = {}
  local hasDefault = true
  local currentPrestige = CoD.PrestigeUtility.GetCurrentPLevel(controller, Engine.CurrentSessionMode())
  local currentRank = CoD.BlackMarketUtility.GetCurrentRank(controller) + 1

  local isMasterPrestige = currentPrestige == 11
  if isMasterPrestige then
    local stats = Engine.GetPlayerStats(controller, CoD.STATS_LOCATION_NORMAL, Engine.CurrentSessionMode())
    if stats and stats.PlayerStatsList and stats.PlayerStatsList.PARAGON_RANK then
      local paragonRank = stats.PlayerStatsList.PARAGON_RANK.StatValue:get()
      if Engine.CurrentSessionMode() == Enum.eModes.MODE_MULTIPLAYER then
        currentRank = paragonRank + 56
      elseif Engine.CurrentSessionMode() == Enum.eModes.MODE_ZOMBIES then
        currentRank = paragonRank + 36
      elseif Engine.CurrentSessionMode() == Enum.eModes.MODE_CAMPAIGN then
        currentRank = paragonRank + 21
      end
    end
  end

  if Engine.CurrentSessionMode() == Enum.eModes.MODE_MULTIPLAYER then
    if not isMasterPrestige then
      rankLevels = { 1, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55 }
    else
      rankLevels = { 56, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000 }
    end
  elseif Engine.CurrentSessionMode() == Enum.eModes.MODE_ZOMBIES then
    if not isMasterPrestige then
      rankLevels = { 1, 5, 10, 15, 20, 25, 30, 35 }
    else
      rankLevels = { 36, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000 }
    end
  elseif Engine.CurrentSessionMode() == Enum.eModes.MODE_CAMPAIGN then
    if not isMasterPrestige then
      rankLevels = { 1, 5, 10, 15, 20 }
    else
      rankLevels = { 21, 50, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000 }
    end
  end

  if #rankLevels > 0 then
    local maxlevel = math.max(table.unpack(rankLevels))
    local minlevel = math.min(table.unpack(rankLevels))

    for index, value in ipairs(rankLevels) do
      table.insert(rankObjs, {
        name = value <= minlevel and "Min" or value >= maxlevel and "Max" or value,
        value = value - 1,
        default = value == currentRank,
        title = "Rank Level",
        desc = value ~= currentRank and "" or "Current Rank",
      })
    end

    if hasDefault and currentRank ~= minlevel and currentRank < maxlevel and not isMasterPrestige then
      table.insert(rankObjs, {
        name = "Current: "
          .. tostring(currentRank <= minlevel and "Min" or currentRank >= maxlevel and "Max" or currentRank),
        value = currentRank - 1,
        default = true,
        title = "Rank Level",
        desc = "Do not adjust rank",
      })
    end
  end

  local prestigeTable = {}
  for i = 0, 11 do
    table.insert(prestigeTable, {
      name = i == 0 and "None" or i == 11 and "Master" or i,
      value = i,
      default = i == currentPrestige,
      title = "Prestige",
      desc = "",
    })
  end

  local createSettingsDatasource = function(controller, datasourceName, optionList, currentValue, loopEdges, action)
    if currentValue == nil then
      currentValue = 0
    end
    DataSources[datasourceName] = DataSourceHelpers.ListSetup(datasourceName, function(ctrl)
      local listItems = {}
      for index, optionDef in ipairs(optionList) do
        table.insert(listItems, {
          models = {
            text = optionList[index].name,
          },
          properties = {
            title = optionList[index].title,
            desc = optionList[index].desc,
            image = optionList[index].image,
            value = optionList[index].value,
            default = optionList[index].default,
            action = action,
            selectIndex = optionList[index].value == currentValue,
            loopEdges = loopEdges,
            showChangeIndicator = function(props, model, element)
              return props.default ~= true
            end,
          },
        })
      end
      if #listItems > 0 then
        listItems[1].properties.first = true
        listItems[#listItems].properties.last = true
      end
      return listItems
    end, nil, nil, nil)
    return datasourceName
  end

  if #rankLevels > 0 then
    table.insert(optionsTable, {
      models = {
        name = "Prestige",
        desc = "",
        image = nil,
        optionsDatasource = createSettingsDatasource(
          controller,
          "MPStatsSettings_rank_prestige",
          prestigeTable,
          CoD.PrestigeUtility.GetCurrentPLevel(controller, Engine.CurrentSessionMode()),
          false,
          function(element, itemModel, controllerIndex, dvarName, param)
            UpdateInfoModels(itemModel)
            local newPrestige = itemModel.value
            if newPrestige == 11 then
              Engine.ExecNow(controllerIndex, "PrestigeStatsMaster " .. tostring(Engine.CurrentSessionMode()))
            end
            Engine.ExecNow(controllerIndex, "statsetbyname plevel " .. newPrestige)
            Engine.ExecNow(controllerIndex, "statsetbyname hasprestiged " .. (newPrestige > 0 and 1 or 0))
            game.savestats(controllerIndex, Engine.CurrentSessionMode())
          end
        ),
      },
      properties = {
        revert = function(element) end,
      },
    })

    table.insert(optionsTable, {
      models = {
        name = "Rank Level",
        desc = "",
        image = nil,
        optionsDatasource = createSettingsDatasource(
          controller,
          "MPStatsSettings_rank_level",
          rankObjs,
          currentRank - 1,
          false,
          function(element, itemModel, controllerIndex, dvarName, param)
            UpdateInfoModels(itemModel)
            local rankTable = nil
            local rank = itemModel.value + 1
            if currentPrestige <= 10 then
              if Engine.CurrentSessionMode() == Enum.eModes.MODE_MULTIPLAYER then
                rankTable = "gamedata/tables/mp/mp_ranktable.csv"
              elseif Engine.CurrentSessionMode() == Enum.eModes.MODE_ZOMBIES then
                rankTable = "gamedata/tables/zm/zm_ranktable.csv"
              elseif Engine.CurrentSessionMode() == Enum.eModes.MODE_CAMPAIGN then
                rankTable = "gamedata/tables/cp/cp_ranktable.csv"
              end
              local skipLines = Engine.CurrentSessionMode() == Enum.eModes.MODE_MULTIPLAYER and 3 or 2
              local maxXp = tonumber(Engine.TableLookupGetColumnValueForRow(rankTable, rank - 2 + skipLines, 7))
              if Engine.CurrentSessionMode() == Enum.eModes.MODE_MULTIPLAYER then
                if maxXp ~= nil and rank == maxlevel then
                  maxXp = maxXp + 55600
                end
              end
              if Engine.CurrentSessionMode() == Enum.eModes.MODE_ZOMBIES then
                if maxXp ~= nil and rank == maxlevel then
                  maxXp = maxXp + 54244
                end
              end
              if maxXp == nil then
                maxXp = 0
              end
              Engine.ExecNow(controllerIndex, "statsetbyname rank " .. rank - 1)
              Engine.ExecNow(controllerIndex, "statsetbyname rankxp " .. maxXp)
              Engine.ExecNow(controllerIndex, "statsetbyname paragon_rankxp " .. 0)
            else
              if Engine.CurrentSessionMode() == Enum.eModes.MODE_MULTIPLAYER then
                rankTable = "gamedata/tables/mp/mp_paragonranktable.csv"
              elseif Engine.CurrentSessionMode() == Enum.eModes.MODE_ZOMBIES then
                rankTable = "gamedata/tables/zm/zm_paragonranktable.csv"
              elseif Engine.CurrentSessionMode() == Enum.eModes.MODE_CAMPAIGN then
                rankTable = "gamedata/tables/cp/cp_paragonranktable.csv"
              end
              local skipLines = 2
              local maxXp = 0
              if Engine.CurrentSessionMode() == Enum.eModes.MODE_MULTIPLAYER then
                maxXp = tonumber(Engine.TableLookupGetColumnValueForRow(rankTable, rank - 57 + skipLines, 7))
                if maxXp ~= nil and rank == maxlevel then
                  maxXp = maxXp + 55600
                end
                rank = rank - 55
              end
              if Engine.CurrentSessionMode() == Enum.eModes.MODE_ZOMBIES then
                maxXp = tonumber(Engine.TableLookupGetColumnValueForRow(rankTable, rank - 37 + skipLines, 7))
                if maxXp ~= nil and rank == maxlevel then
                  maxXp = maxXp + 54244
                end
                rank = rank - 35
              end
              if maxXp == nil then
                maxXp = 0
              end
              Engine.ExecNow(controllerIndex, "statsetbyname paragon_rank  " .. rank - 1)
              Engine.ExecNow(controllerIndex, "statsetbyname paragon_rankxp " .. maxXp)
            end
            game.savestats(controllerIndex, Engine.CurrentSessionMode())

            currentRank = rank
          end
        ),
      },
      properties = {
        revert = function(element) end,
      },
    })
  end -- #rankLevels > 0

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
  GameSettingsBackground.MenuFrame.cac3dTitleIntermediary0.FE3dTitleContainer0.MenuTitle.TextBox1.Label0:setText(
    Engine.Localize("STATS SETTINGS")
  )
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

  self:AddButtonCallbackFunction(
    self,
    controller,
    Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE,
    nil,
    function(element, menu, controller, model)
      GoBack(self, controller)
      SetPerControllerTableProperty(controller, "disableGameSettingsOptions", nil)
      return true
    end,
    function(element, menu, controller)
      CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE, "MENU_BACK")
      return true
    end,
    false
  )

  GameSettingsBackground.MenuFrame:setModel(self.buttonModel, controller)
  Options.id = "Options"

  self:processEvent({
    name = "menu_loaded",
    controller = controller,
  })
  self:processEvent({
    name = "update_state",
    menu = self,
  })
  if not self:restoreState() then
    self.Options:processEvent({
      name = "gain_focus",
      controller = controller,
    })
  end

  LUI.OverrideFunction_CallOriginalSecond(self, "close", function(element)
    element.GameSettingsBackground:close()
    element.Options:close()
    Engine.UnsubscribeAndFreeModel(
      Engine.GetModel(Engine.GetModelForController(controller), "BoiiiStatsMenu.buttonPrompts")
    )
  end)

  if PostLoadFunc then
    PostLoadFunc(self, controller)
  end

  return self
end
