if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

local COLUMNS = 1
local ROWS_PER_COLUMN = 6
local MAX_DROPS = 6
local MINI_COLUMNS = 3
local MINI_WIDTH = 171
local MINI_HEIGHT = 228
local MINI_GAP = 16
local MINI_INSET = 19 -- the card art's inner frame
local MINI_BAR_HEIGHT = 26
local MINI_ART = { [0] = "common", [1] = "rare", [2] = "legendary", [3] = "epic", [4] = "limited" }
local MINI_LABEL = { [0] = "common", [1] = "rare", [2] = "legendary", [3] = "epic", [4] = "epic" }
local RARITY_NAMES = { [0] = "COMMON", [1] = "RARE", [2] = "EPIC", [3] = "LEGENDARY", [4] = "LIMITED" }

local LIST_TOP = 116
local ROW_HEIGHT = 78
local ROW_BOX_HEIGHT = 68
local COLUMN_LEFT = { 612 }
local COLUMN_WIDTH = 628

local TILE_LEFT = 40
local TILE_TOP = 112
local TILE_WIDTH = 240
local TILE_HEIGHT = 320
local TILE_GAP = 20
local TILE_BAR_HEIGHT = 44
local CRATE_ART = { "common", "rare" }
local FOCUS_ART = "epic" -- the game's orange card art
local CRATE_IMAGES = { "t7_blackmarket_crate_common", "t7_blackmarket_crate_rare" }
local KEY_ICON = "uie_t7_blackmarket_promo_cryptokeys"
local ICON_WIDTH = 132
local ICON_HEIGHT = 66

local RARITY_COLORS = {
  [0] = { 0.80, 0.80, 0.80 },
  [1] = { 0.30, 0.65, 1.00 },
  [2] = { 0.75, 0.40, 1.00 }, -- epic: purple
  [3] = { 1.00, 0.62, 0.15 }, -- legendary: orange
  [4] = { 1.00, 0.35, 0.35 },
}
CoD.BoiiiLootRarityColors = RARITY_COLORS

local DEFAULT_LOOT_SOUNDS = {
  cg_loot_open_sound = "uin_bm_chest_open",
  cg_loot_reveal_sound = "uin_bm_cycle_item_hit",
}
local RARITY_SOUND_LAYERS = {
  [1] = "uin_bm_cycle_item_rare_layer",
  [2] = "uin_bm_cycle_item_epic_layer",
  [3] = "uin_bm_cycle_item_legend_layer",
  [4] = "uin_bm_cycle_item_ltd_layer",
}

local function playSoundAlias(alias)
  pcall(function()
    if alias and alias ~= "" and alias ~= "none" and Engine.PlaySound then
      Engine.PlaySound(alias)
    end
  end)
end

local function playLootSound(dvarName)
  local alias = ""
  pcall(function()
    local dvar = Dvar[dvarName]
    alias = dvar and dvar:get() or ""
  end)
  if alias == "" then
    alias = DEFAULT_LOOT_SOUNDS[dvarName] or ""
  end
  playSoundAlias(alias)
end

local SQUARE_ICON_CATEGORIES = { decal = true, emblem = true, reticle = true }
local TILE_ICON_CATEGORIES = {}

local function loadImage(name)
  pcall(function()
    if Engine.PrecacheImage then
      Engine.PrecacheImage(name)
    end
  end)
  return RegisterImage(name)
end

local GENERIC_ICONS = { decal = "t7_icon_menu_simple_emblems", emblem = "t7_icon_menu_simple_emblems" }
local FRAMED_ICON_CATEGORIES = { calling_card = true, specialist_outfit = true, material = true }

local PRELOAD_IMAGES = {
  "t7_blackmarket_crate_common",
  "t7_blackmarket_crate_rare",
  "uie_t7_blackmarket_promo_cryptokeys",
  "uie_radial_gradient",
  "t7_icon_menu_simple_emblems",
}
for _, art in ipairs({ "common", "rare", "epic", "legendary", "limited" }) do
  table.insert(PRELOAD_IMAGES, "uie_t7_blackmarket_" .. art .. "_backing")
  table.insert(PRELOAD_IMAGES, "uie_t7_blackmarket_" .. art .. "_backing_spin")
  if art ~= "limited" then
    table.insert(PRELOAD_IMAGES, "uie_t7_blackmarket_label_" .. art)
  end
end

