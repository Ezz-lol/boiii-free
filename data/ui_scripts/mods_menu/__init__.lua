if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

require("ui.uieditor.widgets.Scrollbars.verticalScrollbar")

CoD.OverlayUtility.AddSystemOverlay("DeleteModConfirmation", {
  menuName = "SystemOverlay_Compact",
  title = "DELETE MOD?",
  description = function(name)
    return "This permanently deletes ^3" .. name .. "^7 from your PC. You will need to download it again to play it."
  end,
  categoryType = CoD.OverlayUtility.OverlayTypes.GenericMessage,
  [CoD.OverlayUtility.GoBackPropertyName] = CoD.OverlayUtility.DefaultGoBack,
  listDatasource = function(name, ugcName)
    DataSources.DeleteModConfirmationList = DataSourceHelpers.ListSetup("DeleteModConfirmationList", function()
      return {
        {
          models = { displayText = "DELETE" },
          properties = {
            action = function(_, _, controller, _, menu)
              GoBack(menu, controller)
              local failure = ModsMenu.Delete(ugcName)
              if not failure and menu.removeFromStack then
                menu:removeFromStack(ugcName)
              end
              if failure then
                LuaUtils.UI_ShowErrorMessageDialog(controller, failure)
              else
                Mods_RefreshListMods(controller)
              end
            end,
          },
        },
        {
          models = { displayText = "MENU_CANCEL_CAPS" },
          properties = {
            action = function(_, _, controller, _, menu)
              GoBack(menu, controller)
            end,
          },
        },
      }
    end, true)
    return "DeleteModConfirmationList"
  end,
})

local ORANGE = { 1, 0.55, 0.1 }
local TILE_WIDTH = 222
local TILE_HEIGHT = 160
local IMAGE_HEIGHT = 125
local PREVIEW_POLL_INTERVAL = 500
local TILE_NAME_LENGTH = 34
local DETAILS_NAME_LENGTH = 34
local ORDER_ROWS = 7
local ORDER_ROW_HEIGHT = 32
local ORDER_NAME_LENGTH = 30

local function stripColors(text)
  return (text or ""):gsub("%^%d", "")
end

local function shortenText(text, maxLength)
  local result, visible, i = "", 0, 1
  while i <= #text do
    local char = text:sub(i, i)
    if char == "^" and text:sub(i + 1, i + 1):match("%d") then
      result = result .. text:sub(i, i + 1)
      i = i + 2
    else
      if visible == maxLength then
        return result .. "..."
      end
      result = result .. char
      visible = visible + 1
      i = i + 1
    end
  end
  return result
end

local function isLoadedMod(ugcName)
  return ugcName ~= nil and Engine.IsUsingMods() and Engine.UsingModsUgcName() == ugcName
end

local function loadedStack()
  local stack = {}
  local main = Engine.IsUsingMods() and Engine.UsingModsUgcName() or ""
  if main == "" or main == "usermaps" then
    return stack
  end
  table.insert(stack, main)
  for id in string.gmatch(ModStack.Get(), "[^,]+") do
    table.insert(stack, id)
  end
  return stack
end

local function stackPosition(stack, ugcName)
  for i, id in ipairs(stack) do
    if id == ugcName then
      return i
    end
  end
end

local function sameStack(a, b)
  if #a ~= #b then
    return false
  end
  for i, id in ipairs(a) do
    if b[i] ~= id then
      return false
    end
  end
  return true
end

local function applyStack(controller, stack)
  ModStack.Set(table.concat(stack, ",", 2))
  if #stack == 0 then
    if Mods_IsUsingMods() then
      Mods_Unload(controller)
    end
  elseif isLoadedMod(stack[1]) then
    ModStack.Reload()
  else
    Engine.LobbyVM_CallFunc("LoadMod", { ugcName = stack[1], ugcType = LuaEnums.MODS_BASE_PATH, ugcVersion = 1 })
  end
end

