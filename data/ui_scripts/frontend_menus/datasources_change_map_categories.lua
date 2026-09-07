local SetupMouseHandling = function(element, controller)
  if not CoD.useMouse then
    return
  else
    LUI.OverrideFunction_CallOriginalFirst(element, "setState", function(element, controller)
      if IsSelfInState(element, "SelectingMap") then
        element.mapList:setMouseDisabled(false)
        element.mapCategoriesList:setMouseDisabled(true)
        element.m_categorySet = false
      else
        element.mapList:setMouseDisabled(true)
        element.mapCategoriesList:setMouseDisabled(false)
      end
    end)
    element.mapList:setMouseDisabled(true)
    element.mapList:registerEventHandler("leftclick_outside", function(element, event)
      if IsSelfInState(element, "SelectingMap") and element.m_categorySet then
        CoD.PCUtil.SimulateButtonPress(controller, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE)
      end
      element.m_categorySet = true
      return true
    end)
  end
end

local PostLoadFunc = function(self, controller)
  SetupMouseHandling(self, controller)
end

local DLC_PACK_INDEX = 10000
local DLC_BONUS_PACK_INDEX = 10001

local GetMapDlcPackCategory = function(mapName)
  local mapData = CoD.mapsTable[mapName]
  if CoD.CONTENT_DLC6_INDEX <= mapData.dlc_pack or mapName == "mp_redwood_ice" or mapName == "mp_veiled_heyday" then
    return DLC_BONUS_PACK_INDEX
  elseif mapData.dlc_pack > 0 then
    return DLC_PACK_INDEX
  else
    return mapData.dlc_pack
  end
end

DataSources.ChangeMapCategories = DataSourceHelpers.ListSetup("ChangeMapCategories", function(controller)
  local categories = {}
  local currentMapDlcPack =
    CoD.GetMapValue(Engine.DvarString(nil, "ui_mapname"), "dlc_pack", CoD.CONTENT_ORIGINAL_MAP_INDEX)

  local CreateCategoryEntry = function(categoryName, packIndex)
    return {
      models = {
        text = Engine.Localize("MPUI_MAP_CATEGORY_" .. categoryName .. "_CAPS"),
        buttonText = Engine.Localize("MPUI_MAP_CATEGORY_" .. categoryName .. "_CAPS"),
        image = "playlist_map",
        description = Engine.Localize("MPUI_MAP_CATEGORY_" .. categoryName .. "_DESC"),
      },
      properties = {
        category = packIndex,
        selectIndex = currentMapDlcPack == packIndex,
      },
    }
  end

  CoD.mapsTable = Engine.GetGDTMapsTable()

  local HasMapsInPack = function(packIndex)
    for mapName, mapData in pairs(CoD.mapsTable) do
      if
        mapData.session_mode == CoD.gameModeEnum
        and GetMapDlcPackCategory(mapName) == packIndex
        and (ShowPurchasableMap(controller, mapName) or Engine.IsMapValid(mapName))
      then
        return true
      end
    end
    return false
  end

  if CoD.isCampaign == true then
    table.insert(categories, CreateCategoryEntry("missions", CoD.CONTENT_ORIGINAL_MAP_INDEX))
    table.insert(categories, CreateCategoryEntry("dev", CoD.CONTENT_DEV_MAP_INDEX))
  else
    table.insert(categories, CreateCategoryEntry("standard", CoD.CONTENT_ORIGINAL_MAP_INDEX))
    if not Dvar.ui_execdemo:get() and HasMapsInPack(DLC_PACK_INDEX) then
      table.insert(categories, CreateCategoryEntry("dlc", DLC_PACK_INDEX))
    end
    if not Dvar.ui_execdemo:get() and HasMapsInPack(DLC_BONUS_PACK_INDEX) then
      table.insert(categories, CreateCategoryEntry("dlc_bonus", DLC_BONUS_PACK_INDEX))
    end
    if Mods_Enabled() then --and Engine.Mods_Lists_GetInfoEntries( LuaEnums.USERMAP_BASE_PATH, 0, Engine.Mods_Lists_GetInfoEntriesCount( LuaEnums.USERMAP_BASE_PATH ) ) ~= nil then
      local modEntries = Engine.Mods_Lists_GetInfoEntries(
        LuaEnums.USERMAP_BASE_PATH,
        0,
        Engine.Mods_Lists_GetInfoEntriesCount(LuaEnums.USERMAP_BASE_PATH)
      )
      if modEntries then
        for i = 0, #modEntries, 1 do
          local modEntry = modEntries[i]
          if LUI.startswith(modEntry.internalName, "mp_") then
            table.insert(categories, CreateCategoryEntry("mods", CoD.CONTENT_MODS_INDEX))
          end
        end
      end
    end
  end
  return categories
end, true)