CoD.BoiiiPreloadLootArt = function()
  for _, name in ipairs(PRELOAD_IMAGES) do
    pcall(loadImage, name)
  end
end
CoD.BoiiiPreloadLootArt()

local function addItemIcon(parent, icon, category, left, top, width, height, frameColor)
  icon = GENERIC_ICONS[category] or icon
  if not icon or icon == "" then
    return
  end
  local isSwatch = string.sub(icon, 1, 5) == "menu_"
  local iconLeft, iconWidth = left, width
  if SQUARE_ICON_CATEGORIES[category] or isSwatch then
    iconWidth = height
    iconLeft = left + (width - height) / 2
  end
  if TILE_ICON_CATEGORIES[category] then
    local tile = LUI.UIImage.new()
    tile:setLeftRight(true, false, iconLeft, iconLeft + iconWidth)
    tile:setTopBottom(true, false, top, top + height)
    tile:setRGB(0.55, 0.55, 0.58)
    parent:addElement(tile)
  end
  local image = LUI.UIImage.new()
  image:setLeftRight(true, false, iconLeft, iconLeft + iconWidth)
  image:setTopBottom(true, false, top, top + height)
  image:setImage(loadImage(icon))
  parent:addElement(image)

  if not frameColor or not (FRAMED_ICON_CATEGORIES[category] or isSwatch) then
    return
  end

  if category == "specialist_outfit" then
    local fadeSteps = 12
    local fadeHeight = height * 0.45
    for step = 0, fadeSteps - 1 do
      local band = LUI.UIImage.new()
      local bandTop = top + height - fadeHeight + step * (fadeHeight / fadeSteps)
      band:setLeftRight(true, false, iconLeft, iconLeft + iconWidth)
      band:setTopBottom(true, false, bandTop, bandTop + fadeHeight / fadeSteps + 1)
      band:setRGB(0, 0, 0)
      local t = (step + 1) / fadeSteps
      band:setAlpha(0.85 * t * t)
      parent:addElement(band)
    end
  end

  local edges = {
    { iconLeft - 1, iconLeft + iconWidth + 1, top - 1, top },
    { iconLeft - 1, iconLeft + iconWidth + 1, top + height, top + height + 1 },
    { iconLeft - 1, iconLeft, top - 1, top + height + 1 },
    { iconLeft + iconWidth, iconLeft + iconWidth + 1, top - 1, top + height + 1 },
  }
  for _, e in ipairs(edges) do
    local edge = LUI.UIImage.new()
    edge:setLeftRight(true, false, e[1], e[2])
    edge:setTopBottom(true, false, e[3], e[4])
    edge:setRGB(frameColor[1], frameColor[2], frameColor[3])
    edge:setAlpha(0.55)
    parent:addElement(edge)
  end
end

local function getProgress()
  if type(game.getlootprogress) == "function" then
    return game.getlootprogress() or {}
  end
  return {}
end

local function addText(parent, left, right, top, bottom, text, font, rgb)
  local label = LUI.UIText.new()
  label:setLeftRight(true, false, left, right)
  label:setTopBottom(true, false, top, bottom)
  label:setTTF(font or "fonts/default.ttf")
  label:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  if rgb then
    label:setRGB(rgb[1], rgb[2], rgb[3])
  end
  label:setText(text or "")
  parent:addElement(label)
  return label
end

local MENU_BACKGROUND = { 0.02, 0.02, 0.03 }
local ROW_CORNER_CUT = 12
local ROW_GRADIENT_STEPS = 32

local function lighten(color, amount)
  return {
    color[1] + (1 - color[1]) * amount,
    color[2] + (1 - color[2]) * amount,
    color[3] + (1 - color[3]) * amount,
  }
end

local function addImage(parent, rgb, alpha)
  local image = LUI.UIImage.new()
  image:setRGB(rgb[1], rgb[2], rgb[3])
  image:setAlpha(alpha or 1)
  parent:addElement(image)
  return image
end

local function addGlow(parent, rgb, alpha)
  local glow = LUI.UIImage.new()
  glow:setImage(RegisterImage("uie_radial_gradient"))
  local additive = pcall(function()
    glow:setMaterial(LUI.UIImage.GetCachedMaterial("ui_add"))
  end)
  glow:setRGB(rgb[1], rgb[2], rgb[3])
  glow:setAlpha(additive and alpha or alpha * 0.25)
  parent:addElement(glow)
  return glow
