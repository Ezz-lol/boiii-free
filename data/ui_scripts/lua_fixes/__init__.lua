-- Fix LUI_NULL_FUNCTION messages
function Engine.PIXBeginEvent() end
function Engine.PIXEndEvent() end

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
      CoD.AARUtilityZM.SetupUIModels = function(arg0)
        local ok, err = pcall(origSetup, arg0)
        if not ok then
          pcall(function()
            local root = Engine.GetModelForController(arg0)
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
      end
    end
    local origGetMatch = CoD.AARUtilityZM.GetMatchStat
    if origGetMatch then
      CoD.AARUtilityZM.GetMatchStat = function(a, b)
        local ok, r = pcall(origGetMatch, a, b)
        if ok and r then
          return r
        end
        return 0
      end
    end
    local origGetXP = CoD.AARUtilityZM.GetXPEarnedDuringMatch
    if origGetXP then
      CoD.AARUtilityZM.GetXPEarnedDuringMatch = function(a, b)
        local ok, r = pcall(origGetXP, a, b)
        if ok and r then
          return r
        end
        return 0
      end
    end
  end
end)

pcall(function()
  if CoD.AARUtility then
    local origSetCurr = CoD.AARUtility.SetCurrLevelModels
    if origSetCurr then
      CoD.AARUtility.SetCurrLevelModels = function(...)
        local ok, err = pcall(origSetCurr, ...)
        if not ok then
        end
      end
    end
    local origSetNext = CoD.AARUtility.SetNextLevelModels
    if origSetNext then
      CoD.AARUtility.SetNextLevelModels = function(...)
        local ok, err = pcall(origSetNext, ...)
        if not ok then
        end
      end
    end
    local origDoXP = CoD.AARUtility.DoXPBarAnimation
    if origDoXP then
      CoD.AARUtility.DoXPBarAnimation = function(...)
        local ok, err = pcall(origDoXP, ...)
        if not ok then
        end
      end
    end
  end
end)
