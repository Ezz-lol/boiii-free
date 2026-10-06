if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

local CARD_WIDTH = 300
local CARD_HEIGHT = 400
local CARD_GAP = 48
local CARD_TOP = 112
local CORNER_CUT = 18 -- size of the chamfered card corners
local PANEL_COLOR = { 0.04, 0.10, 0.12 }
local PANEL_ALPHA = 1
local BACKGROUND = {
  PANEL_COLOR[1] * PANEL_ALPHA + 0.02 * (1 - PANEL_ALPHA),
  PANEL_COLOR[2] * PANEL_ALPHA + 0.02 * (1 - PANEL_ALPHA),
  PANEL_COLOR[3] * PANEL_ALPHA + 0.03 * (1 - PANEL_ALPHA),
}
local CARD_REVEAL_DELAY = 450 -- ms between cards
local ICON_WIDTH = 264
local ICON_HEIGHT = 132
local ART_INSET = 34
local ART_ICON_WIDTH = 224
local ART_ICON_HEIGHT = 112
local ICON_TOP = 48
local BAR_HEIGHT = 52
local GRADIENT_STEPS = 48

local DEFAULT_COLORS = {
  [0] = { 0.80, 0.80, 0.80 },
  [1] = { 0.30, 0.65, 1.00 },
  [2] = { 0.75, 0.40, 1.00 }, -- epic: purple
  [3] = { 1.00, 0.62, 0.15 }, -- legendary: orange
  [4] = { 1.00, 0.35, 0.35 },
}

local DEFAULT_LOOT_SOUNDS = {
  cg_loot_open_sound = "uin_bm_chest_open",
  cg_loot_reveal_sound = "uin_bm_cycle_item_hit",
}
local RARITY_SOUND_LAYERS = {
  [1] = "uin_bm_cycle_item_rare_layer",
  [2] = "uin_bm_cycle_item_legend_layer",
  [3] = "uin_bm_cycle_item_epic_layer",
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

local function rarityColor(rank)
  local colors = CoD.BoiiiLootRarityColors or DEFAULT_COLORS
  return colors[rank or 0] or colors[0]
end

local function getProgress()
  if type(game.getlootprogress) == "function" then
    return game.getlootprogress() or {}
  end
  return {}
end

local function addImage(parent, rgb, alpha)
  local image = LUI.UIImage.new()
  image:setRGB(rgb[1], rgb[2], rgb[3])
  image:setAlpha(alpha or 1)
  parent:addElement(image)
  return image
end

local function addText(parent, text, font, rgb, center)
  local label = LUI.UIText.new()
  label:setTTF(font or "fonts/default.ttf")
  if center then
    label:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)
  else
    label:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  end
  if rgb then
    label:setRGB(rgb[1], rgb[2], rgb[3])
  end
  label:setText(text or "")
  parent:addElement(label)
  return label
end

local CYCLE_INTERVAL = 90 -- ms per flash
local CYCLE_FIRST_LAND = 900 -- ms until the first card lands
local CYCLE_STAGGER = 550 -- ms between cards landing
local CYCLE_ART = { "common", "rare", "legendary", "epic" } -- BO3 art names, low to high

local LAND_GLOW = {
  [0] = "uie_t7_blackmarket_backing_glow_common",
  [1] = "uie_t7_blackmarket_backing_glow_common", -- tinted blue; "backing_glow" is a backdrop, not a ring
  [2] = "uie_t7_blackmarket_backing_glowlegendary",
  [3] = "uie_t7_blackmarket_backing_epicglow",
  [4] = "uie_t7_blackmarket_backing_epicglow", -- limited_backglow is a solid backdrop, not a ring
}
local FIREWORK_SPARKS = {
  "uie_radial_gradient", -- white, so it takes the rarity color
}
local FIREWORK_COUNT = { [2] = 26, [3] = 36, [4] = 36 } -- spark dots per rank
local STREAK_COUNT = { [2] = 44, [3] = 60, [4] = 60 } -- streaks per rank
local STREAK_MS = 650 -- base duration of a streak (plus a random part)
local LAND_GLOW_MS = 1100 -- how long the border glow takes to fade out
local LAND_GLOW_PAD_X = 30 -- extra width on each side
local LAND_GLOW_PAD_TOP = 38 -- extra height above the card
local LAND_GLOW_PAD_BOTTOM = 48 -- extra height below the card
local LAND_GLOW_PAD_BOTTOM_COMMON = 40 -- same, for Common/Rare