local function newImage(left, right, top, bottom, r, g, b, alpha)
  local image = LUI.UIImage.new()
  image:setLeftRight(true, right == nil, left, right or 0)
  image:setTopBottom(true, bottom == nil, top, bottom or 0)
  image:setRGB(r, g, b)
  image:setAlpha(alpha)
  return image
end

local function newText(left, right, top, bottom, font, alignment)
  local text = LUI.UIText.new()
  text:setLeftRight(true, true, left, right)
  text:setTopBottom(true, false, top, bottom)
  text:setTTF(font)
  text:setAlignment(alignment)
  return text
end

local function newButton(left, right, top, bottom, onClick)
  local button = LUI.UIElement.new()
  button:setLeftRight(true, false, left, right)
  button:setTopBottom(true, false, top, bottom)
  button:setHandleMouse(true)
  button:setHandleMouseButton(true)
  button.background = newImage(0, nil, 0, nil, 0.12, 0.12, 0.14, 1)
  button:addElement(button.background)
  button.currentState = "DefaultState"
  button.clipsPerState = { DefaultState = { DefaultClip = function() end, Over = function() end } }
  button.paint = function(element)
    local level = element.hovered and 0.28 or 0.12
    if element.highlighted then
      element.background:setRGB(ORANGE[1] * (element.hovered and 1 or 0.8), ORANGE[2] * 0.8, ORANGE[3])
    elseif element.danger then
      element.background:setRGB(element.hovered and 0.75 or 0.55, 0.1, 0.1)
    else
      element.background:setRGB(level, level, level + 0.02)
    end
    element:setAlpha(element.enabled == false and 0.35 or 1)
  end
  button:registerEventHandler("mouseenter", function(element)
    element.hovered = true
    element:paint()
    return true
  end)
  button:registerEventHandler("mouseleave", function(element)
    element.hovered = false
    element:paint()
    return true
  end)
  button:registerEventHandler("leftmouseup", function(element)
    if element.enabled ~= false then
      onClick(element)
    end
    return true
  end)
  return button
end

CoD.ModTile = InheritFrom(LUI.UIElement)
CoD.ModTile.new = function(menu, controller)
  local self = LUI.UIElement.new()
  self:setClass(CoD.ModTile)
  self.id = "ModTile"
  self.soundSet = "default"
  self:setLeftRight(true, false, 0, TILE_WIDTH)
  self:setTopBottom(true, false, 0, TILE_HEIGHT)
  self:makeFocusable()
  self:setHandleMouse(true)

  self.background = newImage(0, nil, 0, nil, 0.07, 0.07, 0.08, 0.9)
  self:addElement(self.background)

  self.placeholder = newText(0, 0, 38, 92, "fonts/UnitedSansSmCdMd.ttf", Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)
  self.placeholder:setRGB(0.3, 0.3, 0.33)
  self:addElement(self.placeholder)

  self.preview = newImage(0, nil, 0, IMAGE_HEIGHT, 1, 1, 1, 0)
  self:addElement(self.preview)

  self.name = newText(
    8,
    -8,
    IMAGE_HEIGHT + 9,
    IMAGE_HEIGHT + 29,
    "fonts/RefrigeratorDeluxe-Regular.ttf",
    Enum.LUIAlignment.LUI_ALIGNMENT_LEFT
  )
  self:addElement(self.name)

  self.badge = newImage(6, 74, 6, 26, ORANGE[1], ORANGE[2], ORANGE[3], 0)
  self:addElement(self.badge)
  self.badgeText = newText(
    6,
    -(TILE_WIDTH - 74),
    8,
    24,
    "fonts/RefrigeratorDeluxe-Regular.ttf",
    Enum.LUIAlignment.LUI_ALIGNMENT_CENTER
  )
  self.badgeText:setRGB(0, 0, 0)
  self.badgeText:setAlpha(0)
  self:addElement(self.badgeText)

  self.focusBar = newImage(0, nil, TILE_HEIGHT - 3, TILE_HEIGHT, ORANGE[1], ORANGE[2], ORANGE[3], 0)
  self:addElement(self.focusBar)

  self.tryLoadPreview = function(element)
    if element.previewShown or not element.ugcName then
      return false
    end
    local image = ModsMenu.GetPreview(element.ugcName)
    if not image then
      return false
    end
    element.preview:setImage(RegisterImage(image))
    element.preview:setAlpha(1)
    element.placeholder:setAlpha(0)
    element.previewShown = true
    return true
  end

  menu.previewTiles[self] = true

  self:linkToElementModel(self, "name", true, function(model)
    local value = Engine.GetModelValue(model) or ""
    self.name:setText(shortenText(value, TILE_NAME_LENGTH))
    self.placeholder:setText(string.upper(string.sub(stripColors(value), 1, 1)))
  end)

  self.updateBadge = function(element)
    local position = stackPosition(menu.stack, element.ugcName)
    element.badge:setAlpha(position and 1 or 0)
    element.badgeText:setAlpha(position and 1 or 0)
    element.badgeText:setText(position == 1 and "MAIN" or ("#" .. tostring(position)))
  end

  self:subscribeToModel(menu.stackModel, function()
    self:updateBadge()
  end, false)

  self:linkToElementModel(self, "ugcName", true, function(model)
    self.ugcName = Engine.GetModelValue(model)
    self:updateBadge()
    self.preview:setAlpha(0)
    self.placeholder:setAlpha(1)
    self.previewShown = false
    self:tryLoadPreview()
  end)

  self.clipsPerState = {
    DefaultState = {
      DefaultClip = function()
        self.focusBar:setAlpha(0)
        self.background:setRGB(0.07, 0.07, 0.08)
      end,
      Focus = function()
        self.focusBar:setAlpha(1)
        self.background:setRGB(0.15, 0.15, 0.17)
      end,
    },
  }

  self:registerEventHandler("gain_focus", function(element, event)
    menu.focusedTile = element
    if stackPosition(menu.stack, element.ugcName) then
      menu.selected = element.ugcName
    end
    menu:updateModDetails(element)
    return LUI.UIElement.gainFocus(element, event)
  end)

  return self