end

local function chamferCorner(row, color, isRight, isBottom)
  local half = ROW_CORNER_CUT * 0.7071
  pcall(function()
    local mask = addImage(row, MENU_BACKGROUND, 1)
    mask:setLeftRight(not isRight, isRight, -half, half)
    mask:setTopBottom(not isBottom, isBottom, -half, half)
    mask:setZRot(45)

    local edge = addImage(row, color, 0.9)
    local cx = isRight and -ROW_CORNER_CUT / 2 or ROW_CORNER_CUT / 2
    local cy = isBottom and -ROW_CORNER_CUT / 2 or ROW_CORNER_CUT / 2
    edge:setLeftRight(not isRight, isRight, cx - half, cx + half)
    edge:setTopBottom(not isBottom, isBottom, cy - 1, cy + 1)
    if isRight == isBottom then
      edge:setZRot(45)
    else
      edge:setZRot(-45)
    end
  end)
end

local function addDropRow(parent, drop, index)
  local column = math.floor((index - 1) / ROWS_PER_COLUMN) + 1
  local rowIndex = (index - 1) % ROWS_PER_COLUMN
  local left = COLUMN_LEFT[column]
  local top = LIST_TOP + rowIndex * ROW_HEIGHT
  local color = RARITY_COLORS[drop.tier or 0] or RARITY_COLORS[0]
  local bright = lighten(color, 0.35)

  local row = LUI.UIElement.new()
  row:setLeftRight(true, false, left, left + COLUMN_WIDTH)
  row:setTopBottom(true, false, top, top + ROW_BOX_HEIGHT)
  parent:addElement(row)

  local stepWidth = COLUMN_WIDTH / ROW_GRADIENT_STEPS
  for step = 0, ROW_GRADIENT_STEPS - 1 do
    local shade = 0.30 - 0.22 * (step / (ROW_GRADIENT_STEPS - 1))
    local band = addImage(row, { color[1] * shade, color[2] * shade, color[3] * shade }, 1)
    band:setLeftRight(true, false, step * stepWidth, (step + 1) * stepWidth + 1)
    band:setTopBottom(true, true, 0, 0)
  end

  local glow = addGlow(row, color, 0.35)
  glow:setLeftRight(true, false, 0, ICON_WIDTH + 40)
  glow:setTopBottom(true, true, 0, 0)

  addItemIcon(row, drop.icon, drop.category, 14, 2, ICON_WIDTH, ICON_HEIGHT)

  local topEdge = addImage(row, bright, 0.9)
  topEdge:setLeftRight(true, true, 0, 0)
  topEdge:setTopBottom(true, false, 0, 2)
  local bottomEdge = addImage(row, color, 0.45)
  bottomEdge:setLeftRight(true, true, 0, 0)
  bottomEdge:setTopBottom(false, true, -1, 0)
  local rightEdge = addImage(row, color, 0.45)
  rightEdge:setLeftRight(false, true, -1, 0)
  rightEdge:setTopBottom(true, true, 0, 0)
  local stripe = addImage(row, bright, 1)
  stripe:setLeftRight(true, false, 0, 4)
  stripe:setTopBottom(true, true, 0, 0)

  local textLeft = 30 + ICON_WIDTH
  local title = addText(row, 0, 0, 0, 0, drop.title, "fonts/default.ttf", bright)
  title:setLeftRight(true, true, textLeft, -80)
  title:setTopBottom(true, false, 8, 28)

  local name = addText(row, 0, 0, 0, 0, drop.display, "fonts/RefrigeratorDeluxe-Regular.ttf")
  name:setLeftRight(true, true, textLeft, -12)
  name:setTopBottom(true, false, 30, 60)

  if drop.is_new then
    local pill = addImage(row, { 1.0, 0.82, 0.2 }, 1)
    pill:setLeftRight(false, true, -70, -18)
    pill:setTopBottom(true, false, 10, 28)
    local newLabel = addText(row, 0, 0, 0, 0, "NEW", "fonts/RefrigeratorDeluxe-Regular.ttf", { 0.08, 0.06, 0.02 })
    newLabel:setLeftRight(false, true, -70, -18)
    newLabel:setTopBottom(true, false, 10, 28)
    newLabel:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)
  end
end