local function random01()
  if math and math.random then
    return math.random()
  end
  return 0.5
end

local function animateCardIn(card, top, delay)
  card:setTopBottom(true, false, top, top + CARD_HEIGHT)
  card:setAlpha(0)
  local ok = pcall(function()
    card:beginAnimation("keyframe", delay + 1, false, false, CoD.TweenType.Linear)
    card:setAlpha(0)
    card:registerEventHandler("transition_complete_keyframe", function(element, event)
      if element.revealStarted then
        return
      end
      element.revealStarted = true
      playLootSound("cg_loot_reveal_sound")
      playSoundAlias(RARITY_SOUND_LAYERS[element.revealRank or 0])
      element:beginAnimation("keyframe", 80, false, false, CoD.TweenType.Linear)
      element:setAlpha(1)
    end)
  end)
  if not ok then
    card:setAlpha(1)
  end
end

local function addGlow(parent, rgb, alpha, imageName)
  local glow = LUI.UIImage.new()
  glow:setImage(RegisterImage(imageName or "uie_radial_gradient"))
  local additive = pcall(function()
    glow:setMaterial(LUI.UIImage.GetCachedMaterial("ui_add"))
  end)
  glow:setRGB(rgb[1], rgb[2], rgb[3])
  glow:setAlpha(additive and alpha or alpha * 0.25)
  parent:addElement(glow)
  return glow
end

local function lighten(color, amount)
  return {
    color[1] + (1 - color[1]) * amount,
    color[2] + (1 - color[2]) * amount,
    color[3] + (1 - color[3]) * amount,
  }
end

local function chamferCorner(card, color, isRight, isBottom)
  local half = CORNER_CUT * 0.7071
  local ok = pcall(function()
    local mask = addImage(card, BACKGROUND, 1)
    mask:setLeftRight(not isRight, isRight, -half, half)
    mask:setTopBottom(not isBottom, isBottom, -half, half)
    mask:setZRot(45)

    local edge = addImage(card, color, 1)
    local cx = isRight and -CORNER_CUT / 2 or CORNER_CUT / 2
    local cy = isBottom and -CORNER_CUT / 2 or CORNER_CUT / 2
    edge:setLeftRight(not isRight, isRight, cx - half, cx + half)
    edge:setTopBottom(not isBottom, isBottom, cy - 1.5, cy + 1.5)
    if isRight == isBottom then
      edge:setZRot(45)
    else
      edge:setZRot(-45)
    end
  end)
  return ok
end

