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

DataSources.OptionPCVoiceInputDevice = DataSourceHelpers.ListSetup("PC.OptionPCVoiceInputDevice", function()
  local devices = {
    {
      models = {
        value = "",
        valueDisplay = "Default",
      },
    },
  }
  for _, name in ipairs(game.getvoiceinputdevices()) do
    table.insert(devices, {
      models = {
        value = name,
        valueDisplay = #name > 26 and name:sub(1, 23) .. "..." or name,
      },
    })
  end
  return devices
end, true)

if DataSources.OptionVoiceVoice then
  local widgetTypeForItem = DataSources.OptionVoiceVoice.getWidgetTypeForItem
  local function option(models, properties)
    return { models = models, properties = properties }
  end
  DataSources.OptionVoiceVoice = DataSourceHelpers.ListSetup("PC.OptionVoiceVoice", function()
    return {
      option({
        label = "PLATFORM_VOICE_CHAT",
        description = "PLATFORM_VOICECHAT_DESC",
        profileVarName = "cl_voice",
        widgetType = "checkbox",
      }, CoD.PCUtil.OptionsGenericCheckboxProperties),
      option({
        label = "PLATFORM_VOICECHAT_VOLUME",
        description = "PLATFORM_VOICECHAT_VOLUME_DESC",
        profileVarName = "snd_voicechat_volume",
        profileType = "user",
        lowValue = 0,
        highValue = 1,
        widgetType = "slider",
      }, CoD.PCUtil.OptionsGenericSliderProperties),
      option({
        label = "PLATFORM_VOICECHAT_RECORD_LEVEL",
        description = "PLATFORM_VOICECHAT_RECORD_LEVEL_DESC",
        profileVarName = "snd_voicechat_record_level",
        profileType = "user",
        lowValue = 0,
        highValue = 1,
        widgetType = "slider",
      }, CoD.PCUtil.OptionsGenericSliderProperties),
      option({ widgetType = "spacer", height = 32 }),
      option({
        label = "PLATFORM_PUSH_TO_TALK",
        description = "PLATFORM_PUSH_TO_TALK_DESC",
        profileVarName = "snd_voicechat_open_mic",
        profileType = "user",
        lowValue = 1,
        highValue = 0,
        widgetType = "checkbox",
      }, CoD.PCUtil.OptionsGenericCheckboxProperties),
      option({
        label = "Input Device",
        description = "Microphone used for voice chat.",
        profileVarName = "voice_input_device",
        profileType = "function",
        getFunction = function()
          return Dvar.voice_input_device:get()
        end,
        setFunction = function(_, value)
          Engine.SetDvar("voice_input_device", value)
        end,
        datasource = "OptionPCVoiceInputDevice",
        widgetType = "dropdown",
      }, CoD.PCUtil.OptionsGenericDropdownProperties),
      option({
        label = "MENU_MUTE_ALL_EXCEPT_PARTY_OPTION",
        description = "MENU_MUTE_ALL_EXCEPT_PARTY_DESC",
        profileVarName = "mute_all_except_party",
        profileType = "user",
        datasource = "OptionPCVoiceMuteAllExceptParty",
        widgetType = "dropdown",
      }, CoD.PCUtil.OptionsGenericDropdownProperties),
    }
  end, true)
  DataSources.OptionVoiceVoice.getWidgetTypeForItem = widgetTypeForItem
end

if CoD.LobbyBase and CoD.LobbyBase.ShouldBeginLAN then
  local shouldBeginLAN = CoD.LobbyBase.ShouldBeginLAN
  CoD.LobbyBase.ShouldBeginLAN = function(controller)
    if
      not Engine.IsSignedInToLive(controller)
      and not Dvar.lobby_forceOffline:get()
      and not CoD.LobbyBase.FailedDWConnection
      and not CoD.LobbyBase.ChunkAllDownloading()
    then
      return false
    end
    return shouldBeginLAN(controller)
  end
end

if ScoreboardMuteButtonPromptHidden then
  local muteButtonPromptHidden = ScoreboardMuteButtonPromptHidden
  ScoreboardMuteButtonPromptHidden = function(element, controller)
    local hidden = muteButtonPromptHidden(element, controller)
    if not hidden or not Engine.IsInGame() or not ScoreboardVisible(controller) or CoD.IsShoutcaster(controller) then
      return hidden
    end
    local clientNum = CoD.SafeGetModelValue(element:getModel(), "clientNum")
    return clientNum == nil
      or clientNum < 0
      or clientNum == Engine.GetClientNum(controller)
      or CoD.SafeGetModelValue(Engine.GetModelForController(controller), "scoreboardInfo.muteButtonPromptVisible")
        ~= true
  end
end