local function dropSubtitle(title)
  title = title or ""
  local head, context = string.match(title, "^(.-) %- (.+)$")
  head = head or title
  local kind = string.match(head, "^%S+ (.+)$") or head
  if kind == "Outfit" then
    kind = "Theme"
  end
  if context and context ~= "" then
    return context .. " " .. kind
  end
  return kind
end

local function addDropCard(parent, drop, index)
  local column = (index - 1) % MINI_COLUMNS
  local row = math.floor((index - 1) / MINI_COLUMNS)
  local gridWidth = MINI_COLUMNS * MINI_WIDTH + (MINI_COLUMNS - 1) * MINI_GAP
  local left = COLUMN_LEFT[1] + (COLUMN_WIDTH - gridWidth) / 2 + column * (MINI_WIDTH + MINI_GAP)
  local top = LIST_TOP + row * (MINI_HEIGHT + MINI_GAP)
  local rank = drop.tier or 0
  local color = RARITY_COLORS[rank] or RARITY_COLORS[0]
  local bright = lighten(color, 0.35)

  local card = LUI.UIElement.new()
  card:setLeftRight(true, false, left, left + MINI_WIDTH)
  card:setTopBottom(true, false, top, top + MINI_HEIGHT)
  parent:addElement(card)

  local backing = LUI.UIImage.new()
  backing:setLeftRight(true, true, 0, 0)
  backing:setTopBottom(true, true, 0, 0)
  backing:setImage(loadImage("uie_t7_blackmarket_" .. (MINI_ART[rank] or "common") .. "_backing"))
  card:addElement(backing)

  local iconWidth = MINI_WIDTH - MINI_INSET * 2 - 8
  addItemIcon(card, drop.icon, drop.category, (MINI_WIDTH - iconWidth) / 2, 34, iconWidth, iconWidth / 2, bright)

  local nameTop = 34 + iconWidth / 2 + 22
  local name =
    addText(card, 0, 0, 0, 0, string.upper(drop.display or ""), "fonts/RefrigeratorDeluxe-Regular.ttf", bright)
  name:setLeftRight(true, true, MINI_INSET, -MINI_INSET)
  name:setTopBottom(true, false, nameTop, nameTop + 20)
  name:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)

  local subtitle = addText(
    card,
    0,
    0,
    0,
    0,
    string.upper(dropSubtitle(drop.title)),
    "fonts/RefrigeratorDeluxe-Regular.ttf",
    { 0.9, 0.9, 0.9 }
  )
  subtitle:setLeftRight(true, true, MINI_INSET, -MINI_INSET)
  subtitle:setTopBottom(true, false, nameTop + 22, nameTop + 36)
  subtitle:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)

  local label = LUI.UIImage.new()
  label:setLeftRight(true, true, 0, 0)
  label:setTopBottom(false, true, -MINI_BAR_HEIGHT, 0)
  label:setImage(loadImage("uie_t7_blackmarket_label_" .. (MINI_LABEL[rank] or "common")))
  card:addElement(label)

  local rarity =
    addText(card, 0, 0, 0, 0, RARITY_NAMES[rank] or "", "fonts/RefrigeratorDeluxe-Regular.ttf", { 1, 1, 1 })
  rarity:setLeftRight(true, true, 0, 0)
  rarity:setTopBottom(false, true, -MINI_BAR_HEIGHT + 6, -5)
  rarity:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)

  if drop.is_new then
    local newLabel = addText(card, 0, 0, 0, 0, "NEW", "fonts/RefrigeratorDeluxe-Regular.ttf", { 1.0, 0.82, 0.2 })
    newLabel:setLeftRight(false, true, -76, -40)
    newLabel:setTopBottom(false, true, -MINI_BAR_HEIGHT + 7, -6)
    newLabel:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_RIGHT)
  end
end

