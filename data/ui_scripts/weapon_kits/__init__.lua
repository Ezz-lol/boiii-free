if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

local ZM = Enum.eModes.MODE_ZOMBIES
local MP = Enum.eModes.MODE_MULTIPLAYER
local definitions = {
  { ref = "smg_ak74u", group = "weapon_smg", slot = "primary" },
  { ref = "smg_mp40", group = "weapon_smg", slot = "primary" },
  { ref = "smg_ppsh", group = "weapon_smg", slot = "primary" },
  { ref = "ar_peacekeeper", group = "weapon_assault", slot = "primary" },
  { ref = "ar_an94", group = "weapon_assault", slot = "primary" },
  { ref = "ar_garand", group = "weapon_assault", slot = "primary" },
  { ref = "ar_famas", group = "weapon_assault", slot = "primary" },
  { ref = "ar_m16", group = "weapon_assault", slot = "primary" },
  { ref = "ar_galil", group = "weapon_assault", slot = "primary" },
  { ref = "ar_m14", group = "weapon_assault", slot = "primary" },
  { ref = "lmg_rpk", group = "weapon_lmg", slot = "primary" },
  { ref = "sniper_chargeshot", group = "weapon_sniper", slot = "primary" },
  { ref = "shotgun_energy", group = "weapon_cqb", slot = "primary" },
  { ref = "pistol_shotgun", group = "weapon_pistol", slot = "secondary" },
  { ref = "pistol_energy", group = "weapon_pistol", slot = "secondary" },
  { ref = "pistol_m1911", group = "weapon_pistol", slot = "secondary" },
  { ref = "pistol_standard", group = "weapon_pistol", slot = "secondary" },
  { ref = "launcher_ex41", group = "weapon_launcher", slot = "secondary" },
  { ref = "launcher_multi", group = "weapon_launcher", slot = "secondary" },
  { ref = "special_crossbow", group = "weapon_special", slot = "secondary" },
}

if not __boiii_weapon_kit_originals then
  __boiii_weapon_kit_originals = {
    GetUnlockablesTable = CoD.GetUnlockablesTable,
    GetCustomization = CoD.GetCustomization,
    GetItemRef = Engine.GetItemRef,
    GetItemName = Engine.GetItemName,
    GetNumAttachments = Engine.GetNumAttachments,
    IsOptic = Engine.IsOptic,
    GetItemAttachment = Engine.GetItemAttachment,
    GetAttachmentAllocationCost = Engine.GetAttachmentAllocationCost,
    GetAttachmentRef = Engine.GetAttachmentRef,
    GetAttachmentDesc = Engine.GetAttachmentDesc,
    GetAttachmentUniqueImage = Engine.GetAttachmentUniqueImageByAttachmentIndex,
    IsItemLocked = Engine.IsItemLocked,
    IsItemLockedForAll = Engine.IsItemLockedForAll,
    IsCACItemLocked = IsCACItemLocked,
    IsCACItemLockedOrNotPurchased = IsCACItemLockedOrNotPurchased,
    IsItemRefLocked = IsItemRefLocked,
    IsGunsmithItemWeaponOptionLocked = IsGunsmithItemWeaponOptionLocked,
    BMIsItemLocked = CoD.BlackMarketUtility and CoD.BlackMarketUtility.IsItemLocked,
    BMGetItemQuantity = CoD.BlackMarketUtility and CoD.BlackMarketUtility.GetItemQuantity,
    BMIsUnreleased = CoD.BlackMarketUtility and CoD.BlackMarketUtility.IsUnreleasedBlackMarketItem,
    WeaponOptionNewItemCount = Engine.WeaponOptionNewItemCount,
    WeaponOptionNewModeAgnosticItemCount = Engine.WeaponOptionNewModeAgnosticItemCount,
    GetGunsmithWeaponOptionsTable = CoD.GetGunsmithWeaponOptionsTable,
    Gunsmith_FocusCamo = Gunsmith_FocusCamo,
  }
end

local original = __boiii_weapon_kit_originals
original.GetAttachmentUniqueImage = original.GetAttachmentUniqueImage
  or Engine.GetAttachmentUniqueImageByAttachmentIndex
