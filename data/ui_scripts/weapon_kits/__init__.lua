if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

for _, file in ipairs({
  "ui/uieditor/menus/Craft/WeaponBuildKits/WeaponBuildKitsCategorySelect.lua",
  "ui/uieditor/widgets/Craft/WeaponBuildKits/WeaponBuildKitsVariantAttachments.lua",
}) do
  if game.ismodrawfile(file) then
    return
  end
end

if __boiiiWeaponKitsInstalled then
  return
end
__boiiiWeaponKitsInstalled = true

require("ui.uieditor.menus.Craft.WeaponBuildKits.WeaponBuildKitsCategorySelect")
require("ui.uieditor.widgets.Craft.WeaponBuildKits.WeaponBuildKitsVariantAttachments")
require("ui.uieditor.widgets.Lobby.Common.FE_List1ButtonLarge_PH")

local ZOMBIES = Enum.eModes.MODE_ZOMBIES
local CAMO_GROUP = Enum.eWeaponOptionGroup.WEAPONOPTION_GROUP_CAMO
local MAX_OTHER_ATTACHMENTS = 5
local EMPTY = CoD.CraftUtility.Gunsmith.EMPTY_ITEM_INDEX

local WEAPONS_WITHOUT_CAMOS = {
  smg_ppsh = true,
}

local function modelValue(model, name)
  local child = model and Engine.GetModel(model, name)
  return child and Engine.GetModelValue(child)
end

local function hasCamos(weaponIndex)
  return not WEAPONS_WITHOUT_CAMOS[Engine.GetItemRef(weaponIndex, ZOMBIES)]
end

local function shuffle(items)
  for index = #items, 2, -1 do
    local other = math.random(index)
    items[index], items[other] = items[other], items[index]
  end
  return items
end

local function isKitWeapon(model)
  local group = modelValue(model, "group")
  local slot = modelValue(model, "loadoutSlot")
  return group and group:match("^weapon_") and group ~= "weapon_grenade" and (slot == "primary" or slot == "secondary")
end

local function attachmentsByType(weaponIndex)
  local optics, others = {}, {}
  for index = 1, Engine.GetNumAttachments(weaponIndex, ZOMBIES) - 1 do
    local attachment = Engine.GetItemAttachment(weaponIndex, index, ZOMBIES)
    if attachment > EMPTY and Engine.GetAttachmentAllocationCost(weaponIndex, index, ZOMBIES) > 0 then
      table.insert(Engine.IsOptic(weaponIndex, index, ZOMBIES) and optics or others, attachment)
    end
  end
  return optics, others
end