local function buildCard(parent, item, left, index)
  local color = rarityColor(item.rank)
  local bright = lighten(color, 0.35)

  local card = LUI.UIElement.new()
  card:setLeftRight(true, false, left, left + CARD_WIDTH)
  card:setTopBottom(true, false, CARD_TOP, CARD_TOP + CARD_HEIGHT)
  parent:addElement(card)

  local bodyHeight = CARD_HEIGHT - BAR_HEIGHT
  local stepHeight = bodyHeight / GRADIENT_STEPS
  for step = 0, GRADIENT_STEPS - 1 do
    local shade = 0.10 + 0.30 * (step / (GRADIENT_STEPS - 1))
    local band = addImage(card, { color[1] * shade, color[2] * shade, color[3] * shade }, 1)
    band:setLeftRight(true, true, 0, 0)
    band:setTopBottom(true, false, step * stepHeight, (step + 1) * stepHeight + 1)
  end

  local lowerGlow = addGlow(card, color, 0.30)
  lowerGlow:setLeftRight(true, true, 0, 0)
  lowerGlow:setTopBottom(true, false, bodyHeight - 150, bodyHeight + 40)

  addItemIcon(card, item.icon, item.category, (CARD_WIDTH - ICON_WIDTH) / 2, ICON_TOP, ICON_WIDTH, ICON_HEIGHT)

  local nameTop = ICON_TOP + ICON_HEIGHT + 38
  local name = addText(card, string.upper(item.name or ""), "fonts/RefrigeratorDeluxe-Regular.ttf", bright, true)
  name:setLeftRight(true, true, 12, -12)
  name:setTopBottom(true, false, nameTop, nameTop + 32)

  local subtitle =
    addText(card, string.upper(item.subtitle or ""), "fonts/RefrigeratorDeluxe-Regular.ttf", { 0.92, 0.92, 0.92 }, true)
  subtitle:setLeftRight(true, true, 12, -12)
  subtitle:setTopBottom(true, false, nameTop + 34, nameTop + 56)

  local bar = addImage(card, color, 1)
  bar:setLeftRight(true, true, 0, 0)
  bar:setTopBottom(false, true, -BAR_HEIGHT, 0)

  local barShade = addImage(card, { color[1] * 0.6, color[2] * 0.6, color[3] * 0.6 }, 1)
  barShade:setLeftRight(true, true, 0, 0)
  barShade:setTopBottom(false, true, -BAR_HEIGHT, -BAR_HEIGHT + 3)

  local rarity =
    addText(card, string.upper(item.rarity or ""), "fonts/RefrigeratorDeluxe-Regular.ttf", { 1, 1, 1 }, true)
  rarity:setLeftRight(true, true, 0, 0)
  rarity:setTopBottom(false, true, -BAR_HEIGHT + 10, -10)

  local edges = {
    { true, true, 0, 0, true, false, 0, 3 },
    { true, true, 0, 0, false, true, -3, 0 },
    { true, false, 0, 3, true, true, 0, 0 },
    { false, true, -3, 0, true, true, 0, 0 },
  }
  for _, e in ipairs(edges) do
    local edge = addImage(card, bright, 1)
    edge:setLeftRight(e[1], e[2], e[3], e[4])
    edge:setTopBottom(e[5], e[6], e[7], e[8])
  end
  local inner = {
    { true, true, 8, -8, true, false, 8, 9 },
    { true, false, 8, 9, true, true, 8, -BAR_HEIGHT - 6 },
    { false, true, -9, -8, true, true, 8, -BAR_HEIGHT - 6 },
  }
  for _, e in ipairs(inner) do
    local line = addImage(card, bright, 0.18)
    line:setLeftRight(e[1], e[2], e[3], e[4])
    line:setTopBottom(e[5], e[6], e[7], e[8])
  end

  chamferCorner(card, bright, false, false)
  chamferCorner(card, bright, true, false)
  chamferCorner(card, bright, false, true)
  chamferCorner(card, bright, true, true)

  card.revealRank = item.rank or 0
  card:setAlpha(0) -- shown when its rarity cycle lands
  return card
end

local ART_RARITY = { [0] = "common", [1] = "rare", [2] = "legendary", [3] = "epic", [4] = "limited" }
local ART_LABEL = { [0] = "common", [1] = "rare", [2] = "legendary", [3] = "epic", [4] = "epic" }

local function buildArtCard(parent, item, left, index)
  local rank = item.rank or 0
  local color = rarityColor(rank)
  local art = ART_RARITY[rank] or "common"

  local card = LUI.UIElement.new()
  card:setLeftRight(true, false, left, left + CARD_WIDTH)
  card:setTopBottom(true, false, CARD_TOP, CARD_TOP + CARD_HEIGHT)
  parent:addElement(card)

  local backing = LUI.UIImage.new()
  backing:setLeftRight(true, true, 0, 0)
  backing:setTopBottom(true, true, 0, 0)
  backing:setImage(loadImage("uie_t7_blackmarket_" .. art .. "_backing"))
  card:addElement(backing)

  addItemIcon(
    card,
    item.icon,
    item.category,
    (CARD_WIDTH - ART_ICON_WIDTH) / 2,
    ICON_TOP + 12,
    ART_ICON_WIDTH,
    ART_ICON_HEIGHT,
    lighten(color, 0.35)
  )

  local nameTop = ICON_TOP + ICON_HEIGHT + 38
  local name =
    addText(card, string.upper(item.name or ""), "fonts/RefrigeratorDeluxe-Regular.ttf", lighten(color, 0.35), true)
  name:setLeftRight(true, true, ART_INSET, -ART_INSET)
  name:setTopBottom(true, false, nameTop, nameTop + 32)

  local subtitle =
    addText(card, string.upper(item.subtitle or ""), "fonts/RefrigeratorDeluxe-Regular.ttf", { 0.92, 0.92, 0.92 }, true)
  subtitle:setLeftRight(true, true, ART_INSET, -ART_INSET)
  subtitle:setTopBottom(true, false, nameTop + 34, nameTop + 56)

  local label = LUI.UIImage.new()
  label:setLeftRight(true, true, 0, 0)
  label:setTopBottom(false, true, -BAR_HEIGHT, 0)
  label:setImage(loadImage("uie_t7_blackmarket_label_" .. (ART_LABEL[rank] or "common")))
  card:addElement(label)

  local rarity =
    addText(card, string.upper(item.rarity or ""), "fonts/RefrigeratorDeluxe-Regular.ttf", { 1, 1, 1 }, true)
  rarity:setLeftRight(true, true, 0, 0)
  rarity:setTopBottom(false, true, -BAR_HEIGHT + 10, -10)

  card.revealRank = item.rank or 0
  card:setAlpha(0) -- shown when its rarity cycle lands
  return card