local weaponsByRef = {}
local weaponsByIndex = {}

local function getRef(index, mode)
  local ok, ref = pcall(original.GetItemRef, index, mode)
  if ok and ref and ref ~= "" then
    return ref
  end
end

local function findIndex(ref, mode)
  for index = 1, 255 do
    if getRef(index, mode) == ref then
      return index
    end
  end
end

for _, definition in ipairs(definitions) do
  definition.mpIndex = findIndex(definition.ref, MP)
  definition.zmIndex = findIndex(definition.ref, ZM)
  definition.index = definition.zmIndex or definition.mpIndex
  weaponsByRef[definition.ref] = definition
  if definition.index then
    weaponsByIndex[definition.index] = definition
  end
end

local function resolveAttachmentLookup(index, mode)
  local weapon = weaponsByIndex[index]
  if weapon and weapon.mpIndex and not weapon.zmIndex then
    return weapon.mpIndex, MP
  end
  if weapon and mode == MP and weapon.mpIndex and weapon.mpIndex ~= index then
    return weapon.mpIndex, MP
  end
  return index, mode
end

local function getModelValue(model, name)
  if not model then
    return nil
  end
  local child = Engine.GetModel(model, name)
  if not child then
    return nil
  end
  return Engine.GetModelValue(child)
end

local function setModelValue(model, name, value)
  Engine.SetModelValue(Engine.CreateModel(model, name), value)
end

local function containsRef(items, ref)
  if not items then
    return false
  end
  for _, model in ipairs(items) do
    if getModelValue(model, "ref") == ref then
      return true
    end
  end
  return false
end

local function appendUnique(items, model, ref)
  if not containsRef(items, ref) then
    table.insert(items, model)
  end
end

local function populateWeaponAttributes(weapon, model)
  if not weapon.mpIndex then
    return
  end
  local unlockables = Engine.CreateModel(Engine.GetGlobalModel(), "Unlockables")
  local target = Engine.CreateModel(unlockables, weapon.index .. ".weaponAttributes")
  local source = Engine.GetModel(model, "weaponAttributes")
  for _, attribute in ipairs({ "damage", "range", "fireRate", "accuracy" }) do
    setModelValue(target, attribute, getModelValue(source, attribute) or 0)
  end
end

local function allowCamos(ref)
  if CoD.CACUtility.WeaponsWithNoCPCamos then
    CoD.CACUtility.WeaponsWithNoCPCamos[ref] = nil
  end
  if CoD.CACUtility.WeaponsWithNoMPCamos then
    CoD.CACUtility.WeaponsWithNoMPCamos[ref] = nil
  end
  if CoD.CACUtility.WeaponsWithNoZMCamos then
    CoD.CACUtility.WeaponsWithNoZMCamos[ref] = nil
  end
  if CoD.BlackMarketUtility and CoD.BlackMarketUtility.WeaponsWithNoBMCamos then
    CoD.BlackMarketUtility.WeaponsWithNoBMCamos[ref] = nil
  end
end

local function dvarEnabled(name)
  local dvar = Dvar[name]
  return dvar and dvar:get() == true
end

local function lootUnlocked()
  return dvarEnabled("cg_unlockall_loot") or dvarEnabled("cg_unlockall_purchases")
end

if original.BMIsUnreleased then
  CoD.BlackMarketUtility.IsUnreleasedBlackMarketItem = function(ref, ...)
    if lootUnlocked() or weaponsByRef[ref] then
      return false
    end
    return original.BMIsUnreleased(ref, ...)
  end
end

