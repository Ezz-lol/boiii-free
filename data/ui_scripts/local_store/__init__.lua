if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

if
  type(Engine.IsUsingMods) == "function"
  and Engine.IsUsingMods()
  and (type(Engine.UsingModsUgcName) ~= "function" or Engine.UsingModsUgcName() ~= "usermaps")
then
  return
end

require("ui.uieditor.widgets.Store.Store_NonFeaturedFrame")
require("ui.uieditor.widgets.CAC.Customization.CACGenericButton")
require("ui.uieditor.menus.ZM.MegaChewFactory")
require("ui.uieditor.menus.ZM.GobbleGumCookbook")

local originalCACGenericButtonNew = CoD.CACGenericButton.new
CoD.BoiiiDistillPaymentButton = InheritFrom(CoD.CACGenericButton)
CoD.BoiiiDistillPaymentButton.new = function(menu, controller)
  local element = originalCACGenericButtonNew(menu, controller)
  element:setClass(CoD.BoiiiDistillPaymentButton)
  local disabledClip = element.clipsPerState.Disabled.DefaultClip
  element.clipsPerState.Disabled.DefaultClip = function(...)
    disabledClip(...)
    element:setAlpha(0.45)
  end
  element.clipsPerState.Disabled.Focus = element.clipsPerState.Disabled.DefaultClip
  local function enabledClip(clip)
    return function(...)
      clip(...)
      element:setAlpha(1)
    end
  end
  for _, stateName in ipairs({ "DefaultState", "ImageVisible" }) do
    local state = element.clipsPerState[stateName]
    if state then
      for clipName, clip in pairs(state) do
        state[clipName] = enabledClip(clip)
      end
    end
  end
  return element
end

local originalGetDistillCooldown = CoD.BubbleGumBuffUtility.GetDistillCooldown
CoD.BubbleGumBuffUtility.GetDistillCooldown = function(controller, distillID)
  if distillID == CoD.BubbleGumBuffUtility.FREE_DISTILL and type(game.getfreedistillcooldown) == "function" then
    return game.getfreedistillcooldown()
  end
  return originalGetDistillCooldown(controller, distillID)
end

local originalDistillModel = DataSources.GobbleGumDistills.getModel
DataSources.GobbleGumDistills.getModel = function(controller)
  local model = originalDistillModel(controller)
  if type(game.getdistillbalance) ~= "function" then
    return model
  end

  local free = game.getdistillbalance(true)
  local paid = game.getdistillbalance(false)
  if free < 0 or paid < 0 then
    return model
  end
  Engine.SetModelValue(Engine.CreateModel(model, "freeDistills"), free)
  Engine.SetModelValue(Engine.CreateModel(model, "paidDistills"), paid)
  Engine.SetModelValue(Engine.CreateModel(model, "totalDistills"), free + paid)
  return model
end

local originalPurchaseProduct = PurchaseProduct
local packAmounts = { 13000, 5000, 2400, 1100, 200 }

local function modelText(element, controller, name)
  local model = element:getModel(controller, name)
  if not model then
    return ""
  end

  return tostring(Engine.GetModelValue(model) or "")
end

local function getPackAmount(element, controller)
  local text = table.concat({
    modelText(element, controller, "productImage"),
    modelText(element, controller, "previewImage"),
    modelText(element, controller, "name"),
    modelText(element, controller, "desc"),
    modelText(element, controller, "skuID"),
  }, " ")

  for _, amount in ipairs(packAmounts) do
    if string.find(text, tostring(amount), 1, true) then
      return amount
    end
  end
end

PurchaseProduct = function(menu, element, controller)
  if CoD.perController[controller].selectedStoreCategory ~= "CODPOINTS" then
    return originalPurchaseProduct(menu, element, controller)
  end

  local amount = getPackAmount(element, controller)
  if not amount then
    LuaUtils.UI_ShowErrorMessageDialog(controller, "Unable to identify this COD Points pack.")
    return
  end

  if type(game.purchasecodpoints) ~= "function" then
    return originalPurchaseProduct(menu, element, controller)
  end

  local balance = game.purchasecodpoints(controller, amount)
  if balance < 0 then
    LuaUtils.UI_ShowErrorMessageDialog(controller, "Local currency is disabled.")
    return
  end

  local currencyModel = Engine.CreateModel(Engine.GetModelForController(controller), "CryptoKeyProgress")
  Engine.SetModelValue(Engine.CreateModel(currencyModel, "codPoints"), balance)
  CoD.OverlayUtility.ShowToast(
    "BlackMarketEquipped",
    Engine.Localize("MENU_PURCHASE_COMPLETE", amount .. " COD Points"),
    nil,
    "uie_t7_icon_codpoints"
  )
