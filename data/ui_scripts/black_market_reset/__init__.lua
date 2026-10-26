if Engine.GetCurrentMap() ~= "core_frontend" or type(game.resetblackmarket) ~= "function" then
  return
end

if game.ismodrawfile("ui/uieditor/menus/BlackMarket.lua") then
  return
end

if __boiiiBlackMarketResetInstalled then
  return
end
__boiiiBlackMarketResetInstalled = true

require("ui.uieditor.menus.BlackMarket")

-- With Local Currency, owned items are in the local database rather than the game's inventory, so the game's RECENT
-- tab and screen read them from there
local getLootItemCount = Engine.GetLootItemCount
local getLootItems = Engine.GetLootItems

local function isLocalLoot(mode)
  return mode == Enum.eModes.MODE_MULTIPLAYER and Dvar.cg_local_currency:get()
end

Engine.GetLootItemCount = function(controller, mode)
  if not isLocalLoot(mode) then
    return getLootItemCount(controller, mode)
  end
  return #game.getblackmarketdrops()
end

Engine.GetLootItems = function(controller, mode, first, count)
  if not isLocalLoot(mode) then
    return getLootItems(controller, mode, first, count)
  end
  local drops = game.getblackmarketdrops()
  local items = {}
  for index = first + 1, math.min(first + count, #drops) do
    table.insert(items, { id = drops[index].id, datetime = drops[index].time, count = 1 })
  end
  return items
end

CoD.OverlayUtility.AddSystemOverlay("ResetBlackMarketConfirmation", {
  menuName = "SystemOverlay_Full",
  title = "RESET BLACK MARKET?",
  description = "Locks every item you've collected again. Cryptokeys are kept.",
  categoryType = CoD.OverlayUtility.OverlayTypes.GenericMessage,
  listDatasource = function()
    DataSources.ResetBlackMarketConfirmationList = DataSourceHelpers.ListSetup(
      "ResetBlackMarketConfirmationList",
      function()
        return {
          {
            models = { displayText = "RESET COLLECTION", disabled = false },
            properties = {
              action = function(_, _, controller, _, menu)
                if game.resetblackmarket(controller) then
                  GoBack(menu, controller)
                  CoD.OverlayUtility.ShowToast(
                    "BlackMarketEquipped",
                    "COLLECTION RESET",
                    "Every item locked",
                    "uie_t7_blackmarket_promo_cryptokeys"
                  )
                else
                  LuaUtils.UI_ShowErrorMessageDialog(
                    controller,
                    "Could not reset the Black Market. Turn on Local Currency in Stats Settings and try again."
                  )
                end
              end,
            },
          },
          {
            models = { displayText = "MENU_CANCEL_CAPS", disabled = false },
            properties = {
              -- Cancel is focused first, so a quick double press doesn't reset
              selectIndex = true,
              action = function(_, _, controller, _, menu)
                GoBack(menu, controller)
              end,
            },
          },
        }
      end,
      true
    )
    return "ResetBlackMarketConfirmationList"
  end,
})

-- A RESET tab after the others. The list's items come from a function local to the Black Market menu, so the tab is
-- added after it's built, the same way the engine's list helper adds each item.
local tabs = DataSources.BlackMarketList
local originalPrepare = tabs.prepare
tabs.prepare = function(controller, list, filter)
  originalPrepare(controller, list, filter)
  local items = list[list.customDataSourceHelper]
  if not items then
    return
  end
  -- Crates don't keep duplicates, so there's nothing to burn
  if Dvar.cg_local_currency:get() then
    for index = #items, 1, -1 do
      if Engine.GetModelValue(Engine.GetModel(items[index].model, "displayText")) == "MPUI_BM_DUPLICATES" then
        table.remove(items, index)
      end
    end
  end
  local root = ListHelper_GetListHelperModel(list, true)
  local index = #items + 1
  local oldModel = Engine.GetModel(root, index)
  if oldModel then
    Engine.UnsubscribeAndFreeModel(oldModel)
  end
  local model = Engine.CreateModel(root, index)
  ListHelper_CreateModelsFromTable(model, { displayText = "RESET", image = "uie_ui_codpoints_symbol_32x32" })
  table.insert(items, {
    model = model,
    properties = {
      action = function(_, _, actionController, _, menu)
        OpenSystemOverlay(menu, menu, actionController, "ResetBlackMarketConfirmation")
      end,
    },
  })
end