local function buildDropRows(menu)
  if menu.rowsContainer then
    menu.rowsContainer:close()
  end

  local container = LUI.UIElement.new()
  container:setLeftRight(true, true, 0, 0)
  container:setTopBottom(true, true, 0, 0)
  menu:addElement(container)
  menu.rowsContainer = container

  local drops = {}
  if type(game.getrecentlootdrops) == "function" then
    drops = game.getrecentlootdrops(MAX_DROPS) or {}
  end

  addText(
    container,
    COLUMN_LEFT[1],
    COLUMN_LEFT[1] + COLUMN_WIDTH,
    LIST_TOP - 26,
    LIST_TOP - 6,
    "RECENT DROPS",
    "fonts/default.ttf",
    { 0.75, 0.9, 0.95 }
  )

  if #drops == 0 then
    addText(
      container,
      COLUMN_LEFT[1],
      COLUMN_LEFT[1] + COLUMN_WIDTH,
      LIST_TOP + 10,
      LIST_TOP + 40,
      "No drops yet. Earn Cryptokeys by playing, then open a Supply Drop!",
      "fonts/RefrigeratorDeluxe-Regular.ttf"
    )
  end

  for index, drop in ipairs(drops) do
    if index > MAX_DROPS then
      break
    end
    addDropCard(container, drop, index)
  end

  local progress = getProgress()
  addText(
    container,
    COLUMN_LEFT[1],
    COLUMN_LEFT[1] + COLUMN_WIDTH,
    LIST_TOP + 2 * MINI_HEIGHT + MINI_GAP + 12,
    LIST_TOP + 2 * MINI_HEIGHT + MINI_GAP + 34,
    string.format("Collection: %d / %d Black Market items", progress.owned or 0, progress.total or 0),
    "fonts/default.ttf",
    { 0.75, 0.75, 0.75 }
  )

  local keys = progress.keys or 0
  if menu.keysLabel then
    menu.keysLabel:setText(tostring(keys))
  end
  if menu.nextKeyFill then
    local fraction = math.max(0, math.min(1, progress.next_key or 0))
    menu.nextKeyFill:setLeftRight(false, true, -260, -260 + 220 * fraction)
  end

  if type(game.marklootdropsseen) == "function" then
    game.marklootdropsseen()
  end
end

local function refreshDropsMenu(menu)
  if not menu then
    return
  end
  buildDropRows(menu)
  if menu.SupplyDrops then
    pcall(function()
      menu.SupplyDrops:updateDataSource()
    end)
  end
end
CoD.BoiiiRefreshLootDropsMenu = refreshDropsMenu

local function openSupplyDrop(menu, kind, controller)
  if type(game.openlootsupplydrop) ~= "function" then
    return
  end
  if menu.openingDrop then
    return
  end
  menu.openingDrop = true
  pcall(function()
    local timer = LUI.UITimer.newElementTimer(300, true, function()
      menu.openingDrop = false
    end)
    menu:addElement(timer)
  end)

  if game.openlootsupplydrop(kind) then
    playLootSound("cg_loot_open_sound")
    CoD.BoiiiLootRevealKind = kind
    CoD.BoiiiLootDropsMenuInstance = menu
    OpenPopup(menu, "BoiiiLootRevealMenu", controller)
  end
end

local CRATE_KINDS = { "common", "rare" }
local CRATE_TITLES = { "Common Supply Drop", "Rare Supply Drop" }
local CRATE_DESCRIPTIONS = {
  "3 random Black Market items.",
  "3 items with a guaranteed Rare or better, increased chance of Epic and Legendary. Includes bonus Cryptokeys.",
}

local TILE_FRAME = { 0.35, 0.6, 0.62 }
local TILE_FOCUS = { 1.0, 0.6, 0.2 }