end

local purchaseVialsOverlay = CoD.OverlayUtility.Overlays.PurchaseVialsConfirmation
local originalVialsList = purchaseVialsOverlay.listDatasource

purchaseVialsOverlay.listDatasource = function(controller, purchase)
  if purchase.cryptokeyCost then
    return originalVialsList(controller, purchase)
  end

  DataSources.PurchaseVialsConfirmationList = DataSourceHelpers.ListSetup(
    "PurchaseVialsConfirmationList",
    function(_, list)
      if list.purchasingVials then
        return {}
      end

      local function buy(element, _, actionController, params, menu)
        if not game.purchasevials(actionController, params.cost, params.vials) then
          LuaUtils.UI_ShowErrorMessageDialog(actionController, "MPUI_BM_VIALS_PURCHASE_FAILED", "")
          return
        end

        local controllerModel = Engine.GetModelForController(actionController)
        local currencyModel = Engine.CreateModel(controllerModel, "CryptoKeyProgress")
        Engine.SetModelValue(Engine.CreateModel(currencyModel, "codPoints"), Engine.GetCoDPoints(actionController))
        local tokenModel = Engine.CreateModel(controllerModel, "MegaChewTokens")
        Engine.SetModelValue(Engine.CreateModel(tokenModel, "remainingTokens"), Engine.GetZMVials(actionController))

        element:setState("Purchasing")
        element.ZMDiviniumAnimFill:playClip("Start")
        element.ZMDiviniumAnimFill.nextClip = "Loop"
        list.purchasingVials = true
        list.disabled = true
        list:updateDataSource(true)
        menu[CoD.OverlayUtility.GoBackPropertyName] = nil
        element.optionsHorizontal.disabled = true
        element.optionsHorizontal.purchasingVials = true
        element.optionsHorizontal:updateDataSource(true)
        CoD.Menu.UpdateAllButtonPrompts(menu, actionController)
        element.ZMDiviniumAnimFill:registerEventHandler("clip_over", function(animation, event)
          animation:playClip("Stop")
          animation:registerEventHandler("clip_over", function(animation, event)
            animation:registerEventHandler("clip_over", LUI.UIElement.clipOver)
            element:setState("DefaultState")
            GoBack(menu, actionController)
          end)
        end)
      end

      local function cancel(_, _, actionController, _, menu)
        GoBack(menu, actionController)
      end

      return {
        {
          models = {
            displayText = Engine.Localize("ZMUI_PURCHASE_VIALS", purchase.cost),
            vials = purchase.vials,
            disabled = false,
          },
          properties = {
            action = buy,
            actionParam = { vials = purchase.vials, cost = purchase.cost },
            selectIndex = true,
            showStoreButton = false,
          },
        },
        {
          models = { displayText = "MENU_CANCEL_CAPS", disabled = false },
          properties = { action = cancel, selectIndex = false },
        },
      }
    end,
    true
  )
  return "PurchaseVialsConfirmationList"
end

local function refreshDistillModels(controller)
  DataSources.GobbleGumDistills.getModel(controller)
  local currencyModel = Engine.CreateModel(Engine.GetModelForController(controller), "CryptoKeyProgress")
  Engine.SetModelValue(Engine.CreateModel(currencyModel, "codPoints"), Engine.GetCoDPoints(controller))
  local tokenModel = Engine.CreateModel(Engine.GetModelForController(controller), "MegaChewTokens")
  Engine.SetModelValue(Engine.CreateModel(tokenModel, "remainingTokens"), Engine.GetZMVials(controller))
end