CoD.GetUnlockablesTable = function(controller, filter, mode)
  local result = original.GetUnlockablesTable(controller, filter, mode)
  if not result then
    return result
  end
  result.filterList = result.filterList or {}
  for _, weapon in ipairs(definitions) do
    if weapon.index then
      local model
      for _, candidate in ipairs(result.filterList) do
        if getModelValue(candidate, "ref") == weapon.ref then
          model = candidate
          break
        end
      end
      if not model then
        local lookupIndex = weapon.mpIndex or weapon.index
        local weaponMode = weapon.mpIndex and MP or ZM
        local ok, candidate =
          pcall(Engine.GetUnlockableInfoModelByIndex, lookupIndex, "BoiiiWeaponKits." .. weapon.index, weaponMode)
        if ok then
          model = candidate
        end
      end
      if model then
        setModelValue(model, "itemIndex", weapon.index)
        setModelValue(model, "ref", weapon.ref)
        local group = getModelValue(model, "group") or weapon.group
        local slot = getModelValue(model, "loadoutSlot") or weapon.slot
        setModelValue(model, "group", group)
        setModelValue(model, "loadoutSlot", slot)
        setModelValue(model, "itemType", Enum.VoteItemType.VOTE_ITEM_TYPE_ITEM)
        setModelValue(model, "isLocked", false)
        setModelValue(model, "isBMClassified", false)
        setModelValue(model, "isContractClassified", false)
        populateWeaponAttributes(weapon, model)
        result[group] = result[group] or {}
        result[slot] = result[slot] or {}
        appendUnique(result.filterList, model, weapon.ref)
        appendUnique(result[group], model, weapon.ref)
        appendUnique(result[slot], model, weapon.ref)
      end
    end
  end
  return result
end

CoD.GetCustomization = function(controller, key, ...)
  if key == "weapon_ref" then
    local index = original.GetCustomization(controller, "weapon_index")
    local weapon = weaponsByIndex[index]
    if weapon then
      return weapon.ref
    end
  end
  return original.GetCustomization(controller, key, ...)
end

if original.GetGunsmithWeaponOptionsTable then
  CoD.GetGunsmithWeaponOptionsTable = function(controller, table, group, weaponIndex, ...)
    if group == Enum.eWeaponOptionGroup.WEAPONOPTION_GROUP_CAMO then
      local weapon = weaponsByIndex[weaponIndex]
      if weapon and weapon.mpIndex and weapon.zmIndex and weapon.mpIndex ~= weaponIndex then
        weaponIndex = weapon.mpIndex
      end
    end
    return original.GetGunsmithWeaponOptionsTable(controller, table, group, weaponIndex, ...)
  end
end

if original.Gunsmith_FocusCamo then
  Gunsmith_FocusCamo = function(menu, element, controller, ...)
    local index = original.GetCustomization(controller, "weapon_index")
    local weapon = weaponsByIndex[index]
    if weapon and weapon.mpIndex and weapon.zmIndex and weapon.mpIndex ~= index then
      return
    end
    return original.Gunsmith_FocusCamo(menu, element, controller, ...)
  end
end

Engine.GetItemRef = function(index, mode, ...)
  local weapon = weaponsByIndex[index]
  if weapon and (mode == nil or mode == MP or mode == ZM) then
    allowCamos(weapon.ref)
    return weapon.ref
  end
  local ref = original.GetItemRef(index, mode, ...)
  if weaponsByRef[ref] then
    allowCamos(ref)
  end
  return ref
end

if original.GetItemName then
  Engine.GetItemName = function(index, mode, ...)
    index, mode = resolveAttachmentLookup(index, mode)
    return original.GetItemName(index, mode, ...)
  end
end

if original.GetNumAttachments then
  Engine.GetNumAttachments = function(index, mode, ...)
    index, mode = resolveAttachmentLookup(index, mode)
    return original.GetNumAttachments(index, mode, ...)
  end
end

if original.IsOptic then
  Engine.IsOptic = function(index, attachment, mode, ...)
    index, mode = resolveAttachmentLookup(index, mode)
    return original.IsOptic(index, attachment, mode, ...)
  end
end

if original.GetItemAttachment then
  Engine.GetItemAttachment = function(index, attachment, mode, ...)
    index, mode = resolveAttachmentLookup(index, mode)
    return original.GetItemAttachment(index, attachment, mode, ...)
  end
end

if original.GetAttachmentAllocationCost then
  Engine.GetAttachmentAllocationCost = function(index, attachment, mode, ...)
    index, mode = resolveAttachmentLookup(index, mode)
    return original.GetAttachmentAllocationCost(index, attachment, mode, ...)
  end
end

if original.GetAttachmentRef then
  Engine.GetAttachmentRef = function(index, attachment, mode, ...)
    index, mode = resolveAttachmentLookup(index, mode)
    return original.GetAttachmentRef(index, attachment, mode, ...)
  end