CoD.BoiiiLootCrateTile = InheritFrom(LUI.UIElement)
CoD.BoiiiLootCrateTile.new = function(menu, controller)
  local self = LUI.UIElement.new()
  self:setUseStencil(false)
  self:setClass(CoD.BoiiiLootCrateTile)
  self.id = "BoiiiLootCrateTile"
  self.soundSet = "default"
  self:setLeftRight(true, false, 0, TILE_WIDTH)
  self:setTopBottom(true, false, 0, TILE_HEIGHT)
  self:makeFocusable()
  self:setHandleMouse(true)
  self.anyChildUsesUpdateState = true

  local focusArtWarm = LUI.UIImage.new()
  focusArtWarm:setLeftRight(true, true, 0, 0)
  focusArtWarm:setTopBottom(true, true, 0, 0)
  focusArtWarm:setImage(loadImage("uie_t7_blackmarket_" .. FOCUS_ART .. "_backing"))
  focusArtWarm:setAlpha(0.01)
  self:addElement(focusArtWarm)

  local focusLabelWarm = LUI.UIImage.new()
  focusLabelWarm:setLeftRight(true, true, 0, 0)
  focusLabelWarm:setTopBottom(false, true, -TILE_BAR_HEIGHT, 0)
  focusLabelWarm:setImage(loadImage("uie_t7_blackmarket_label_" .. FOCUS_ART))
  focusLabelWarm:setAlpha(0.01)
  self:addElement(focusLabelWarm)

  local backing = LUI.UIImage.new()
  backing:setLeftRight(true, true, 0, 0)
  backing:setTopBottom(true, true, 0, 0)
  self:addElement(backing)

  local crate = LUI.UIImage.new()
  crate:setLeftRight(false, false, -96, 96)
  crate:setTopBottom(true, false, 84, 84 + 128)
  self:addElement(crate)
  crate:linkToElementModel(self, "image", true, function(model)
    local value = Engine.GetModelValue(model)
    if value then
      crate:setImage(loadImage(value))
    end
  end)

  local keyIcon = LUI.UIImage.new()
  keyIcon:setLeftRight(false, true, -88, -62)
  keyIcon:setTopBottom(true, false, 34, 60)
  keyIcon:setImage(loadImage(KEY_ICON))
  self:addElement(keyIcon)

  local cost = addText(self, 0, 0, 0, 0, "", "fonts/RefrigeratorDeluxe-Regular.ttf", { 1, 1, 1 })
  cost:setLeftRight(false, true, -58, -24)
  cost:setTopBottom(true, false, 34, 60)
  cost:linkToElementModel(self, "cost", true, function(model)
    local value = Engine.GetModelValue(model)
    if value then
      cost:setText(tostring(value))
    end
  end)
  cost:linkToElementModel(self, "affordable", true, function(model)
    if Engine.GetModelValue(model) == 1 then
      cost:setRGB(1, 1, 1)
    else
      cost:setRGB(1, 0.3, 0.25)
    end
  end)

  local label = LUI.UIImage.new()
  label:setLeftRight(true, true, 0, 0)
  label:setTopBottom(false, true, -TILE_BAR_HEIGHT, 0)
  self:addElement(label)

  local title = addText(self, 0, 0, 0, 0, "", "fonts/RefrigeratorDeluxe-Regular.ttf", { 1, 1, 1 })
  title:setLeftRight(true, true, 0, 0)
  title:setTopBottom(false, true, -TILE_BAR_HEIGHT + 10, -10)
  title:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)
  title:linkToElementModel(self, "displayText", true, function(model)
    local value = Engine.GetModelValue(model)
    if value then
      title:setText(value)
    end
  end)

  self.crateArt = "common"
  local function setArt(art)
    backing:setImage(loadImage("uie_t7_blackmarket_" .. art .. "_backing"))
    label:setImage(loadImage("uie_t7_blackmarket_label_" .. art))
  end
  local artHolder = LUI.UIElement.new()
  self:addElement(artHolder)
  artHolder:linkToElementModel(self, "art", true, function(model)
    self.crateArt = Engine.GetModelValue(model) or "common"
    setArt(self.crateFocused and FOCUS_ART or self.crateArt)
  end)

  self.crateDescriptionText = ""
  local descriptionHolder = LUI.UIElement.new()
  self:addElement(descriptionHolder)
  descriptionHolder:linkToElementModel(self, "description", true, function(model)
    self.crateDescriptionText = Engine.GetModelValue(model) or ""
    if menu.selectedCrate == self and menu.crateDescription then
      menu.crateDescription:setText(self.crateDescriptionText)
    end
  end)

  local function showDescription(tile)
    if menu.crateDescription and tile then
      menu.crateDescription:setText(tile.crateDescriptionText or "")
    end
  end

  local function applyState(state)
    local focused = state ~= "default"
    self.crateFocused = focused
    self:setupElementClipCounter(2)

    backing:completeAnimation()
    label:completeAnimation()
    setArt(focused and FOCUS_ART or self.crateArt)
    self.clipFinished(backing, {})
    self.clipFinished(label, {})

    if state == "focus" then
      menu.selectedCrate = self
      showDescription(self)
    elseif state == "over" then
      showDescription(self)
    else
      if menu.selectedCrate == self then
        menu.selectedCrate = nil
      end
      showDescription(menu.selectedCrate)
    end
  end

  self.clipsPerState = {
    DefaultState = {
      DefaultClip = function()
        applyState("default")
      end,
      Focus = function()
        applyState("focus")
      end,
      Over = function()
        applyState("over")
      end,
    },
  }

  self:registerEventHandler("leftmouseup", function(element, event)
    ProcessListAction(menu, self, controller)
    return true
  end)

  return self
end

