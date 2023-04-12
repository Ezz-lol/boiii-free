if Engine.GetCurrentMap() ~= "core_frontend" then
	return
end

function ShouldUnloadMod( arg0 )
	return false
end

-- Load usermaps mod if custom map selected
local MustLoadUsermaps = function( arg0, arg1 )  
	if CoD.perController[arg1].mapCategory == CoD.CONTENT_MODS_INDEX then
		if not Engine.IsUsingMods() then
			LuaUtils.Mods_LoadMod( "usermaps" ) -- LuaUtils.Mods_LoadMod(arg0.loadModName)
	
			local MENU_MODS_CAPS = Engine.Localize( "MENU_MODS_CAPS" )
			local PLATFORM_MODS_LOADED = Engine.Localize( "PLATFORM_MODS_LOADED" )
			CoD.OverlayUtility.ShowToast( "DefaultState", MENU_MODS_CAPS, PLATFORM_MODS_LOADED .. " " ..  arg0.mapName )
		end
	end
end

function MapSelected( arg0, arg1 )
	SetMap( arg1, arg0.mapName, false )
	MustLoadUsermaps( arg0, arg1 )
end