local function randomAttachments(weaponIndex)
  local optics, others = attachmentsByType(weaponIndex)

  local chosen = { #optics > 0 and optics[math.random(#optics)] or EMPTY }
  local function compatible(attachment)
    for _, existing in ipairs(chosen) do
      if existing > EMPTY and not Engine.AreAttachmentsCompatibleByAttachmentID(existing, attachment) then
        return false
      end
    end
    return true
  end

  for _, attachment in ipairs(shuffle(others)) do
    if #chosen > MAX_OTHER_ATTACHMENTS then
      break
    end
    if compatible(attachment) then
      table.insert(chosen, attachment)
    end
  end
  return chosen
end

local function randomCamo(controller, weaponIndex)
  if not hasCamos(weaponIndex) then
    return CoD.CraftUtility.Gunsmith.CAMO_NONE
  end
  local options = {}
  CoD.GetWeaponOptionsTableForCategory(controller, options, CAMO_GROUP, weaponIndex, 0, false)
  local camos = {}
  for _, model in ipairs(options.camo or {}) do
    local camo = modelValue(model, "itemIndex")
    local filter = modelValue(model, "filterMode")
    if
      camo
      and camo ~= CoD.CraftUtility.Gunsmith.CAMO_NONE
      and filter
      and filter ~= ""
      and not Engine.IsItemOptionLocked(controller, weaponIndex, Engine.GetWeaponOptionGroupIndex(camo, CAMO_GROUP))
    then
      table.insert(camos, camo)
    end
  end
  return #camos > 0 and camos[math.random(#camos)] or CoD.CraftUtility.Gunsmith.CAMO_NONE
end

local function showAllCamoTabs(controller)
  local restricted = { CoD.CACUtility.WeaponsWithNoZMCamos, CoD.CACUtility.WeaponsWithNoCPCamos }
  local removed = {}
  local unlockables = CoD.GetUnlockablesTable(controller, nil, ZOMBIES)
  for _, model in ipairs(unlockables and unlockables.filterList or {}) do
    local ref = modelValue(model, "ref")
    if ref and isKitWeapon(model) and not WEAPONS_WITHOUT_CAMOS[ref] then
      for _, list in ipairs(restricted) do
        if list[ref] then
          list[ref] = nil
          table.insert(removed, { list = list, ref = ref })
        end
      end
    end
  end
  return function()
    for _, entry in ipairs(removed) do
      entry.list[entry.ref] = true
    end
  end
end

local function createKit(root, weaponIndex)
  local variantIndex = CoD.CraftUtility.Gunsmith.GetEmptyVariantIndex()
  local variant = variantIndex and root.variant[variantIndex]
  if not variant then
    return nil
  end
  variant.variantName:set("")
  variant.variantIndex:set(variantIndex)
  variant.reticleIndex:set(CoD.CraftUtility.Gunsmith.RETICLE_NONE)
  variant.paintjobSlot:set(Enum.CustomizationPaintjobInvalidID.CUSTOMIZATION_INVALID_PAINTJOB_SLOT)
  variant.paintjobIndex:set(Enum.CustomizationPaintjobInvalidID.CUSTOMIZATION_INVALID_PAINTJOB_INDEX)
  variant.sortIndex:set(CoD.CraftUtility.Gunsmith.GetHighestSortIndexForWeapon(weaponIndex) + 1)
  variant.weaponIndex:set(weaponIndex)
  variant.createTime:set(Engine.GetCurrentUTCTimeStr())
  CoD.CraftUtility.Gunsmith.CachedVariants[variantIndex + 1].weaponIndex = weaponIndex
  return variant
end

local function randomizeKits(controller)
  local fileType = CoD.CraftUtility.Gunsmith.GetFileType()
  local root = CoD.CraftUtility.GetDDLRoot(controller, fileType)
  if not root or not root.variant or not CoD.CraftUtility.Gunsmith.ParseDDL(controller, fileType) then
    return 0
  end

  local kits = {}
  for _, cached in ipairs(CoD.CraftUtility.Gunsmith.CachedVariants) do
    kits[cached.weaponIndex] = root.variant[cached.variantIndex]
  end

  local count = 0
  local unlockables = CoD.GetUnlockablesTable(controller, nil, ZOMBIES)
  for _, model in ipairs(unlockables and unlockables.filterList or {}) do
    local weaponIndex = modelValue(model, "itemIndex")
    if weaponIndex and isKitWeapon(model) then
      local variant = kits[weaponIndex] or createKit(root, weaponIndex)
      if variant then
        local attachments = randomAttachments(weaponIndex)
        for slot = 1, CoD.CraftUtility.Gunsmith.MAX_ATTACHMENTS do
          variant.attachment[slot - 1]:set(attachments[slot] or EMPTY)
          variant.attachmentVariant[slot - 1]:set(EMPTY)
        end
        variant.camoIndex:set(randomCamo(controller, weaponIndex))
        count = count + 1
      end
    end
  end

  if count > 0 then
    CoD.CraftUtility.UploadAndParseBuffer(controller, Enum.StorageFileType.STORAGE_GUNSMITH)
  end
  return count
end

local function randomizeAndNotify(controller)
  local count = randomizeKits(controller)
  if count > 0 then
    CoD.OverlayUtility.ShowToast(
      "BlackMarketEquipped",
      "Randomized " .. count .. " weapon kits.",
      nil,
      "uie_img_t7_menu_startmenu_identity_gunsmith"
    )
  else
    LuaUtils.UI_ShowErrorMessageDialog(controller, "Could not randomize the weapon kits. Try again.")
  end
end

local createCategorySelect = LUI.createMenu.WeaponBuildKitsCategorySelect
LUI.createMenu.WeaponBuildKitsCategorySelect = function(controller)
  local menu = createCategorySelect(controller)
  local restoreCamoTabs = showAllCamoTabs(controller)
  local button = CoD.FE_List1ButtonLarge_PH.new(menu, controller)
  button:setLeftRight(true, false, 117, 437)
  button:setTopBottom(true, false, 612, 644)
  button.btnDisplayText:setText(Engine.Localize("RANDOMIZE WEAPON KITS"))
  button.btnDisplayTextStroke:setText(Engine.Localize("RANDOMIZE WEAPON KITS"))
  button.FEButtonWarning:setAlpha(0)
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
    randomizeAndNotify(controller)
    return true
  end, function(_, currentMenu)
    CoD.Menu.SetButtonLabel(currentMenu, Enum.LUIButton.LUI_KEY_XBA_PSCROSS, "MENU_SELECT")
    return true
  end, false)
  menu:addElement(button)
  menu.RandomizeWeaponKits = button
  menu.categoryList.navigation = { down = button }
  button.navigation = { up = menu.categoryList }
  button.id = "RandomizeWeaponKits"
  LUI.OverrideFunction_CallOriginalSecond(menu, "close", function(element)
    element.RandomizeWeaponKits:close()
    restoreCamoTabs()
  end)
  return menu
end

local createVariantAttachments = CoD.WeaponBuildKitsVariantAttachments.new
CoD.WeaponBuildKitsVariantAttachments.new = function(menu, controller)
  local self = createVariantAttachments(menu, controller)
  local function setVisible(visible, header, ...)
    for _, element in ipairs({ header, ... }) do
      element.hiddenByWeaponKits = not visible
      if not element.weaponKitsSetAlpha then
        element.weaponKitsSetAlpha = element.setAlpha
        element.setAlpha = function(target, alpha, ...)
          return target:weaponKitsSetAlpha(target.hiddenByWeaponKits and 0 or alpha, ...)
        end
      end
      element:setAlpha(1)
      if element ~= header then
        if visible then
          element:makeFocusable()
        else
          element:makeNotFocusable()
        end
      end
    end
  end
  self:linkToElementModel(self, "weaponIndex", true, function(model)
    local weaponIndex = Engine.GetModelValue(model)
    local optics, others = {}, {}
    if weaponIndex then
      optics, others = attachmentsByType(weaponIndex)
    end
    setVisible(not weaponIndex or #optics > 0, self.opticCategoryHeader, self.Optic)
    setVisible(
      not weaponIndex or #others > 0,
      self.attachCategoryHeader,
      self.Attachment1,
      self.Attachment2,
      self.Attachment3
    )
    setVisible(not weaponIndex or hasCamos(weaponIndex), self.camoCategoryHeader, self.Camo)
  end)
  return self
end
