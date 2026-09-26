if Engine.GetCurrentMap() ~= "core_frontend" or type(game.resetgobblegums) ~= "function" then
  return
end

if
  type(Engine.IsUsingMods) == "function"
  and Engine.IsUsingMods()
  and (type(Engine.UsingModsUgcName) ~= "function" or Engine.UsingModsUgcName() ~= "usermaps")
then
  return
end

if __boiiiGobbleGumResetInstalled then
  return
end
__boiiiGobbleGumResetInstalled = true

require("ui.uieditor.menus.ZM.BubblegumBuffs")
require("ui.uieditor.widgets.Lobby.Common.FE_List1ButtonLarge_PH")

CoD.OverlayUtility.AddSystemOverlay("ResetGobbleGumsConfirmation", {
  menuName = "SystemOverlay_Full",
  title = "RESET GOBBLEGUMS?",
  description = "This turns off unlimited GobbleGums and resets every earned Mega GobbleGum to zero. Other loot and currencies are not changed.",
  categoryType = CoD.OverlayUtility.OverlayTypes.GenericMessage,
  listDatasource = function()
    DataSources.ResetGobbleGumsConfirmationList = DataSourceHelpers.ListSetup(
      "ResetGobbleGumsConfirmationList",
      function()
        return {
          {
            models = { displayText = "RESET TO ZERO", disabled = false },
            properties = {
              selectIndex = true,
              action = function(_, _, controller, _, menu)
                if game.resetgobblegums(controller) then
                  Engine.SetDvar("cg_unlockall_gobblegums", 0)
                  GoBack(menu, controller)
                  CoD.OverlayUtility.ShowToast(
                    "BlackMarketEquipped",
                    "GobbleGum inventory reset to zero.",
                    nil,
                    "uie_t7_menu_gobblegum_comsumable"
                  )
                else
                  LuaUtils.UI_ShowErrorMessageDialog(
                    controller,
                    "Could not reset GobbleGums. Return to the Zombies menu and try again."
                  )
                end
              end,
            },
          },
          {
            models = { displayText = "MENU_CANCEL_CAPS", disabled = false },
            properties = {
              action = function(_, _, controller, _, menu)
                GoBack(menu, controller)
              end,
            },
          },
        }
      end,
      true
    )
    return "ResetGobbleGumsConfirmationList"
  end,
})

local createBubblegumBuffs = LUI.createMenu.BubblegumBuffs
LUI.createMenu.BubblegumBuffs = function(controller)
  local menu = createBubblegumBuffs(controller)
  local button = CoD.FE_List1ButtonLarge_PH.new(menu, controller)
  button:setLeftRight(true, false, 64, 344)
  button:setTopBottom(true, false, 342, 374)
  button.btnDisplayText:setText(Engine.Localize("RESET GOBBLEGUM INVENTORY"))
  button.btnDisplayTextStroke:setText(Engine.Localize("RESET GOBBLEGUM INVENTORY"))
  button:registerEventHandler("gain_focus", function(element, event)
    local result
    if element.gainFocus then
      result = element:gainFocus(event)
    elseif element.super.gainFocus then
      result = element.super:gainFocus(event)
    end
    CoD.Menu.UpdateButtonShownState(element, menu, controller, Enum.LUIButton.LUI_KEY_XBA_PSCROSS)
    return result
  end)
  button:registerEventHandler("lose_focus", function(element, event)
    if element.loseFocus then
      return element:loseFocus(event)
    elseif element.super.loseFocus then
      return element.super:loseFocus(event)
    end
  end)
  menu:AddButtonCallbackFunction(button, controller, Enum.LUIButton.LUI_KEY_XBA_PSCROSS, "ENTER", function()
    OpenSystemOverlay(menu, menu, controller, "ResetGobbleGumsConfirmation")
    return true
  end, function(_, currentMenu)
    CoD.Menu.SetButtonLabel(currentMenu, Enum.LUIButton.LUI_KEY_XBA_PSCROSS, "MENU_SELECT")
    return true
  end, false)
  menu:addElement(button)
  menu.ResetGobbleGumInventory = button
  menu.FEList1ButtonLargePH0.navigation.down = button
  button.navigation = { up = menu.FEList1ButtonLargePH0 }
  button.id = "ResetGobbleGumInventory"
  LUI.OverrideFunction_CallOriginalSecond(menu, "close", function(element)
    element.ResetGobbleGumInventory:close()
  end)
  return menu
end
