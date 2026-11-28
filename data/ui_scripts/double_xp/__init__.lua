if CoD.isFrontend then
  return
end

-- The stock 2XP checks hide the badges whenever any mod is loaded, but the XP
-- scales apply with mods too
local function isDoubleScale(controller, getScale)
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

-- The HUD only checks IsDoubleXP during a double XP weekend, which the game
-- never reports while a mod is loaded
local isDoubleXPWeekend = Engine.IsDoubleXPWeekend
Engine.IsDoubleXPWeekend = function(controller, ...)
  return IsDoubleXP(controller) or isDoubleXPWeekend(controller, ...)
end
