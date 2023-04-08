local IsGamescomDemo = function()
  return Dvar.ui_execdemo_gamescom:get()
end

local IsBetaDemo = function()
  return Dvar.ui_execdemo_beta:get()
end

local SetButtonState = function(button, state)
  if state == nil then
    return
  elseif state == CoD.LobbyButtons.DISABLED then
    button.disabled = true
  elseif state == CoD.LobbyButtons.HIDDEN then
    button.hidden = true
  end
end

local AddButton = function(controller, options, button, isLargeButton, index)
  if button == nil then
    return
  end

  button.disabled = false
  button.hidden = false
  button.selected = false
  button.warning = false
  if button.defaultState ~= nil then
    if button.defaultState == CoD.LobbyButtons.DISABLED then
      button.disabled = true
    elseif button.defaultState == CoD.LobbyButtons.HIDDEN then
      button.hidden = true
    end
  end
  if button.disabledFunc ~= nil then
    button.disabled = button.disabledFunc(controller)
  end
  if button.visibleFunc ~= nil then
    button.hidden = not button.visibleFunc(controller)
  end
  if IsBetaDemo() then
    SetButtonState(button, button.demo_beta)
  elseif IsGamescomDemo() then
    SetButtonState(button, button.demo_gamescom)
  end
  if button.hidden then
    return
  end
  local lobbyNav = LobbyData.GetLobbyNav()
  if button.selectedFunc ~= nil then
    button.selected = button.selectedFunc(button.selectedParam)
  elseif CoD.LobbyMenus.History[lobbyNav] ~= nil then
    button.selected = CoD.LobbyMenus.History[lobbyNav] == button.customId
  end
  if button.newBreadcrumbFunc then
    local f8_local1 = button.newBreadcrumbFunc
    if type(f8_local1) == "string" then
      f8_local1 = LUI.getTableFromPath(f8_local1)
    end
    if f8_local1 then
      button.isBreadcrumbNew = f8_local1(controller)
    end
  end
  if button.warningFunc ~= nil then
    button.warning = button.warningFunc(controller)
  end
  if button.starterPack == CoD.LobbyButtons.STARTERPACK_UPGRADE then
    button.starterPackUpgrade = true
    if IsStarterPack() then
      button.disabled = false
    end
  end
  if index ~= nil then
    table.insert(options, index, {
      optionDisplay = button.stringRef,
      action = button.action,
      param = button.param,
      customId = button.customId,
      isLargeButton = isLargeButton,
      isLastButtonInGroup = false,
      disabled = button.disabled,
      selected = button.selected,
      isBreadcrumbNew = button.isBreadcrumbNew,
      warning = button.warning,
      requiredChunk = button.selectedParam,
      starterPackUpgrade = button.starterPackUpgrade,
      unloadMod = button.unloadMod
    })
  else
    table.insert(options, {
      optionDisplay = button.stringRef,
      action = button.action,
      param = button.param,
      customId = button.customId,
      isLargeButton = isLargeButton,
      isLastButtonInGroup = false,
      disabled = button.disabled,
      selected = button.selected,
      isBreadcrumbNew = button.isBreadcrumbNew,
      warning = button.warning,
      requiredChunk = button.selectedParam,
      starterPackUpgrade = button.starterPackUpgrade,
      unloadMod = button.unloadMod
    })
  end
end

local AddLargeButton = function(controller, options, button, index)
  AddButton(controller, options, button, true, index)
end

local AddSmallButton = function(controller, options, button, index)
  AddButton(controller, options, button, false, index)
end

local AddSpacer = function(options, index)
  if index ~= nil then
    options[index].isLastButtonInGroup = true
  elseif 0 < #options then
    options[#options].isLastButtonInGroup = true
  end
end

return {
  AddButton = AddButton,
  AddLargeButton = AddLargeButton,
  AddSmallButton = AddSmallButton,
  AddSpacer = AddSpacer
}
