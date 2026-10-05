-- Fix LUI_NULL_FUNCTION messages
function Engine.PIXBeginEvent() end

function Engine.PIXEndEvent() end

Engine.SetDvar("tu5_enableVialsOwed", 0)

if type(GoBackAndOpenOverlayOnParent) == "function" then
  GoBackAndOpenOverlayOnParent = function(menu, overlay, controller)
    return OpenOverlay(GoBack(menu, controller), overlay, controller)
  end
end

local function guarded(owner, name, fallback)
  local original = owner and owner[name]
  if original then
    owner[name] = function(...)
      local ok, result = pcall(original, ...)
      if ok and result ~= nil then
        return result
      end
      return fallback
    end
  end
end

-- XP bar pcall fix
pcall(function()
  if DataSources and DataSources.XPProgressionBar then
    local origGetModel = DataSources.XPProgressionBar.getModel
    if origGetModel then
      DataSources.XPProgressionBar.getModel = function(ctrl)
        local ok, model = pcall(origGetModel, ctrl)
        if ok and model then
          return model
        end
        local root = Engine.CreateModel(Engine.GetGlobalModel(), "XPBarFallback")
        Engine.SetModelValue(Engine.CreateModel(root, "rank"), "1")
        Engine.SetModelValue(Engine.CreateModel(root, "rankIcon"), "")
        Engine.SetModelValue(Engine.CreateModel(root, "xpProgress"), 0)
        return root
      end
    end
  end
end)

-- AAR nil error fixes
pcall(function()
  if CoD.AARUtilityZM then
    local origSetup = CoD.AARUtilityZM.SetupUIModels
    if origSetup then
      CoD.AARUtilityZM.SetupUIModels = function(controller)
        local ok, err = pcall(origSetup, controller)
        if not ok then
          pcall(function()
            local root = Engine.GetModelForController(controller)
            local sm = Engine.CreateModel(root, "aarStats.performanceTabStats")
            local defs = {
              kills = 0,
              rounds = 0,
              headshots = 0,
              revives = 0,
              downs = 0,
              meleeKills = 0,
              pointsPerKill = 0,
              total_points = 0,
              bgbTokensGainedThisGame = 0,
              xpEarnedDuringMatch = 0,
              showBestScoreIcon = 0,
              showBestStyleIcon = 0,
              showBestRoundIcon = 0,
              nextLevel = "1",
              nextLevelIcon = "",
            }
            for k, v in pairs(defs) do
              Engine.SetModelValue(Engine.CreateModel(sm, k), v)
            end
          end)
        end
        pcall(function()
          local root = Engine.GetModelForController(controller)
          local stats = Engine.CreateModel(root, "aarStats.performanceTabStats")
          local gained = Dvar.cg_last_divinium_award and Dvar.cg_last_divinium_award:get() or 0
          Engine.SetModelValue(Engine.CreateModel(stats, "bgbTokensGainedThisGame"), gained)
        end)
      end
    end
    guarded(CoD.AARUtilityZM, "GetMatchStat", 0)
    guarded(CoD.AARUtilityZM, "GetXPEarnedDuringMatch", 0)
  end
end)

guarded(CoD.AARUtility, "SetCurrLevelModels")
guarded(CoD.AARUtility, "SetNextLevelModels")
guarded(CoD.AARUtility, "DoXPBarAnimation")