end

local function useArtCards()
  local style = 1
  pcall(function()
    style = Dvar.cg_loot_card_style:get()
  end)
  return style == 1
end

local function buildReveal(menu)
  if menu.cardsContainer then
    menu.cardsContainer:close()
  end

  local container = LUI.UIElement.new()
  container:setLeftRight(true, true, 0, 0)
  container:setTopBottom(true, true, 0, 0)
  menu:addElement(container)
  menu.cardsContainer = container

  local drop = type(game.getlastsupplydrop) == "function" and game.getlastsupplydrop() or {}
  local items = drop.items or {}

  local panel = addImage(container, PANEL_COLOR, PANEL_ALPHA)
  panel:setLeftRight(true, true, 100, -100)
  panel:setTopBottom(true, false, CARD_TOP - 24, CARD_TOP + CARD_HEIGHT + 24)
  local panelEdges = {
    { true, true, 100, -100, true, false, CARD_TOP - 24, CARD_TOP - 22 },
    { true, true, 100, -100, true, false, CARD_TOP + CARD_HEIGHT + 22, CARD_TOP + CARD_HEIGHT + 24 },
    { true, false, 100, 102, true, false, CARD_TOP - 24, CARD_TOP + CARD_HEIGHT + 24 },
    { false, true, -102, -100, true, false, CARD_TOP - 24, CARD_TOP + CARD_HEIGHT + 24 },
  }
  for _, e in ipairs(panelEdges) do
    local edge = addImage(container, { 0.25, 0.55, 0.60 }, 0.45)
    edge:setLeftRight(e[1], e[2], e[3], e[4])
    edge:setTopBottom(e[5], e[6], e[7], e[8])
  end

  local count = #items
  local totalWidth = count * CARD_WIDTH + math.max(count - 1, 0) * CARD_GAP
  local left = (1280 - totalWidth) / 2
  local cardBuilder = useArtCards() and buildArtCard or buildCard
  local slots = {}

  local burstLayer = LUI.UIElement.new()
  burstLayer:setLeftRight(true, true, 0, 0)
  burstLayer:setTopBottom(true, true, 0, 0)
  container:addElement(burstLayer)
  for index, item in ipairs(items) do
    local slotLeft = left + (index - 1) * (CARD_WIDTH + CARD_GAP)
    local card = cardBuilder(container, item, slotLeft, index)

    local rank = item.rank or 0
    local warmNames = { LAND_GLOW[rank] }
    if FIREWORK_COUNT[rank] then
      for _, spark in ipairs(FIREWORK_SPARKS) do
        table.insert(warmNames, spark)
      end
    end
    for _, name in ipairs(warmNames) do
      if name then
        local warm = LUI.UIImage.new()
        warm:setLeftRight(true, false, slotLeft, slotLeft + CARD_WIDTH)
        warm:setTopBottom(true, false, CARD_TOP, CARD_TOP + CARD_HEIGHT)
        warm:setImage(RegisterImage(name))
        warm:setAlpha(0.01)
        container:addElement(warm)
      end
    end

    local cycler = LUI.UIImage.new()
    cycler:setLeftRight(true, false, slotLeft, slotLeft + CARD_WIDTH)
    cycler:setTopBottom(true, false, CARD_TOP, CARD_TOP + CARD_HEIGHT)
    cycler:setImage(RegisterImage("uie_t7_blackmarket_common_backing"))
    container:addElement(cycler)

    table.insert(slots, {
      card = card,
      cycler = cycler,
      left = slotLeft,
      rank = rank,
      landed = false,
    })
  end

  local function land(slot)
    slot.landed = true
    slot.cycler:setAlpha(0)

    pcall(function()
      local tint = lighten(rarityColor(slot.rank), 0.4)

      local glowName = LAND_GLOW[slot.rank] or LAND_GLOW[0]
      local glow = addGlow(burstLayer, tint, 1, glowName)
      glow:setLeftRight(true, false, slot.left - LAND_GLOW_PAD_X, slot.left + CARD_WIDTH + LAND_GLOW_PAD_X)
      local padBottom = (slot.rank <= 1) and LAND_GLOW_PAD_BOTTOM_COMMON or LAND_GLOW_PAD_BOTTOM
      glow:setTopBottom(true, false, CARD_TOP - LAND_GLOW_PAD_TOP, CARD_TOP + CARD_HEIGHT + padBottom)
      pcall(function()
        glow:beginAnimation("keyframe", LAND_GLOW_MS, false, false, CoD.TweenType.Linear)
        glow:setAlpha(0)
      end)

      local streaks = STREAK_COUNT[slot.rank]
      if streaks then
        local centerX = slot.left + CARD_WIDTH / 2
        local centerY = CARD_TOP + CARD_HEIGHT / 2
        for index = 1, streaks do
          local angle = random01() * 6.2832
          local dx, dy = math.cos(angle), math.sin(angle)
          local startReach = 70 + random01() * 110
          local travel = 160 + random01() * 260
          local length = 40 + random01() * 90
          local thickness = 2 + random01() * 3
          local startX = centerX + dx * startReach
          local startY = centerY + dy * startReach
          local endX = startX + dx * travel
          local endY = startY + dy * travel

          local streak = addGlow(burstLayer, (index % 4 == 0) and lighten(tint, 0.5) or tint, 1, "uie_radial_gradient")
          streak:setLeftRight(true, false, startX - length / 2, startX + length / 2)
          streak:setTopBottom(true, false, startY - thickness / 2, startY + thickness / 2)
          pcall(function()
            streak:setZRot(-angle * 57.2958)
          end)
          pcall(function()
            streak:beginAnimation("keyframe", STREAK_MS + random01() * 450, false, false, CoD.TweenType.Linear)
            streak:setLeftRight(true, false, endX - length / 2, endX + length / 2)
            streak:setTopBottom(true, false, endY - thickness / 2, endY + thickness / 2)
            streak:setAlpha(0)
          end)
        end
      end

      local count = FIREWORK_COUNT[slot.rank]
      if count then
        playSoundAlias("uin_bm_chest_open_sparks")
        local centerX = slot.left + CARD_WIDTH / 2
        local centerY = CARD_TOP + CARD_HEIGHT / 2
        local halfWidth = CARD_WIDTH / 2 - 16 -- the art's visible frame
        local halfHeight = CARD_HEIGHT / 2 - 16
        local bright = lighten(tint, 0.5)

        for index = 1, count do
          local angle = random01() * 6.2832
          local dx, dy = math.cos(angle), math.sin(angle)

          local toSide = math.abs(dx) > 0.001 and halfWidth / math.abs(dx) or 100000
          local toEdge = math.abs(dy) > 0.001 and halfHeight / math.abs(dy) or 100000
          local reach = math.min(toSide, toEdge)
          local startX = centerX + dx * reach
          local startY = centerY + dy * reach

          local distance = 40 + random01() * 130
          local endX = startX + dx * distance
          local endY = startY + dy * distance
          local size = 6 + random01() * 12

          local color = (index % 3 == 0) and bright or tint
          local spark = addGlow(container, color, 1, FIREWORK_SPARKS[((index - 1) % #FIREWORK_SPARKS) + 1])
          spark:setLeftRight(true, false, startX - size / 2, startX + size / 2)
          spark:setTopBottom(true, false, startY - size / 2, startY + size / 2)
          pcall(function()
            spark:beginAnimation("keyframe", 450 + random01() * 750, false, false, CoD.TweenType.Linear)
            spark:setLeftRight(true, false, endX - size / 2, endX + size / 2)
            spark:setTopBottom(true, false, endY - size / 2, endY + size / 2)
            spark:setAlpha(0)
          end)
        end
      end
    end)

    animateCardIn(slot.card, CARD_TOP, 0)
  end

  local ok = pcall(function()
    local elapsed = 0
    local tick = 0
    local timer
    timer = LUI.UITimer.newElementTimer(CYCLE_INTERVAL, false, function()
      elapsed = elapsed + CYCLE_INTERVAL
      tick = tick + 1
      local cycling = false
      for index, slot in ipairs(slots) do
        if not slot.landed then
          if elapsed >= CYCLE_FIRST_LAND + (index - 1) * CYCLE_STAGGER then
            land(slot)
          else
            cycling = true
            local art = CYCLE_ART[((tick + index) % #CYCLE_ART) + 1]
            slot.cycler:setImage(RegisterImage("uie_t7_blackmarket_" .. art .. "_backing"))
          end
        end
      end
      if cycling then
        playSoundAlias("uin_bm_item_scroll")
      elseif timer then
        pcall(function()
          container:removeElement(timer)
        end)
        timer = nil
      end
    end)
    container:addElement(timer)
  end)

  if not ok then
    for index, slot in ipairs(slots) do
      slot.cycler:setAlpha(0)
      animateCardIn(slot.card, CARD_TOP, (index - 1) * CARD_REVEAL_DELAY)
    end
  end

  if (drop.bonus_keys or 0) > 0 then
    local centerY = CARD_TOP + CARD_HEIGHT / 2 - 20

    local medallionGlow = addGlow(container, { 1.0, 0.72, 0.2 }, 0.6)
    medallionGlow:setLeftRight(false, true, -96, -4)
    medallionGlow:setTopBottom(true, false, centerY - 66, centerY + 26)

    local keyIcon = LUI.UIImage.new()
    keyIcon:setLeftRight(false, true, -86, -14)
    keyIcon:setTopBottom(true, false, centerY - 56, centerY + 16)
    keyIcon:setImage(RegisterImage("uie_t7_blackmarket_promo_cryptokeys"))
    container:addElement(keyIcon)

    local bonus = addText(
      container,
      "+" .. tostring(drop.bonus_keys),
      "fonts/RefrigeratorDeluxe-Regular.ttf",
      { 1.0, 0.88, 0.45 },
      true
    )
    bonus:setLeftRight(false, true, -100, 0)
    bonus:setTopBottom(true, false, centerY + 22, centerY + 62)

    local bonusTitle =
      addText(container, "CRYPTOKEY BONUS", "fonts/RefrigeratorDeluxe-Regular.ttf", { 0.75, 0.9, 0.95 }, true)
    bonusTitle:setLeftRight(false, true, -100, 0)
    bonusTitle:setTopBottom(true, false, centerY + 66, centerY + 84)
  end

  if menu.keysLabel then
    menu.keysLabel:setText(string.format("CRYPTOKEYS: %d", getProgress().keys or 0))
  end
end

DataSources.BoiiiLootRevealOptions = ListHelper_SetupDataSource("BoiiiLootRevealOptions", function()
  local kind = CoD.BoiiiLootRevealKind or "common"
  local progress = getProgress()
  local cost = (kind == "rare") and (progress.rare_cost or 30) or (progress.common_cost or 10)
  local keys = progress.keys or 0
  return {
    {
      models = {
        displayText = string.format("OPEN ANOTHER (%d KEYS)", cost),
        action = function(menu, element, controller)
          if type(game.openlootsupplydrop) == "function" and game.openlootsupplydrop(kind) then
            playLootSound("cg_loot_open_sound")
            buildReveal(menu)
            if menu.RevealOptions then
              pcall(function()
                menu.RevealOptions:updateDataSource()
              end)
            end
          end
        end,
        disabledFunction = function()
          return keys < cost
        end,
      },
    },
    {
      models = {
        displayText = "DONE",
        action = function(menu, element, controller)
          GoBack(menu, controller)
        end,
      },
    },
  }
end, true)

LUI.createMenu.BoiiiLootRevealMenu = function(controller)
  local self = CoD.Menu.NewForUIEditor("BoiiiLootRevealMenu")
  self.soundSet = "ChooseDecal"
  self:setOwner(controller)
  self:setLeftRight(true, true, 0, 0)
  self:setTopBottom(true, true, 0, 0)
  self:playSound("menu_open", controller)
  self.buttonModel = Engine.CreateModel(Engine.GetModelForController(controller), "BoiiiLootRevealMenu.buttonPrompts")
  self.anyChildUsesUpdateState = true

  local title = (CoD.BoiiiLootRevealKind == "rare") and "RARE SUPPLY DROP" or "COMMON SUPPLY DROP"
  if CoD.GameSettings_Background then
    local settingsBackground = CoD.GameSettings_Background.new(self, controller)
    settingsBackground:setLeftRight(true, true, 0, 0)
    settingsBackground:setTopBottom(true, true, 0, 0)
    settingsBackground.MenuFrame.titleLabel:setText(Engine.Localize(title))
    pcall(function()
      settingsBackground.MenuFrame.cac3dTitleIntermediary0.FE3dTitleContainer0.MenuTitle.TextBox1.Label0:setText(
        Engine.Localize(title)
      )
    end)
    pcall(function()
      settingsBackground.GameSettingsSelectedItemInfo:setAlpha(0)
    end)
    self:addElement(settingsBackground)
    self.SettingsBackground = settingsBackground
    self.MenuFrame = settingsBackground.MenuFrame
  else
    local background = LUI.UIImage.new()
    background:setLeftRight(true, true, 0, 0)
    background:setTopBottom(true, true, 0, 0)
    background:setRGB(0.02, 0.02, 0.03)
    self:addElement(background)
  end

  self.keysLabel = addText(self, "", "fonts/RefrigeratorDeluxe-Regular.ttf", { 1.0, 0.85, 0.2 }, false)
  self.keysLabel:setLeftRight(false, true, -300, -70)
  self.keysLabel:setTopBottom(true, false, CARD_TOP + CARD_HEIGHT + 30, CARD_TOP + CARD_HEIGHT + 56)
  self.keysLabel:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_RIGHT)

  local keysIcon = LUI.UIImage.new()
  keysIcon:setLeftRight(false, true, -66, -36)
  keysIcon:setTopBottom(true, false, CARD_TOP + CARD_HEIGHT + 28, CARD_TOP + CARD_HEIGHT + 58)
  keysIcon:setImage(RegisterImage("uie_t7_blackmarket_promo_cryptokeys"))
  self:addElement(keysIcon)

  local list = LUI.UIList.new(self, controller, 2, 0, nil, true, false, 0, 0, false, false)
  list:makeFocusable()
  list:setLeftRight(false, false, -240, 240)
  list:setTopBottom(true, false, CARD_TOP + CARD_HEIGHT + 24, CARD_TOP + CARD_HEIGHT + 92)
  list:setWidgetType(CoD.List1ButtonLarge_PH)
  list:setVerticalCount(2)
  list:setDataSource("BoiiiLootRevealOptions")
  self:addElement(list)
  self.RevealOptions = list

  buildReveal(self)

  if not self.SettingsBackground and CoD.GenericMenuFrame then
    local frame = CoD.GenericMenuFrame.new(self, controller)
    frame:setLeftRight(true, true, 0, 0)
    frame:setTopBottom(true, true, 0, 0)
    frame.titleLabel:setText(Engine.Localize(title))
    pcall(function()
      frame.cac3dTitleIntermediary0.FE3dTitleContainer0.MenuTitle.TextBox1.Label0:setText(Engine.Localize(title))
    end)
    frame:setModel(self.buttonModel, controller)
    self:addElement(frame)
    self.MenuFrame = frame
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

  if self.SettingsBackground then
    self.MenuFrame:setModel(self.buttonModel, controller)
  end

  list.id = "RevealOptions"
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
    if element.RevealOptions then
      element.RevealOptions:close()
    end
    if element.cardsContainer then
      element.cardsContainer:close()
    end
    Engine.UnsubscribeAndFreeModel(
      Engine.GetModel(Engine.GetModelForController(controller), "BoiiiLootRevealMenu.buttonPrompts")
    )
    if CoD.BoiiiRefreshLootDropsMenu then
      pcall(CoD.BoiiiRefreshLootDropsMenu, CoD.BoiiiLootDropsMenuInstance)
    end
  end)
  return self
end