end

LUI.createMenu.MenuModsLoad = function(controller)
  Engine.Mods_Lists_UpdateMods()

  local self = CoD.Menu.NewForUIEditor("MenuModsLoad")
  self.soundSet = "default"
  self:setOwner(controller)
  self:setLeftRight(true, true, 0, 0)
  self:setTopBottom(true, true, 0, 0)
  self:playSound("menu_open", controller)
  self.buttonModel = Engine.CreateModel(Engine.GetModelForController(controller), "MenuModsLoad.buttonPrompts")
  self.anyChildUsesUpdateState = true
  self.previewTiles = setmetatable({}, { __mode = "k" })
  self.stackModel = Engine.CreateModel(Engine.GetGlobalModel(), "ModsLoad.stack")
  self.appliedStack = loadedStack()
  self.stack = {}
  for i, id in ipairs(self.appliedStack) do
    self.stack[i] = id
  end

  local background = CoD.MP_Background.new(self, controller)
  background:setLeftRight(true, false, 0, 1280)
  background:setTopBottom(true, false, 0, 720)
  self:addElement(background)
  self.background = background

  self:addElement(newImage(0, nil, 0, nil, 0, 0, 0, 0.55))

  local list = LUI.UIList.new(self, controller, 14, 0, nil, false, false, 0, 0, false, true)
  list:makeFocusable()
  list:setLeftRight(true, false, 64, 64 + TILE_WIDTH * 3 + 28)
  list:setTopBottom(true, false, 120, 120 + TILE_HEIGHT * 3 + 28)
  list:setWidgetType(CoD.ModTile)
  list:setHorizontalCount(3)
  list:setVerticalCount(3)
  list:setSpacing(14)
  list:setVerticalScrollbar(CoD.verticalScrollbar)
  list:setDataSource("ModsLoadEntry")
  list:subscribeToGlobalModel(controller, "ModsGlobal", "update", function()
    UpdateDataSource(self, list, controller)
  end)
  self:AddButtonCallbackFunction(
    list,
    controller,
    Enum.LUIButton.LUI_KEY_XBA_PSCROSS,
    nil,
    function(element, menu, controller)
      local ugcName = menu.focusedTile and menu.focusedTile.ugcName
      local position = stackPosition(menu.stack, ugcName)
      if position then
        table.remove(menu.stack, position)
      elseif ugcName then
        table.insert(menu.stack, ugcName)
        menu.selected = ugcName
      end
      menu:stackChanged(controller)
      return true
    end,
    function(element, menu)
      local ugcName = menu.focusedTile and menu.focusedTile.ugcName
      CoD.Menu.SetButtonLabel(
        menu,
        Enum.LUIButton.LUI_KEY_XBA_PSCROSS,
        stackPosition(menu.stack, ugcName) and "Remove" or (#menu.stack == 0 and "MENU_LOAD" or "Add")
      )
      return ugcName ~= nil
    end,
    false
  )
  for _, move in ipairs({
    { button = Enum.LUIButton.LUI_KEY_LB, key = "Q", label = "Higher priority", offset = -1 },
    { button = Enum.LUIButton.LUI_KEY_RB, key = "E", label = "Lower priority", offset = 1 },
  }) do
    self:AddButtonCallbackFunction(list, controller, move.button, move.key, function(element, menu)
      menu:moveSelected(move.offset)
      return true
    end, function(element, menu)
      CoD.Menu.SetButtonLabel(menu, move.button, move.label)
      return stackPosition(menu.stack, menu.selected) ~= nil and #menu.stack > 1
    end, false)
  end
  self:addElement(list)
  self.ModsList = list

  local panelLeft = 64 + TILE_WIDTH * 3 + 60
  local panelBottom = 120 + TILE_HEIGHT * 3 + 28
  self:addElement(newImage(panelLeft, 1216, 120, panelBottom, 0.06, 0.06, 0.07, 0.85))
  self:addElement(newImage(panelLeft, 1216, 120, 123, ORANGE[1], ORANGE[2], ORANGE[3], 1))

  local detailsName =
    newText(panelLeft + 16, -80, 136, 162, "fonts/UnitedSansSmCdMd.ttf", Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  self:addElement(detailsName)

  local detailsStatus =
    newText(panelLeft + 16, -80, 168, 188, "fonts/RefrigeratorDeluxe-Regular.ttf", Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  self:addElement(detailsStatus)

  self:addElement(newImage(panelLeft + 16, 1200, 202, 203, 0.3, 0.3, 0.33, 1))

  local orderTitle =
    newText(panelLeft + 16, -80, 214, 240, "fonts/UnitedSansSmCdMd.ttf", Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  orderTitle:setText("LOAD ORDER")
  self:addElement(orderTitle)

  local orderHint =
    newText(panelLeft + 16, -80, 244, 262, "fonts/RefrigeratorDeluxe-Regular.ttf", Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  orderHint:setRGB(0.6, 0.6, 0.63)
  self:addElement(orderHint)

  local rows = {}
  for i = 1, ORDER_ROWS do
    local row = newButton(
      panelLeft + 16,
      1190,
      272 + (i - 1) * ORDER_ROW_HEIGHT,
      268 + i * ORDER_ROW_HEIGHT,
      function(element)
        if element.ugcName then
          self.selected = element.ugcName
          self:stackChanged(controller)
        end
      end
    )
    row.number = newText(8, -360, 5, 25, "fonts/UnitedSansSmCdMd.ttf", Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
    row.number:setRGB(ORANGE[1], ORANGE[2], ORANGE[3])
    row:addElement(row.number)
    row.name = newText(44, -8, 6, 24, "fonts/RefrigeratorDeluxe-Regular.ttf", Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
    row:addElement(row.name)
    self:addElement(row)
    rows[i] = row
  end

  local scrollTrack = newImage(1196, 1200, 272, 268 + ORDER_ROWS * ORDER_ROW_HEIGHT, 0.2, 0.2, 0.22, 0)
  self:addElement(scrollTrack)
  local scrollThumb = newImage(1196, 1200, 272, 300, ORANGE[1], ORANGE[2], ORANGE[3], 0)
  self:addElement(scrollThumb)

  local buttonTop = 280 + ORDER_ROWS * ORDER_ROW_HEIGHT
  local buttons = {}
  for _, info in ipairs({
    {
      left = 16,
      right = 116,
      label = "MOVE UP",
      action = function()
        self:moveSelected(-1)
      end,
    },
    {
      left = 124,
      right = 224,
      label = "MOVE DOWN",
      action = function()
        self:moveSelected(1)
      end,
    },
    {
      left = 232,
      right = 410,
      label = "LOAD ALL",
      action = function()
        self:applyNow()
      end,
    },
    {
      left = 232,
      right = 410,
      top = 40,
      label = "REMOVE",
      danger = true,
      action = function()
        self:removeFromStack(self.selected)
      end,
    },
  }) do
    local top = buttonTop + (info.top or 0)
    local button = newButton(panelLeft + info.left, panelLeft + info.right, top, top + 32, info.action)
    button.danger = info.danger
    local text = newText(0, 0, 7, 25, "fonts/RefrigeratorDeluxe-Regular.ttf", Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)
    text:setText(info.label)
    button:addElement(text)
    self:addElement(button)
    table.insert(buttons, button)
  end

  local orderFooter = newText(
    panelLeft + 16,
    -80,
    buttonTop + 80,
    buttonTop + 98,
    "fonts/RefrigeratorDeluxe-Regular.ttf",
    Enum.LUIAlignment.LUI_ALIGNMENT_LEFT
  )
  self:addElement(orderFooter)

  local emptyText =
    newText(64, -64, 300, 330, "fonts/RefrigeratorDeluxe-Regular.ttf", Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)
  emptyText:setText("No mods installed. Subscribe to mods on the Steam Workshop or use the launcher's Workshop tab.")
  emptyText:setAlpha(Engine.Mods_Lists_GetInfoEntriesCount(LuaEnums.MODS_BASE_PATH) == 0 and 1 or 0)
  self:addElement(emptyText)

  local modNames = {}
  self.updateNames = function(menu)
    modNames = {}
    local entries = Engine.Mods_Lists_GetInfoEntries(
      LuaEnums.MODS_BASE_PATH,
      0,
      Engine.Mods_Lists_GetInfoEntriesCount(LuaEnums.MODS_BASE_PATH)
    )
    for _, entry in pairs(entries or {}) do
      if entry.ugcName then
        modNames[entry.ugcName] = stripColors(entry.name or entry.ugcName)
      end
    end
  end

  self.updateOrder = function(menu)
    if not stackPosition(menu.stack, menu.selected) then
      menu.selected = menu.stack[1]
    end
    local position = stackPosition(menu.stack, menu.selected)
    local last = math.max(1, #menu.stack - ORDER_ROWS + 1)
    local first = math.min(menu.orderFirst or 1, last)
    if position then
      first = math.max(math.min(first, position - 1), position - ORDER_ROWS + 2)
    end
    first = math.max(1, math.min(first, last))
    menu.orderFirst = first
    scrollTrack:setAlpha(#menu.stack > ORDER_ROWS and 1 or 0)
    scrollThumb:setAlpha(#menu.stack > ORDER_ROWS and 1 or 0)
    if #menu.stack > ORDER_ROWS then
      local height = ORDER_ROWS * ORDER_ROW_HEIGHT - 4
      local top = 272 + height * (first - 1) / #menu.stack
      scrollThumb:setTopBottom(true, false, top, top + height * ORDER_ROWS / #menu.stack)
    end
    for i, row in ipairs(rows) do
      local index = first + i - 1
      local id = menu.stack[index]
      row.ugcName = id
      row.highlighted = id ~= nil and id == menu.selected
      row:paint()
      row:setAlpha(id and 1 or 0)
      row.number:setText(id and tostring(index) or "")
      local name = id and shortenText(modNames[id] or id, ORDER_NAME_LENGTH) or ""
      if index == 1 and id then
        name = name .. "  ^3MAIN"
      elseif id and not stackPosition(menu.appliedStack, id) then
        name = name .. "  ^3NEW"
      end
      row.name:setText(name)
    end

    local pending = not sameStack(menu.stack, menu.appliedStack)
    buttons[1].enabled = position ~= nil and position > 1
    buttons[2].enabled = position ~= nil and position < #menu.stack
    buttons[3].enabled = pending
    buttons[3].highlighted = pending
    buttons[4].enabled = position ~= nil
    for _, button in ipairs(buttons) do
      button:paint()
    end

    if #menu.stack == 0 then
      orderHint:setText("Select mods to load them together.")
    elseif #menu.stack > ORDER_ROWS then
      orderHint:setText(tostring(#menu.stack) .. " mods. The top one wins when two mods change the same thing.")
    else
      orderHint:setText("The top one wins when two mods change the same thing.")
    end

    if sameStack(menu.stack, menu.appliedStack) then
      orderFooter:setText(#menu.stack > 0 and "All mods are loaded." or "")
      orderFooter:setRGB(0.6, 0.6, 0.63)
    else
      orderFooter:setText("Not loaded yet. Press LOAD ALL to apply, or leave to cancel.")
      orderFooter:setRGB(ORANGE[1], ORANGE[2], ORANGE[3])
    end
  end

  self.stackChanged = function(menu, controller)
    Engine.ForceNotifyModelSubscriptions(menu.stackModel)
    if menu.focusedTile then
      menu:updateModDetails(menu.focusedTile)
    end
    menu:updateOrder()
    for _, button in ipairs({
      Enum.LUIButton.LUI_KEY_XBA_PSCROSS,
      Enum.LUIButton.LUI_KEY_LB,
      Enum.LUIButton.LUI_KEY_RB,
      Enum.LUIButton.LUI_KEY_XBY_PSTRIANGLE,
      Enum.LUIButton.LUI_KEY_RSTICK_PRESSED,
    }) do
      CoD.Menu.UpdateButtonShownState(menu, menu, controller, button)
    end
  end

  self.moveSelected = function(menu, offset)
    local position = stackPosition(menu.stack, menu.selected)
    local target = position and position + offset
    if target and target >= 1 and target <= #menu.stack then
      menu.stack[position], menu.stack[target] = menu.stack[target], menu.stack[position]
      menu:stackChanged(controller)
    end
  end

  self.removeFromStack = function(menu, ugcName)
    local position = stackPosition(menu.stack, ugcName)
    if position then
      table.remove(menu.stack, position)
      menu:stackChanged(controller)
    end
  end

  self.applyNow = function(menu)
    menu.appliedStack = {}
    for i, id in ipairs(menu.stack) do
      menu.appliedStack[i] = id
    end
    applyStack(controller, menu.stack)
    menu:stackChanged(controller)
  end

  self.updateModDetails = function(menu, tile)
    local model = tile:getModel()
    if not model then
      return
    end
    local ugcName = CoD.SafeGetModelValue(model, "ugcName")
    detailsName:setText(shortenText(CoD.SafeGetModelValue(model, "name") or "", DETAILS_NAME_LENGTH))
    local position = stackPosition(menu.stack, ugcName)
    if position then
      detailsStatus:setText(
        (position == 1 and "Main mod" or ("Priority " .. tostring(position)))
          .. (stackPosition(menu.appliedStack, ugcName) and "  |  Loaded" or "  |  Not loaded yet")
      )
      detailsStatus:setRGB(ORANGE[1], ORANGE[2], ORANGE[3])
    else
      detailsStatus:setText(
        "Version " .. tostring(CoD.SafeGetModelValue(model, "ugcVersion") or 1) .. "  |  " .. (ugcName or "")
      )
      detailsStatus:setRGB(0.6, 0.6, 0.63)
    end
    menu:updateOrder()
  end

  self:registerEventHandler("poll_mod_previews", function(element, event)
    for tile in pairs(element.previewTiles) do
      if tile:tryLoadPreview() and tile == element.focusedTile then
        element:updateModDetails(tile)
      end
    end
    return true
  end)
  self:addElement(LUI.UITimer.new(PREVIEW_POLL_INTERVAL, "poll_mod_previews", false, self))

  local menuFrame = CoD.GenericMenuFrame.new(self, controller)
  menuFrame:setLeftRight(true, true, 0, 0)
  menuFrame:setTopBottom(true, true, 0, 0)
  menuFrame.titleLabel:setText(Engine.Localize("MENU_MODS_CAPS"))
  menuFrame.cac3dTitleIntermediary0.FE3dTitleContainer0.MenuTitle.TextBox1.Label0:setText(
    Engine.Localize("MENU_MODS_CAPS")
  )
  self:addElement(menuFrame)
  self.MenuFrame = menuFrame

  self:subscribeToModel(Engine.GetModel(Engine.GetGlobalModel(), "ModsGlobal.update"), function()
    self:updateNames()
    self:updateOrder()
    CoD.Menu.UpdateButtonShownState(self, self, controller, Enum.LUIButton.LUI_KEY_XBY_PSTRIANGLE)
    emptyText:setAlpha(Engine.Mods_Lists_GetInfoEntriesCount(LuaEnums.MODS_BASE_PATH) == 0 and 1 or 0)
  end)

  self:AddButtonCallbackFunction(self, controller, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE, nil, function()
    GoBack(self, controller)
    return true
  end, function(element, menu)
    CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE, "MENU_BACK")
    return true
  end, false)

  self:AddButtonCallbackFunction(self, controller, Enum.LUIButton.LUI_KEY_XBX_PSSQUARE, "R", function(element)
    Mods_RefreshListMods(controller, element)
    return true
  end, function(element, menu)
    CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBX_PSSQUARE, "MENU_REFRESH")
    return true
  end, false)

  self:AddButtonCallbackFunction(
    list,
    controller,
    Enum.LUIButton.LUI_KEY_START,
    "D",
    function(element, menu, controller)
      local model = element:getModel()
      if model then
        CoD.OverlayUtility.CreateOverlay(
          controller,
          menu,
          "DeleteModConfirmation",
          CoD.SafeGetModelValue(model, "name") or "",
          CoD.SafeGetModelValue(model, "ugcName")
        )
      end
      return true
    end,
    function(element, menu)
      CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_START, "Delete")
      return true
    end,
    false
  )

  self:AddButtonCallbackFunction(self, controller, Enum.LUIButton.LUI_KEY_RSTICK_PRESSED, "F", function(element, menu)
    if not sameStack(menu.stack, menu.appliedStack) then
      menu:applyNow()
      return true
    end
  end, function(element, menu)
    CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_RSTICK_PRESSED, "Load mods")
    return not sameStack(menu.stack, menu.appliedStack)
  end, false)

  self:AddButtonCallbackFunction(self, controller, Enum.LUIButton.LUI_KEY_XBY_PSTRIANGLE, "U", function(element, menu)
    if #menu.stack > 0 then
      menu.stack = {}
      menu:stackChanged(controller)
      return true
    end
  end, function(element, menu)
    CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBY_PSTRIANGLE, "Clear all")
    return #menu.stack > 0
  end, false)

  list.id = "ModsList"
  menuFrame:setModel(self.buttonModel, controller)
  self:processEvent({ name = "menu_loaded", controller = controller })
  self:processEvent({ name = "update_state", menu = self })
  if not self:restoreState() then
    list:processEvent({ name = "gain_focus", controller = controller })
  end

  LUI.OverrideFunction_CallOriginalSecond(self, "close", function(element)
    element.background:close()
    element.ModsList:close()
    element.MenuFrame:close()
    Engine.UnsubscribeAndFreeModel(
      Engine.GetModel(Engine.GetModelForController(controller), "MenuModsLoad.buttonPrompts")
    )
  end)

  return self
end
