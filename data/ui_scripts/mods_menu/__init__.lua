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
local DETAILS_NAME_LENGTH = 80
local DETAILS_LINE_HEIGHT = 30
local DESCRIPTION_LIMIT = 255

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

local function cleanDescription(text)
  text = text or ""
  local truncated = #text >= DESCRIPTION_LIMIT
  text = text
    :gsub("\r", "")
    :gsub("%[img%].-%[/img%]", "")
    :gsub("%[previewyoutube.-%[/previewyoutube%]", "")
    :gsub("%[/?[%w%*]+[^%]]*%]", "")
    :gsub("%[[^%]]*$", "")
    :gsub("https?://%S+", "")
    :gsub("www%.%S+", "")
    :gsub("[ \t]+\n", "\n")
    :gsub("\n%s*\n+", "\n")
    :match("^%s*(.-)%s*$")
  if truncated then
    text = text .. "..."
  end
  return text
end

local function isLoadedMod(ugcName)
  return ugcName ~= nil and Engine.IsUsingMods() and Engine.UsingModsUgcName() == ugcName
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
  self.badgeText:setText("LOADED")
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

  self:linkToElementModel(self, "ugcName", true, function(model)
    self.ugcName = Engine.GetModelValue(model)
    local loaded = isLoadedMod(self.ugcName)
    self.badge:setAlpha(loaded and 1 or 0)
    self.badgeText:setAlpha(loaded and 1 or 0)
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
      Mods_LoadMod(controller, element)
      return true
    end,
    function(element, menu)
      CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBA_PSCROSS, "MENU_LOAD")
      return true
    end,
    false
  )
  self:addElement(list)
  self.ModsList = list

  local panelLeft = 64 + TILE_WIDTH * 3 + 60
  self:addElement(newImage(panelLeft, 1216, 120, 120 + TILE_HEIGHT * 3 + 28, 0.06, 0.06, 0.07, 0.85))
  self:addElement(newImage(panelLeft, 1216, 120, 123, ORANGE[1], ORANGE[2], ORANGE[3], 1))

  local detailsPreview = newImage(panelLeft + 16, 1200, 138, 138 + 170, 1, 1, 1, 0)
  self:addElement(detailsPreview)

  local detailsName =
    newText(panelLeft + 16, -80, 320, 346, "fonts/UnitedSansSmCdMd.ttf", Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  self:addElement(detailsName)

  local detailsStatus =
    newText(panelLeft + 16, -80, 352, 372, "fonts/RefrigeratorDeluxe-Regular.ttf", Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  self:addElement(detailsStatus)

  local detailsDescription =
    newText(panelLeft + 16, -80, 384, 402, "fonts/RefrigeratorDeluxe-Regular.ttf", Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  detailsDescription:setRGB(0.75, 0.75, 0.78)
  self:addElement(detailsDescription)

  local emptyText =
    newText(64, -64, 300, 330, "fonts/RefrigeratorDeluxe-Regular.ttf", Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)
  emptyText:setText("No mods installed. Subscribe to mods on the Steam Workshop or use the launcher's Workshop tab.")
  emptyText:setAlpha(Engine.Mods_Lists_GetInfoEntriesCount(LuaEnums.MODS_BASE_PATH) == 0 and 1 or 0)
  self:addElement(emptyText)

  self.updateModDetails = function(menu, tile)
    local model = tile:getModel()
    if not model then
      return
    end
    local ugcName = CoD.SafeGetModelValue(model, "ugcName")
    detailsName:setText(shortenText(CoD.SafeGetModelValue(model, "name") or "", DETAILS_NAME_LENGTH))
    local nameLines = math.min(2, math.max(1, math.ceil(detailsName:getTextWidth() / (1200 - panelLeft - 16))))
    local offset = (nameLines - 1) * DETAILS_LINE_HEIGHT
    detailsStatus:setTopBottom(true, false, 352 + offset, 372 + offset)
    detailsDescription:setTopBottom(true, false, 384 + offset, 402 + offset)
    detailsDescription:setText(cleanDescription(CoD.SafeGetModelValue(model, "description")))
    if isLoadedMod(ugcName) then
      detailsStatus:setText("LOADED")
      detailsStatus:setRGB(ORANGE[1], ORANGE[2], ORANGE[3])
    else
      detailsStatus:setText(
        "Version " .. tostring(CoD.SafeGetModelValue(model, "ugcVersion") or 1) .. "  |  " .. (ugcName or "")
      )
      detailsStatus:setRGB(0.6, 0.6, 0.63)
    end
    local image = ugcName and ModsMenu.GetPreview(ugcName)
    if image then
      detailsPreview:setImage(RegisterImage(image))
      detailsPreview:setAlpha(1)
    else
      detailsPreview:setAlpha(0)
    end
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

  self:AddButtonCallbackFunction(self, controller, Enum.LUIButton.LUI_KEY_XBY_PSTRIANGLE, "U", function(element)
    if Mods_IsUsingMods() then
      Mods_Unload(controller, element)
      return true
    end
  end, function(element, menu)
    if Mods_IsUsingMods() then
      CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBY_PSTRIANGLE, "PLATFORM_MODS_UNLOAD")
      return true
    end
    return false
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