local function buyDistills(element, controller, params, menu, list)
  local payment = -1
  if params.distillID ~= CoD.BubbleGumBuffUtility.FREE_DISTILL then
    local vialSku = Engine.DvarInt(nil, "loot_distill_paid_" .. params.distillID .. "_vial_sku")
    payment = params.sku == vialSku and 3 or 0
  end

  if params.distillID == CoD.BubbleGumBuffUtility.FREE_DISTILL and not CanGetFreeGobblegumCookbookUses(controller) then
    return
  end

  if type(game.purchasedistills) ~= "function" or not game.purchasedistills(controller, params.distillID, payment) then
    LuaUtils.UI_ShowErrorMessageDialog(controller, "ZMUI_COOKBOOK_DISTILL_PURCHASE_FAILED", "")
    return
  end

  refreshDistillModels(controller)
  list.buyingDistill = true
  menu[CoD.OverlayUtility.GoBackPropertyName] = nil
  element:setState("BuyingDistill")
  element.BuyingDistillAnimation:playClip("StartLiquefying")
  element.optionsHorizontal.disabled = true
  element.optionsVertical.disabled = true
  list:updateDataSource(true)
  CoD.Menu.UpdateAllButtonPrompts(menu, controller)
  element.BuyingDistillAnimation:registerEventHandler("clip_over", function(animation, event)
    animation:registerEventHandler("clip_over", LUI.UIElement.clipOver)
    if params.distillID == CoD.BubbleGumBuffUtility.FREE_DISTILL then
      element:setState("DefaultState")
      list.disabled = false
      list.buyingDistill = nil
      menu[CoD.OverlayUtility.GoBackPropertyName] = CoD.OverlayUtility.DefaultGoBack()
      element.optionsHorizontal.disabled = nil
      element.optionsVertical.disabled = nil
      list:updateDataSource()
      CoD.Menu.UpdateAllButtonPrompts(menu, controller)
    else
      GoBackAndOpenSystemOverlayOnParent(element, controller, "PurchaseDistills")
    end
  end)
end

local function isDistillPaymentDisabled(controller, params)
  local vialSku = Engine.DvarInt(nil, "loot_distill_paid_" .. params.distillID .. "_vial_sku")
  if params.sku == vialSku then
    return Engine.GetZMVials(controller) < params.vials
  end
  return Engine.GetCoDPoints(controller) < params.codPoints
end

local function wrapDistillDatasource(name, freeOnly)
  local source = DataSources[name]
  if not source or source.localDistillPurchases then
    return
  end

  local originalPrepare = source.prepare
  source.prepare = function(controller, list, filter)
    originalPrepare(controller, list, filter)
    local items = list[list.customDataSourceHelper] or {}
    for _, item in ipairs(items) do
      local properties = item.properties
      local params = properties and properties.actionParam
      if
        params
        and params.distillID
        and (not freeOnly or params.distillID == CoD.BubbleGumBuffUtility.FREE_DISTILL)
      then
        local originalAction = properties.action
        if not freeOnly then
          local disabled = isDistillPaymentDisabled(controller, params)
          properties.disabled = disabled
          Engine.SetModelValue(Engine.CreateModel(item.model, "disabled"), disabled)
          properties.customWidgetOverride = CoD.BoiiiDistillPaymentButton
        end
        properties.action = function(element, _, actionController, actionParams, menu)
          if type(game.purchasedistills) ~= "function" then
            return originalAction(element, nil, actionController, actionParams, menu)
          end
          buyDistills(element, actionController, actionParams, menu, list)
        end
      end
    end
  end
  source.localDistillPurchases = true
end

local purchaseDistillsOverlay = CoD.OverlayUtility.Overlays.PurchaseDistills
local originalDistillsList = purchaseDistillsOverlay.listDatasource
purchaseDistillsOverlay.listDatasource = function(controller, purchase)
  local name = originalDistillsList(controller, purchase)
  wrapDistillDatasource(name, true)
  return name
end

local confirmDistillsOverlay = CoD.OverlayUtility.Overlays.PurchaseDistillsConfirmation
local originalConfirmDistillsList = confirmDistillsOverlay.listDatasource
confirmDistillsOverlay.listDatasource = function(controller, purchase)
  local name = originalConfirmDistillsList(controller, purchase)
  wrapDistillDatasource(name, false)
  return name
end

Engine.CookGobbleGumRecipe = function(controller, recipe, offset)
  if type(game.cookgobblegumrecipe) ~= "function" then
    return false
  end
  local success = game.cookgobblegumrecipe(controller, recipe, offset == CoD.BubbleGumBuffUtility.FreeDistillOffset)
  if success then
    refreshDistillModels(controller)
  end
  return success
end