local function fadeInLootElement(element, firstOpen)
  pcall(function()
    element:setAlpha(0.01)
    element:beginAnimation("keyframe", firstOpen and 600 or 150, false, false, CoD.TweenType.Linear)
    element:setAlpha(0.01)
    element:registerEventHandler("transition_complete_keyframe", function(target, event)
      if target.lootFadedIn then
        return
      end
      target.lootFadedIn = true
      target:beginAnimation("keyframe", 200, false, false, CoD.TweenType.Linear)
      target:setAlpha(1)
    end)
  end)
end

DataSources.BoiiiLootSupplyDrops = ListHelper_SetupDataSource("BoiiiLootSupplyDrops", function()
  local progress = getProgress()
  local keys = progress.keys or 0
  local costs = { progress.common_cost or 10, progress.rare_cost or 30 }
  local options = {}
  for index, kind in ipairs(CRATE_KINDS) do
    table.insert(options, {
      models = {
        displayText = CRATE_TITLES[index],
        image = CRATE_IMAGES[index],
        art = CRATE_ART[index],
        cost = costs[index],
        affordable = (keys >= costs[index]) and 1 or 0,
        description = CRATE_DESCRIPTIONS[index],
        action = function(menu, element, controller)
          openSupplyDrop(menu, kind, controller)
        end,
      },
    })
  end
  return options
end, true)

