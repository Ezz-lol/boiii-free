if CoD.isFrontend then
  return
end

-- Servers load custom maps through the usermaps mod, and the stock 2XP checks
-- hide the badges whenever any mod is loaded
local function isUsingMod()
  return Engine.IsUsingMods() and Engine.UsingModsUgcName() ~= "usermaps"
end

local function isDoubleScale(controller, getScale)
  if isUsingMod() then
    return false
  end
  if not IsPublicOrLeagueGame() and not (IsZombies() and IsLive()) then
    return false
  end
  return getScale(controller) >= 2
end

IsDoubleXP = function(controller)
  return isDoubleScale(controller, Engine.GetXPScale)
end

IsDoubleWeaponXP = function(controller)
  return isDoubleScale(controller, Engine.GetGunXPScale)
end
