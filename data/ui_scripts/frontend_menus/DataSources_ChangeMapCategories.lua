local f0_local0 = function(f1_arg0, f1_arg1)
	if not CoD.useMouse then
		return
	else
		LUI.OverrideFunction_CallOriginalFirst(f1_arg0, "setState", function(element, controller)
			if IsSelfInState(f1_arg0, "SelectingMap") then
				f1_arg0.mapList:setMouseDisabled(false)
				f1_arg0.mapCategoriesList:setMouseDisabled(true)
				f1_arg0.m_categorySet = false
			else
				f1_arg0.mapList:setMouseDisabled(true)
				f1_arg0.mapCategoriesList:setMouseDisabled(false)
			end
		end)
		f1_arg0.mapList:setMouseDisabled(true)
		f1_arg0.mapList:registerEventHandler("leftclick_outside", function(element, event)
			if IsSelfInState(f1_arg0, "SelectingMap") and f1_arg0.m_categorySet then
				CoD.PCUtil.SimulateButtonPress(f1_arg1, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE)
			end
			f1_arg0.m_categorySet = true
			return true
		end)
	end
end

local PostLoadFunc = function(f4_arg0, f4_arg1)
	f0_local0(f4_arg0, f4_arg1)
end

local f0_local2 = 10000
local f0_local3 = 10001
local f0_local4 = function(f5_arg0)
	local f5_local0 = CoD.mapsTable[f5_arg0]
	if CoD.CONTENT_DLC6_INDEX <= f5_local0.dlc_pack or f5_arg0 == "mp_redwood_ice" or f5_arg0 == "mp_veiled_heyday" then
		return f0_local3
	elseif f5_local0.dlc_pack > 0 then
		return f0_local2
	else
		return f5_local0.dlc_pack
	end
end

DataSources.ChangeMapCategories = DataSourceHelpers.ListSetup("ChangeMapCategories", function(f6_arg0)
	local f6_local0 = {}
	local f6_local1 = CoD.GetMapValue(Engine.DvarString(nil, "ui_mapname"), "dlc_pack", CoD.CONTENT_ORIGINAL_MAP_INDEX)
	local f6_local2 = function(f7_arg0, f7_arg1)
		return {
			models = {
				text = Engine.Localize("MPUI_MAP_CATEGORY_" .. f7_arg0 .. "_CAPS"),
				buttonText = Engine.Localize("MPUI_MAP_CATEGORY_" .. f7_arg0 .. "_CAPS"),
				image = "playlist_map",
				description = Engine.Localize("MPUI_MAP_CATEGORY_" .. f7_arg0 .. "_DESC")
			},
			properties = {
				category = f7_arg1,
				selectIndex = f6_local1 == f7_arg1
			}
		}
	end

	CoD.mapsTable = Engine.GetGDTMapsTable()
	local f6_local3 = function(f8_arg0)
		for f8_local3, f8_local4 in pairs(CoD.mapsTable) do
			if f8_local4.session_mode == CoD.gameModeEnum and f0_local4(f8_local3) == f8_arg0 and (ShowPurchasableMap(f6_arg0, f8_local3) or Engine.IsMapValid(f8_local3)) then
				return true
			end
		end
		return false
	end

	if CoD.isCampaign == true then
		table.insert(f6_local0, f6_local2("missions", CoD.CONTENT_ORIGINAL_MAP_INDEX))
		table.insert(f6_local0, f6_local2("dev", CoD.CONTENT_DEV_MAP_INDEX))
	else
		table.insert(f6_local0, f6_local2("standard", CoD.CONTENT_ORIGINAL_MAP_INDEX))
		if not Dvar.ui_execdemo:get() and f6_local3(f0_local2) then
			table.insert(f6_local0, f6_local2("dlc", f0_local2))
		end
		if not Dvar.ui_execdemo:get() and f6_local3(f0_local3) then
			table.insert(f6_local0, f6_local2("dlc_bonus", f0_local3))
		end
		if Mods_Enabled() then --and Engine.Mods_Lists_GetInfoEntries( LuaEnums.USERMAP_BASE_PATH, 0, Engine.Mods_Lists_GetInfoEntriesCount( LuaEnums.USERMAP_BASE_PATH ) ) ~= nil then
			local f9_local11 = Engine.Mods_Lists_GetInfoEntries(LuaEnums.USERMAP_BASE_PATH, 0,
			Engine.Mods_Lists_GetInfoEntriesCount(LuaEnums.USERMAP_BASE_PATH))
			if f9_local11 then
				for f9_local12 = 0, #f9_local11, 1 do
					local f9_local17 = f9_local11[f9_local12]
					if LUI.startswith(f9_local17.internalName, "mp_") then
						table.insert(f6_local0, f6_local2("mods", CoD.CONTENT_MODS_INDEX))
					end
				end
			end
		end
	end
	return f6_local0
end, true)
