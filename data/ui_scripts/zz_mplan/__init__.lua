-- DEV-ONLY (2026-09-28, not shipped): enable MULTIPLAYER in the offline main
-- menu. Stock CoD.LobbyButtons.MP_LAN.disabledFunc (traced with zz_mpprobe)
-- checks IsMpOwned, GetLobbyNetworkMode, IsShipBuild, IsUsingMods: the retail
-- PC build disables offline MP unless a mod is loaded. This keeps only the
-- ownership check. No io/os, no tracing - nothing held in LUI memory.

if rawget(_G, "__zz_mplan_loaded") then
	return
end
rawset(_G, "__zz_mplan_loaded", true)

if CoD == nil or CoD.LobbyButtons == nil or CoD.LobbyButtons.MP_LAN == nil or Engine == nil or Engine.IsMpOwned == nil then
	return
end
CoD.LobbyButtons.MP_LAN.disabledFunc = function(controller)
	return not Engine.IsMpOwned()
end