end

if original.GetAttachmentDesc then
  Engine.GetAttachmentDesc = function(index, attachment, mode, ...)
    index, mode = resolveAttachmentLookup(index, mode)
    return original.GetAttachmentDesc(index, attachment, mode, ...)
  end
end

if original.WeaponOptionNewItemCount then
  Engine.WeaponOptionNewItemCount = function(controller, index, ...)
    local weapon = weaponsByIndex[index]
    if weapon and weapon.mpIndex and weapon.mpIndex ~= index then
      index = weapon.mpIndex
    end
    return original.WeaponOptionNewItemCount(controller, index, ...)
  end
end

if original.WeaponOptionNewModeAgnosticItemCount then
  Engine.WeaponOptionNewModeAgnosticItemCount = function(controller, index, ...)
    local weapon = weaponsByIndex[index]
    if weapon and weapon.mpIndex and weapon.mpIndex ~= index then
      index = weapon.mpIndex
    end
    return original.WeaponOptionNewModeAgnosticItemCount(controller, index, ...)
  end
end

Engine.GetAttachmentUniqueImageByAttachmentIndex = function(mode, weaponIndex, attachmentIndex, ...)
  local weapon = weaponsByIndex[weaponIndex]
  if weapon and weapon.mpIndex and not weapon.zmIndex then
    mode = MP
    weaponIndex = weapon.mpIndex
  elseif weapon and mode == MP and weapon.mpIndex then
    weaponIndex = weapon.mpIndex
  end
  return original.GetAttachmentUniqueImage(mode, weaponIndex, attachmentIndex, ...)
end

if original.IsItemLocked then
  Engine.IsItemLocked = function(controller, index, ...)
    if lootUnlocked() or weaponsByIndex[index] then
      return false
    end
    return original.IsItemLocked(controller, index, ...)
  end
end

if original.IsItemLockedForAll then
  Engine.IsItemLockedForAll = function(index, ...)
    if lootUnlocked() or weaponsByIndex[index] then
      return false
    end
    return original.IsItemLockedForAll(index, ...)
  end
end

local function getElementRef(element)
  if not element or not element.getModel then
    return nil
  end
  return getModelValue(element:getModel(), "ref")
end

if original.IsCACItemLocked then
  IsCACItemLocked = function(element, controller, ...)
    if weaponsByRef[getElementRef(element)] then
      return false
    end
    return original.IsCACItemLocked(element, controller, ...)
  end
end

if original.IsCACItemLockedOrNotPurchased then
  IsCACItemLockedOrNotPurchased = function(element, controller, ...)
    if weaponsByRef[getElementRef(element)] then
      return false
    end
    return original.IsCACItemLockedOrNotPurchased(element, controller, ...)
  end
end

if original.IsItemRefLocked then
  IsItemRefLocked = function(controller, ref, ...)
    if lootUnlocked() or weaponsByRef[ref] then
      return false
    end
    return original.IsItemRefLocked(controller, ref, ...)
  end
end

if original.IsGunsmithItemWeaponOptionLocked then
  IsGunsmithItemWeaponOptionLocked = function(menu, element, controller, ...)
    local index = original.GetCustomization(controller, "weapon_index")
    if weaponsByIndex[index] then
      return false
    end
    return original.IsGunsmithItemWeaponOptionLocked(menu, element, controller, ...)
  end
end

if CoD.BlackMarketUtility then
  if original.BMIsItemLocked then
    CoD.BlackMarketUtility.IsItemLocked = function(controller, ref, ...)
      if lootUnlocked() or weaponsByRef[ref] then
        return false
      end
      return original.BMIsItemLocked(controller, ref, ...)
    end
  end
  if original.BMGetItemQuantity then
    CoD.BlackMarketUtility.GetItemQuantity = function(controller, ref, ...)
      if lootUnlocked() or weaponsByRef[ref] then
        return 1
      end
      return original.BMGetItemQuantity(controller, ref, ...)
    end
  end
end

for ref in pairs(weaponsByRef) do
  allowCamos(ref)
end

CoD.UnlockablesTable = nil