LUI.createMenu.BoiiiLootDropsMenu = function(controller)
  local self = CoD.Menu.NewForUIEditor("BoiiiLootDropsMenu")
  self.soundSet = "ChooseDecal"
  self:setOwner(controller)
  self:setLeftRight(true, true, 0, 0)
  self:setTopBottom(true, true, 0, 0)
  self:playSound("menu_open", controller)
  self.buttonModel = Engine.CreateModel(Engine.GetModelForController(controller), "BoiiiLootDropsMenu.buttonPrompts")
  self.anyChildUsesUpdateState = true

  local background = LUI.UIImage.new()
  background:setLeftRight(true, true, 0, 0)
  background:setTopBottom(true, true, 0, 0)
  background:setRGB(0.02, 0.02, 0.03)
  background:setAlpha(1)
  self:addElement(background)

  local frameTitle = "BLACK MARKET"
  if CoD.GameSettings_Background then
    local settingsBackground = CoD.GameSettings_Background.new(self, controller)
    settingsBackground:setLeftRight(true, true, 0, 0)
    settingsBackground:setTopBottom(true, true, 0, 0)
    settingsBackground.MenuFrame.titleLabel:setText(Engine.Localize(frameTitle))
    pcall(function()
      settingsBackground.MenuFrame.cac3dTitleIntermediary0.FE3dTitleContainer0.MenuTitle.TextBox1.Label0:setText(
        Engine.Localize(frameTitle)
      )
    end)
    pcall(function()
      settingsBackground.GameSettingsSelectedItemInfo:setAlpha(0)
    end)
    self:addElement(settingsBackground)
    self.SettingsBackground = settingsBackground
    self.MenuFrame = settingsBackground.MenuFrame
  elseif CoD.GenericMenuFrame then
    local frame = CoD.GenericMenuFrame.new(self, controller)
    frame:setLeftRight(true, true, 0, 0)
    frame:setTopBottom(true, true, 0, 0)
    frame.titleLabel:setText(Engine.Localize(frameTitle))
    pcall(function()
      frame.cac3dTitleIntermediary0.FE3dTitleContainer0.MenuTitle.TextBox1.Label0:setText(Engine.Localize(frameTitle))
    end)
    self:addElement(frame)
    self.MenuFrame = frame
  end

  local list = LUI.UIList.new(self, controller, TILE_GAP, 0, nil, false, false, 0, 0, false, false)
  list:makeFocusable()
  list:setLeftRight(true, false, TILE_LEFT, TILE_LEFT + TILE_WIDTH * 2 + TILE_GAP)
  list:setTopBottom(true, false, TILE_TOP, TILE_TOP + TILE_HEIGHT)
  list:setWidgetType(CoD.BoiiiLootCrateTile)
  list:setHorizontalCount(2)
  list:setVerticalCount(1)
  list:setDataSource("BoiiiLootSupplyDrops")
  self:addElement(list)
  self.SupplyDrops = list

  self.lootFirstOpen = not CoD.BoiiiLootArtLoaded
  CoD.BoiiiLootArtLoaded = true

  self.crateDescription =
    addText(self, 0, 0, 0, 0, CRATE_DESCRIPTIONS[1], "fonts/RefrigeratorDeluxe-Regular.ttf", { 0.85, 0.85, 0.85 })
  self.crateDescription:setLeftRight(true, false, TILE_LEFT, TILE_LEFT + TILE_WIDTH * 2 + TILE_GAP)
  self.crateDescription:setTopBottom(true, false, TILE_TOP + TILE_HEIGHT + 14, TILE_TOP + TILE_HEIGHT + 36)

  local balanceIcon = LUI.UIImage.new()
  balanceIcon:setLeftRight(false, true, -392, -358)
  balanceIcon:setTopBottom(true, false, 28, 62)
  balanceIcon:setImage(loadImage(KEY_ICON))
  self:addElement(balanceIcon)

  self.keysLabel = addText(self, 0, 0, 0, 0, "", "fonts/RefrigeratorDeluxe-Regular.ttf", { 1, 1, 1 })
  self.keysLabel:setLeftRight(false, true, -352, -280)
  self.keysLabel:setTopBottom(true, false, 30, 60)

  local nextKeyTitle = addText(self, 0, 0, 0, 0, "NEXT CRYPTOKEY", "fonts/default.ttf", { 0.85, 0.85, 0.85 })
  nextKeyTitle:setLeftRight(false, true, -260, -40)
  nextKeyTitle:setTopBottom(true, false, 24, 42)

  local nextKeyBack = addImage(self, { 0.25, 0.28, 0.32 }, 0.8)
  nextKeyBack:setLeftRight(false, true, -260, -40)
  nextKeyBack:setTopBottom(true, false, 48, 58)

  self.nextKeyFill = addImage(self, { 1.0, 0.82, 0.3 }, 1)
  self.nextKeyFill:setLeftRight(false, true, -260, -260)
  self.nextKeyFill:setTopBottom(true, false, 48, 58)

  buildDropRows(self)

  fadeInLootElement(list, self.lootFirstOpen)
  if self.rowsContainer then
    fadeInLootElement(self.rowsContainer, self.lootFirstOpen)
  end

  self:AddButtonCallbackFunction(
    list,
    controller,
    Enum.LUIButton.LUI_KEY_XBA_PSCROSS,
    "ENTER",
    function(element, menu, actionController)
      ProcessListAction(self, element, actionController)
      return true
    end,
    function(element, menu)
      CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBA_PSCROSS, "MENU_SELECT")
      return true
    end,
    false
  )

  self:AddButtonCallbackFunction(
    self,
    controller,
    Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE,
    nil,
    function(element, menu, actionController)
      GoBack(self, actionController)
      return true
    end,
    function(element, menu)
      CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE, "MENU_BACK")
      return true
    end,
    false
  )

  self:AddButtonCallbackFunction(
    self,
    controller,
    Enum.LUIButton.LUI_KEY_XBY_PSTRIANGLE,
    "S",
    function(element, menu, actionController)
      OpenPopup(self, "BoiiiLootSettingsMenu", actionController)
      return true
    end,
    function(element, menu)
      CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBY_PSTRIANGLE, "MENU_SETTINGS")
      return true
    end,
    false
  )

  if self.MenuFrame then
    self.MenuFrame:setModel(self.buttonModel, controller)
  end

  list.id = "SupplyDrops"
  self:processEvent({ name = "menu_loaded", controller = controller })
  self:processEvent({ name = "update_state", menu = self })
  if not self:restoreState() then
    list:processEvent({ name = "gain_focus", controller = controller })
  end

  LUI.OverrideFunction_CallOriginalSecond(self, "close", function(element)
    if element.SettingsBackground then
      element.SettingsBackground:close()
    elseif element.MenuFrame then
      element.MenuFrame:close()
    end
    if element.SupplyDrops then
      element.SupplyDrops:close()
    end
    if element.rowsContainer then
      element.rowsContainer:close()
    end
    Engine.UnsubscribeAndFreeModel(
      Engine.GetModel(Engine.GetModelForController(controller), "BoiiiLootDropsMenu.buttonPrompts")
    )
    pcall(function()
      local nav = Engine.GetModel(Engine.GetGlobalModel(), "lobbyRoot.lobbyNav")
      if nav then
        Engine.ForceNotifyModelSubscriptions(nav)
      end
    end)
  end)
  return self
end
