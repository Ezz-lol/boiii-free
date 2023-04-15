if Engine.GetCurrentMap() ~= "core_frontend" then
	return
end

function ShouldUnloadMod(arg0)
	return false
end

-- Load usermaps mod if custom map selected
local mustLoadUsermaps = function(element, controller)
	if CoD.perController[controller].mapCategory == CoD.CONTENT_MODS_INDEX then
		if not Engine.IsUsingMods() then
			LuaUtils.Mods_LoadMod("usermaps") -- LuaUtils.Mods_LoadMod(element.loadModName)

			CoD.OverlayUtility.ShowToast("DefaultState", Engine.Localize("MENU_MODS_CAPS"), Engine.Localize("PLATFORM_MODS_LOADED") .. " " .. element.mapName)
		end
	end
end

function MapSelected(element, controller)
	SetMap(controller, element.mapName, false)
	mustLoadUsermaps(element, controller)
end
