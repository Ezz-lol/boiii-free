if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

pcall(function()
  require("ui.uieditor.widgets.PC.ServerBrowser.ServerBrowserRow")
end)
pcall(function()
  require("ui.uieditor.widgets.PC.ServerBrowser.ServerBrowserHeaderNamedColumn")
end)
pcall(function()
  require("ui.uieditor.widgets.Lobby.Common.FE_List1ButtonLarge_PH")
end)
pcall(function()
  require("ui.uieditor.widgets.PC.ServerBrowser.ServerBrowserButton")
end)
pcall(function()
  require("ui.uieditor.widgets.horizontalScrollingTextBox_18pt")
end)
pcall(function()
  require("ui.uieditor.widgets.Lobby.Common.FE_FocusBarContainer")
end)
pcall(function()
  require("ui.uieditor.widgets.Lobby.Common.FE_ButtonIdle")
end)
pcall(function()
  require("ui.uieditor.widgets.Lobby.Common.FE_ButtonFocus")
end)
pcall(function()
  require("ui.uieditor.widgets.Lobby.Common.FE_TabBar")
end)
pcall(function()
  require("ui.uieditor.widgets.BackgroundFrames.GenericMenuFrame")
end)
pcall(function()
  require("ui.uieditor.widgets.backgrounds.MP_Background")
end)
pcall(function()
  require("ui.uieditor.widgets.Lobby.Common.FE_Menu_LeftGraphics")
end)
pcall(function()
  require("ui.uieditor.widgets.Scrollbars.verticalScrollbar")
end)

local currentFilter = "all"
local currentSort = "name"
local sortAscending = true
local currentPage = 1
local searchQuery = ""

local visibleIds = {}
local knownItems = {}

local function isTruthy(value)
  return value ~= nil and value ~= false and value ~= 0 and value ~= "false"
end

local function formatSize(bytes)
  bytes = tonumber(bytes) or 0
  if bytes <= 0 then
    return "-"
  end
  local units = { "B", "KB", "MB", "GB", "TB" }
  local value = bytes
  local idx = 1
  while value >= 1024 and idx < #units do
    value = value / 1024
    idx = idx + 1
  end
  return string.format("%.1f %s", value, units[idx])
end

local function getSearchQuery()
  return searchQuery
end

local function setSearchQuery(text)
  searchQuery = tostring(text or "")
end

local function isInstalled(id)
  id = tostring(id or "")
  if id == "" then
    return false
  end
  local ok, value = pcall(function()
    return game.isworkshopinstalled and game.isworkshopinstalled(id)
  end)
  return ok and value and value ~= 0 and value ~= "false"
end

local function fetchInstalledItems()
  local ok, items = pcall(function()
    return game.getworkshopinstalled and game.getworkshopinstalled() or {}
  end)
  if not ok or type(items) ~= "table" then
    return {}
  end
  local search = string.lower(getSearchQuery() or "")
  if search == "" then
    return items
  end
  local filtered = {}
  for _, item in ipairs(items) do
    local hay = string.lower((item.title or "") .. " " .. (item.id or ""))
    if string.find(hay, search, 1, true) then
      table.insert(filtered, item)
    end
  end
  return filtered
end

local function fetchQueueItems()
  local ok, items = pcall(function()
    return game.getworkshopqueue and game.getworkshopqueue() or {}
  end)
  if not ok or type(items) ~= "table" then
    return {}
  end
  local out = {}
  for _, item in ipairs(items) do
    local known = knownItems[item.id] or {}
    table.insert(out, {
      id = item.id,
      title = (item.title and item.title ~= "" and item.title ~= item.id) and item.title or known.title or item.id,
      kind = item.kind or known.kind or "Map",
      file_size = (tonumber(item.file_size) or 0) > 0 and item.file_size or known.file_size or 0,
      description = known.description or "",
      preview = known.preview or "",
      state = item.state or "",
      position = tonumber(item.position) or -1,
    })
  end
  return out
end

local function queuePosition(id)
  id = tostring(id or "")
  if id == "" or not game or not game.getworkshopqueueposition then
    return -1
  end
  local ok, pos = pcall(function()
    return game.getworkshopqueueposition(id)
  end)
  return (ok and tonumber(pos)) or -1
end

local function queueHistoryState(id)
  for _, item in ipairs(fetchQueueItems()) do
    if item.id == id and item.position < 0 then
      return item.state
    end
  end
  return nil
end

local function queueHasWork()
  for _, item in ipairs(fetchQueueItems()) do
    if item.position >= 0 then
      return true
    end
  end
  return false
end

local function queueGeneration()
  if not game or not game.getworkshopqueuegeneration then
    return 0
  end
  local ok, gen = pcall(function()
    return game.getworkshopqueuegeneration()
  end)
  return (ok and tonumber(gen)) or 0
end

local function queueStatusText(item)
  local state = item.state or ""
  if state == "downloading" then
    return "DOWNLOADING"
  elseif state == "queued" then
    return "QUEUED #" .. tostring(item.position)
  elseif state == "done" then
    return "DONE"
  elseif state == "failed" then
    return "FAILED"
  elseif state == "cancelled" then
    return "CANCELLED"
  end
  return string.upper(state)
end

local function kindMatchesFilter(kind)
  local k = string.lower(kind or "")
  if currentFilter == "map" then
    return k == "map" or k == "maps" or k == "usermap" or k == "usermaps"
  end
  if currentFilter == "mod" then
    return k == "mod" or k == "mods"
  end
  return true
end

local function catalogStatus()
  local st = {}
  pcall(function()
    if game.getworkshopcatalogstatus then
      st = game.getworkshopcatalogstatus() or {}
    end
  end)
  return st
end

local function requestCatalog(page)
  if page ~= nil then
    currentPage = tonumber(page) or currentPage
  end
  if currentPage < 1 then
    currentPage = 1
  end
  pcall(function()
    if game.requestworkshopcatalog then
      game.requestworkshopcatalog(getSearchQuery(), currentPage)
    end
  end)
end

local function fetchCatalogItems()
  local ok, items = pcall(function()
    return game.getworkshopcatalog(getSearchQuery(), currentPage)
  end)
  if not ok or type(items) ~= "table" then
    return {}
  end
  return items
end

local function sortItems(items)
  table.sort(items, function(a, b)
    local va, vb
    if currentSort == "kind" then
      va = string.lower(a.kind or "")
      vb = string.lower(b.kind or "")
    elseif currentSort == "size" then
      va = tonumber(a.file_size) or 0
      vb = tonumber(b.file_size) or 0
    else
      va = string.lower(a.title or a.id or "")
      vb = string.lower(b.title or b.id or "")
    end
    if va == vb then
      return string.lower(a.title or "") < string.lower(b.title or "")
    end
    if sortAscending then
      return va < vb
    end
    return va > vb
  end)
end

local function truncate(text, maxLen)
  text = tostring(text or "")
  if #text <= maxLen then
    return text
  end
  return string.sub(text, 1, maxLen - 3) .. "..."
end

local function readModel(element, key)
  if not element or not element.getModel then
    return nil
  end
  local model = element:getModel()
  if not model then
    return nil
  end
  local keyModel = Engine.GetModel(model, key)
  if not keyModel then
    return nil
  end
  return Engine.GetModelValue(keyModel)
end

local refreshActionButton
local rebuildList
local updateDetails
local updateProgressPanel

local function getDownloadState()
  if not game or not game.getworkshopdownload then
    return nil
  end
  local ok, st = pcall(function()
    return game.getworkshopdownload()
  end)
  if not ok or type(st) ~= "table" then
    return nil
  end
  return st
end

local function isAnyDownloadActive()
  local st = getDownloadState()
  return st ~= nil and isTruthy(st.active)
end

local function flashInfo(menu, id, label, status)
  menu.flash = { id = id, label = label or "", status = status or "", ticks = 12 }
  if updateProgressPanel then
    updateProgressPanel(menu)
  end
end

local function selectionAction(menu)
  local sel = menu and menu.wsSel
  if not sel or sel.id == "" then
    return "none"
  end
  local pos = queuePosition(sel.id)
  if pos == 0 then
    return "cancel"
  end
  if pos > 0 then
    return "unqueue"
  end
  if sel.installed or isInstalled(sel.id) then
    if menu.pendingDeleteId == sel.id then
      return "confirm"
    end
    return "delete"
  end
  if isAnyDownloadActive() or queueHasWork() then
    return "queue"
  end
  return "download"
end

local ACTION_LABELS = {
  none = "DOWNLOAD",
  cancel = "CANCEL DOWNLOAD",
  unqueue = "REMOVE FROM QUEUE",
  confirm = "CONFIRM DELETE",
  delete = "DELETE",
  queue = "ADD TO QUEUE",
  download = "DOWNLOAD",
}

refreshActionButton = function(menu)
  if not menu or not menu.DownloadButton or not menu.DownloadButton.Label then
    return
  end
  menu.DownloadButton.Label:setText(ACTION_LABELS[selectionAction(menu)] or "DOWNLOAD")
end

local function queueSelected(menu)
  local sel = menu.wsSel
  local started = false
  pcall(function()
    started = game.queueworkshopitem(
      sel.id,
      sel.workshopKind or "Map",
      sel.title or sel.id,
      tostring(math.floor(tonumber(sel.fileSize) or 0))
    )
  end)
  started = isTruthy(started)
  if started then
    menu.pendingDeleteId = nil
  else
    flashInfo(menu, sel.id, "Could not add to the queue.", "")
  end
  return started
end

local function deleteSelected(menu)
  local sel = menu.wsSel
  if menu.pendingDeleteId ~= sel.id then
    menu.pendingDeleteId = sel.id
    if updateProgressPanel then
      updateProgressPanel(menu)
    end
    return true
  end
  menu.pendingDeleteId = nil
  local ok = false
  pcall(function()
    ok = isTruthy(game.deleteworkshopitem and game.deleteworkshopitem(sel.id))
  end)
  if ok then
    sel.installed = false
  end
  flashInfo(menu, sel.id, ok and "Deleted." or "Delete failed.", "")
  rebuildList(menu)
  return ok
end

local function runSelectionAction(menu)
  if not menu then
    return false
  end
  local action = selectionAction(menu)
  local sel = menu.wsSel
  local result = false
  if action == "cancel" then
    pcall(function()
      if game.cancelworkshopdownload then
        game.cancelworkshopdownload()
      end
    end)
    flashInfo(menu, sel.id, "Cancelling...", "")
    result = true
  elseif action == "unqueue" then
    pcall(function()
      result = isTruthy(game.removeworkshopqueue and game.removeworkshopqueue(sel.id))
    end)
    flashInfo(menu, sel.id, result and "Removed from queue." or "Could not remove.", "")
  elseif action == "delete" or action == "confirm" then
    result = deleteSelected(menu)
  elseif action == "download" or action == "queue" then
    result = queueSelected(menu)
  end
  refreshActionButton(menu)
  return result
end

local function actOnElement(menu, element)
  if updateDetails then
    updateDetails(menu, element)
  end
  return runSelectionAction(menu)
end

if CoD and not CoD.WorkshopBrowserRow then
  CoD.WorkshopBrowserRow = InheritFrom(LUI.UIElement)
end

CoD.WorkshopBrowserRow.new = function(menu, controller)
  local self = LUI.UIElement.new()
  self:setUseStencil(false)
  self:setClass(CoD.WorkshopBrowserRow)
  self.id = "WorkshopBrowserRow"
  self.soundSet = "default"
  self:setLeftRight(true, false, 0, 700)
  self:setTopBottom(true, false, 0, 22)
  self:makeFocusable()
  self:setHandleMouse(true)
  self.anyChildUsesUpdateState = true

  local background = LUI.UIImage.new()
  background:setLeftRight(true, true, 0, 0)
  background:setTopBottom(true, true, 0, 0)
  background:setRGB(0.2, 0.2, 0.2)
  background:setAlpha(0.8)
  self:addElement(background)
  self.background = background

  local function bindText(widget, key)
    widget:linkToElementModel(self, key, true, function(model)
      local value = Engine.GetModelValue(model)
      if value == nil then
        return
      end
      if widget.textBox then
        widget.textBox:setText(tostring(value))
      else
        widget:setText(tostring(value))
      end
    end)
  end

  local name
  if CoD.horizontalScrollingTextBox_18pt then
    name = CoD.horizontalScrollingTextBox_18pt.new(menu, controller)
  else
    name = LUI.UIText.new()
    name:setTTF("fonts/default.ttf")
  end
  name:setLeftRight(true, false, 8, 420)
  name:setTopBottom(true, false, 2, 20)
  if name.textBox then
    name.textBox:setTTF("fonts/default.ttf")
    name.textBox:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  else
    name:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
    name:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
  end
  bindText(name, "name")
  self:addElement(name)
  self.name = name

  local kind
  if CoD.horizontalScrollingTextBox_18pt then
    kind = CoD.horizontalScrollingTextBox_18pt.new(menu, controller)
  else
    kind = LUI.UIText.new()
    kind:setTTF("fonts/default.ttf")
  end
  kind:setLeftRight(true, false, 428, 540)
  kind:setTopBottom(true, false, 2, 20)
  if kind.textBox then
    kind.textBox:setTTF("fonts/default.ttf")
    kind.textBox:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  else
    kind:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
    kind:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
  end
  bindText(kind, "kind")
  self:addElement(kind)
  self.kind = kind

  local size
  if CoD.horizontalScrollingTextBox_18pt then
    size = CoD.horizontalScrollingTextBox_18pt.new(menu, controller)
  else
    size = LUI.UIText.new()
    size:setTTF("fonts/default.ttf")
  end
  size:setLeftRight(true, false, 548, 692)
  size:setTopBottom(true, false, 2, 20)
  if size.textBox then
    size.textBox:setTTF("fonts/default.ttf")
    size.textBox:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_RIGHT)
  else
    size:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_RIGHT)
    size:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
  end
  bindText(size, "size")
  self:addElement(size)
  self.size = size

  if CoD.FE_FocusBarContainer then
    local FocusBarB = CoD.FE_FocusBarContainer.new(menu, controller)
    FocusBarB:setLeftRight(true, true, -2, 2)
    FocusBarB:setTopBottom(false, true, -1, 3)
    FocusBarB:setAlpha(0)
    FocusBarB:setZoom(1)
    self:addElement(FocusBarB)
    self.FocusBarB = FocusBarB

    local FocusBarT = CoD.FE_FocusBarContainer.new(menu, controller)
    FocusBarT:setLeftRight(true, true, -2, 2)
    FocusBarT:setTopBottom(true, false, -3, 1)
    FocusBarT:setAlpha(0)
    FocusBarT:setZoom(1)
    self:addElement(FocusBarT)
    self.FocusBarT = FocusBarT
  end

  self.clipsPerState = {
    DefaultState = {
      DefaultClip = function()
        self:setupElementClipCounter(3)
        background:completeAnimation()
        background:setRGB(0.2, 0.2, 0.2)
        if self.FocusBarB then
          self.FocusBarB:completeAnimation()
          self.FocusBarB:setAlpha(0)
        end
        if self.FocusBarT then
          self.FocusBarT:completeAnimation()
          self.FocusBarT:setAlpha(0)
        end
      end,
      Focus = function()
        self:setupElementClipCounter(3)
        background:completeAnimation()
        background:setRGB(0.2, 0.2, 0.2)
        if self.FocusBarB then
          self.FocusBarB:completeAnimation()
          self.FocusBarB:setAlpha(1)
        end
        if self.FocusBarT then
          self.FocusBarT:completeAnimation()
          self.FocusBarT:setAlpha(1)
        end
      end,
      Over = function()
        self:setupElementClipCounter(3)
        background:completeAnimation()
        background:setRGB(0.32, 0.32, 0.32)
        if self.FocusBarB then
          self.FocusBarB:completeAnimation()
          self.FocusBarB:setAlpha(0)
        end
        if self.FocusBarT then
          self.FocusBarT:completeAnimation()
          self.FocusBarT:setAlpha(0)
        end
      end,
    },
  }

  LUI.OverrideFunction_CallOriginalSecond(self, "close", function(element)
    if element.name and element.name.close then
      element.name:close()
    end
    if element.kind and element.kind.close then
      element.kind:close()
    end
    if element.size and element.size.close then
      element.size:close()
    end
    if element.FocusBarB then
      element.FocusBarB:close()
    end
    if element.FocusBarT then
      element.FocusBarT:close()
    end
  end)

  return self
end

local function createWorkshopOption(item, sizeText)
  local title = (item.title and item.title ~= "") and item.title or item.id
  local kind = (item.kind and item.kind ~= "") and item.kind or "Map"
  local installed = item.installed == true or item.installed == 1 or item.installed == "true" or isInstalled(item.id)
  local kindLabel = kind
  if installed then
    kindLabel = kind .. " *"
  end
  return {
    models = {
      name = title,
      kind = kindLabel,
      size = sizeText or formatSize(item.file_size),
      realSize = formatSize(item.file_size),
      desc = item.description or item.path or "",
      description = item.description or item.path or "",
      workshopId = item.id,
      workshopPreview = item.preview or "",
      workshopKind = kind,
      fileSize = tonumber(item.file_size) or 0,
      installed = installed and 1 or 0,
      mapName = "",
      modName = kind,
      gameType = formatSize(item.file_size),
      displayText = title,
      action = function(self, element, controller)
        actOnElement(self, element)
      end,
    },
  }
end

DataSources.BoiiiWorkshopItems = ListHelper_SetupDataSource("BoiiiWorkshopItems", function()
  local options = {}
  local items = {}
  local isQueue = currentFilter == "queue"
  if currentFilter == "installed" then
    items = fetchInstalledItems()
  elseif isQueue then
    items = fetchQueueItems()
  else
    items = fetchCatalogItems()
  end
  local filtered = {}
  for _, item in ipairs(items) do
    if currentFilter == "installed" or isQueue or kindMatchesFilter(item.kind) then
      table.insert(filtered, item)
    end
  end
  if not isQueue then
    sortItems(filtered)
  end
  visibleIds = {}
  for _, item in ipairs(filtered) do
    if item.id and item.id ~= "" then
      visibleIds[item.id] = true
      if not isQueue then
        local prev = knownItems[item.id]
        local size = tonumber(item.file_size) or 0
        if size <= 0 and prev and tonumber(prev.file_size) and tonumber(prev.file_size) > 0 then
          size = tonumber(prev.file_size)
          item.file_size = size
        end
        knownItems[item.id] = {
          title = item.title,
          kind = item.kind,
          file_size = size,
          description = item.description,
          preview = item.preview,
        }
      end
    end
    table.insert(options, createWorkshopOption(item, isQueue and queueStatusText(item) or nil))
  end
  if #options == 0 then
    local search = getSearchQuery()
    local st = catalogStatus()
    local loading = st.loading and st.loading ~= 0 and st.loading ~= "false"
    local msg = "NO ITEMS FOUND"
    local hint = "Type a keyword and press START to search."
    if isQueue then
      msg = "QUEUE IS EMPTY"
      hint = "Items you download are added here and installed one after another."
    elseif currentFilter == "installed" then
      msg = "NO INSTALLED ITEMS"
      hint = "Folders in usermaps/ and mods/ appear here."
    elseif loading then
      msg = "LOADING WORKSHOP..."
      hint = "Fetching Steam workshop page " .. tostring(currentPage) .. "."
    elseif search ~= "" then
      msg = 'NO ITEMS FOUND FOR "' .. search .. '"'
    elseif currentFilter ~= "all" then
      msg = "NO ITEMS FOUND IN " .. string.upper(currentFilter)
    end
    table.insert(options, {
      models = {
        name = msg,
        kind = "",
        size = "",
        desc = hint,
        description = hint,
        workshopId = "",
        workshopKind = "",
        mapName = "",
        modName = "",
        gameType = "",
        disabledFunction = function()
          return true
        end,
      },
    })
  end
  return options
end, true)

rebuildList = function(menu)
  if not menu or not menu.Items then
    return
  end
  pcall(function()
    menu.Items:updateDataSource(true, true)
  end)
  pcall(function()
    menu.Items:updateDataSource()
  end)
end

local function wrapText(text, width)
  text = tostring(text or ""):gsub("\r", "")
  local lines = {}
  for para in string.gmatch(text .. "\n", "(.-)\n") do
    while #para > width do
      local cut = width
      local space = string.sub(para, 1, width):match("^.*() ")
      if space and space > width / 2 then
        cut = space
      end
      table.insert(lines, string.sub(para, 1, cut))
      para = string.sub(para, cut + 1)
    end
    table.insert(lines, para)
  end
  return lines
end

local function setKeywordLabel(menu, text)
  if not menu or not menu.SearchText then
    return
  end
  local shown = text or ""
  if shown ~= "" then
    menu.SearchText:setText(shown)
  else
    menu.SearchText:setText("...")
  end
end

local function applySearch(menu, text)
  currentPage = 1
  setSearchQuery(text)
  setKeywordLabel(menu, text)
  requestCatalog(1)
  rebuildList(menu)
  if menu.refreshPages then
    menu.refreshPages()
  end
end

local function openSearchKeyboard(menu, element, controller)
  if not menu then
    return
  end
  menu.workshopSearchPending = true
  local current = getSearchQuery()
  pcall(function()
    Engine.SetDvar("ui_keyboard_dvar_edit", current)
    Engine.SetDvar("ui_keyboardtitle", "PLATFORM_KEYWORDS_CAPS")
  end)
  if ShowKeyboard then
    ShowKeyboard(menu, element or menu, controller, "KEYBOARD_TYPE_SERVER_FILTER_KEYWORDS")
  else
    OpenPopup(menu, "KeyboardInputOverlay", controller)
  end
end

local DESC_VISIBLE = 8

local function refreshDescription(menu)
  if not menu or not menu.DescLines then
    return
  end
  local lines = menu.descAllLines or {}
  local maxScroll = math.max(0, #lines - DESC_VISIBLE)
  menu.descScroll = math.max(0, math.min(menu.descScroll or 0, maxScroll))
  for i, label in ipairs(menu.DescLines) do
    label:setText(lines[menu.descScroll + i] or "")
  end
  if menu.DescScrollHint then
    if maxScroll > 0 then
      menu.DescScrollHint:setText(string.format("%d/%d", menu.descScroll + 1, #lines))
    else
      menu.DescScrollHint:setText("")
    end
  end
end

local function setDescription(menu, text)
  if not menu or not menu.DescLines then
    return
  end
  menu.descAllLines = wrapText(text or "", 28)
  menu.descScroll = 0
  refreshDescription(menu)
end

local function scrollDescription(menu, delta)
  if not menu then
    return
  end
  menu.descScroll = (menu.descScroll or 0) + delta
  refreshDescription(menu)
end

local function bindWorkshopCover(menu)
  if not menu or not menu.CoverImage then
    return
  end
  pcall(function()
    menu.CoverImage:setImage(RegisterImage("img_t7_mod_preview"))
  end)
end

local function snapshotFromElement(element)
  local id = tostring(readModel(element, "workshopId") or "")
  if id == "" then
    return nil
  end
  return {
    id = id,
    title = readModel(element, "name") or id,
    kind = readModel(element, "kind") or "",
    workshopKind = readModel(element, "workshopKind") or "Map",
    size = readModel(element, "realSize") or readModel(element, "size") or "",
    desc = readModel(element, "desc") or readModel(element, "description") or "",
    preview = readModel(element, "workshopPreview") or "",
    fileSize = tonumber(readModel(element, "fileSize")) or 0,
    installed = isTruthy(readModel(element, "installed")),
  }
end

updateDetails = function(menu, element)
  if not menu then
    return
  end
  local sel = snapshotFromElement(element)
  local prevId = menu.wsSel and menu.wsSel.id or ""
  local newId = sel and sel.id or ""
  menu.wsSel = sel
  menu.selectedWorkshopElement = sel and element or nil
  if menu.pendingDeleteId ~= newId then
    menu.pendingDeleteId = nil
  end
  if newId ~= prevId then
    menu.smoothFrac = 0
    menu.flash = nil
  end

  if menu.DetailsTitle then
    menu.DetailsTitle:setText(sel and truncate(sel.title, 48) or "WORKSHOP")
  end
  if menu.DetailsKind then
    menu.DetailsKind:setText(sel and sel.kind or "")
  end
  if menu.DetailsSize then
    menu.DetailsSize:setText(sel and (sel.size ~= "" and sel.size or "-") or "")
  end
  if menu.DetailsId then
    menu.DetailsId:setText(sel and ("ID  " .. sel.id) or "")
  end
  if not sel then
    setDescription(menu, "Select an item from the list.")
  elseif sel.desc == "" then
    setDescription(menu, "No description.")
  else
    setDescription(menu, sel.desc)
  end

  pcall(function()
    if game.setworkshopcover then
      game.setworkshopcover(newId)
    end
    if sel and game.requestworkshoppreview then
      game.requestworkshoppreview(sel.id, sel.preview)
    end
  end)
  bindWorkshopCover(menu)
  refreshActionButton(menu)
  if updateProgressPanel then
    updateProgressPanel(menu)
  end
end

local function makeNamedColumn(menu, controller, left, right, label, sortKey)
  local col
  if CoD.ServerBrowserHeaderNamedColumn then
    col = CoD.ServerBrowserHeaderNamedColumn.new(menu, controller)
  else
    col = LUI.UIText.new()
    col:setTTF("fonts/FoundryGridnik-Medium.ttf")
    col:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
    col:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
  end
  col:setLeftRight(true, false, left, right)
  col:setTopBottom(true, true, 0, 0)
  pcall(function()
    if col.name and col.name.textBox then
      col.name.textBox:setText(label)
    elseif col.setText then
      col:setText(label)
    end
  end)
  col:setHandleMouse(true)
  local function onSort()
    if currentSort == sortKey then
      sortAscending = not sortAscending
    else
      currentSort = sortKey
      sortAscending = true
    end
    rebuildList(menu)
    return true
  end
  col:registerEventHandler("button_action", onSort)
  col:registerEventHandler("leftmouseup", onSort)
  return col
end

local function makeText(left, right, top, bottom, font, rgb)
  local label = LUI.UIText.new()
  label:setLeftRight(true, false, left, right)
  label:setTopBottom(true, false, top, bottom)
  label:setTTF(font or "fonts/default.ttf")
  if rgb then
    label:setRGB(rgb[1], rgb[2], rgb[3])
  end
  label:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  label:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
  return label
end

local function enableMouse(element)
  if not element then
    return
  end
  pcall(function()
    element:setHandleMouse(true)
  end)
  pcall(function()
    element:setHandleMouseMove(true)
  end)
  pcall(function()
    if EnableMouseButtonOnElement then
      EnableMouseButtonOnElement(element)
    end
  end)
  pcall(function()
    if EnableMouseOnElement then
      EnableMouseOnElement(element)
    end
  end)
end

local function makeHoverable(element, paint)
  local noop = function() end
  element.currentState = "DefaultState"
  element.clipsPerState = { DefaultState = { DefaultClip = noop, Over = noop } }
  element:registerEventHandler("mouseenter", function()
    paint(true)
    return true
  end)
  element:registerEventHandler("mouseleave", function()
    paint(false)
    return true
  end)
end

local function applyFrontendSlice(img, imageName, materialName, sliceW, sliceH)
  if not img then
    return
  end
  pcall(function()
    img:setImage(RegisterImage(imageName))
  end)
  pcall(function()
    local mat
    if LUI and LUI.UIImage and LUI.UIImage.GetCachedMaterial then
      mat = LUI.UIImage.GetCachedMaterial(materialName)
    elseif GetCachedMaterial then
      mat = GetCachedMaterial(materialName)
    end
    if mat then
      img:setMaterial(mat)
    end
  end)
  pcall(function()
    img:setupNineSliceShader(sliceW, sliceH)
  end)
end

local function tintImage(img, imageName)
  if not img then
    return
  end
  pcall(function()
    img:setImage(RegisterImage(imageName))
  end)
end

local function makeActionButton(menu, controller, left, right, top, bottom, label, onClick)
  local btn = LUI.UIElement.new()
  btn:setLeftRight(true, false, left, right)
  btn:setTopBottom(true, false, top, bottom)
  btn:makeFocusable()
  enableMouse(btn)

  local idle = LUI.UIImage.new()
  idle:setLeftRight(true, true, 0, 0)
  idle:setTopBottom(true, true, 0, 0)
  idle:setAlpha(1)
  applyFrontendSlice(idle, "uie_t7_menu_frontend_buttonidlefull", "uie_nineslice_add", 8, 8)
  btn:addElement(idle)
  btn.Idle = idle

  local focus = LUI.UIImage.new()
  focus:setLeftRight(true, true, -6, 6)
  focus:setTopBottom(true, true, -3, 3)
  focus:setAlpha(0)
  applyFrontendSlice(focus, "uie_t7_menu_frontend_buttonfocusfull", "uie_nineslice_add", 8, 8)
  btn:addElement(focus)
  btn.Focus = focus

  if CoD and CoD.FE_FocusBarContainer then
    local barB = CoD.FE_FocusBarContainer.new(menu, controller)
    barB:setLeftRight(true, true, -2, 2)
    barB:setTopBottom(false, true, -2, 2)
    barB:setAlpha(0)
    btn:addElement(barB)
    btn.FocusBarB = barB
    local barT = CoD.FE_FocusBarContainer.new(menu, controller)
    barT:setLeftRight(true, true, -2, 2)
    barT:setTopBottom(true, false, -2, 2)
    barT:setAlpha(0)
    btn:addElement(barT)
    btn.FocusBarT = barT
  end

  local text = LUI.UIText.new()
  text:setLeftRight(true, true, 12, -12)
  text:setTopBottom(false, false, -10, 10)
  text:setTTF("fonts/FoundryGridnik-Medium.ttf")
  text:setRGB(1, 1, 1)
  text:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)
  text:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
  text:setText(label)
  btn:addElement(text)
  btn.Label = text

  local function paint(hovered)
    btn.hovered = hovered and true or false
    if hovered then
      idle:setAlpha(0.25)
      focus:setAlpha(1)
      if btn.FocusBarB then
        btn.FocusBarB:setAlpha(1)
      end
      if btn.FocusBarT then
        btn.FocusBarT:setAlpha(1)
      end
      text:setRGB(1, 1, 1)
    else
      idle:setAlpha(1)
      focus:setAlpha(0)
      if btn.FocusBarB then
        btn.FocusBarB:setAlpha(0)
      end
      if btn.FocusBarT then
        btn.FocusBarT:setAlpha(0)
      end
      text:setRGB(0.85, 0.85, 0.85)
    end
  end
  paint(false)
  btn.PaintHover = paint

  btn.currentState = "DefaultState"
  btn.clipsPerState = {
    DefaultState = {
      DefaultClip = function()
        paint(false)
      end,
      Focus = function()
        paint(true)
      end,
    },
  }

  if onClick then
    btn:registerEventHandler("leftmouseup", function()
      onClick()
      return true
    end)
    btn:registerEventHandler("button_action", function()
      onClick()
      return true
    end)
  end
  return btn
end

local function makeFrontendButton(menu, controller, left, right, top, bottom, label)
  return makeActionButton(menu, controller, left, right, top, bottom, label, nil)
end

local PROGRESS_BAR_LEFT = 796
local PROGRESS_BAR_RIGHT = 1200
local PROGRESS_BAR_WIDTH = PROGRESS_BAR_RIGHT - PROGRESS_BAR_LEFT

local function formatSpeed(bps)
  bps = tonumber(bps) or 0
  if bps <= 0 then
    return ""
  end
  return formatSize(bps) .. "/s"
end

local function formatEta(seconds)
  seconds = tonumber(seconds) or -1
  if seconds < 0 then
    return ""
  end
  seconds = math.floor(seconds + 0.5)
  local m = math.floor(seconds / 60)
  local s = seconds % 60
  if m > 0 then
    return string.format("%dm %02ds", m, s)
  end
  return string.format("%ds", s)
end

local function setProgressBar(menu, fraction)
  if not menu or not menu.ProgressFill then
    return
  end
  if fraction < 0 then
    fraction = 0
  end
  if fraction > 1 then
    fraction = 1
  end
  local width = math.floor(PROGRESS_BAR_WIDTH * fraction + 0.5)
  menu.ProgressFill:setLeftRight(true, false, PROGRESS_BAR_LEFT, PROGRESS_BAR_LEFT + width)
end

local function setInfo(menu, label, status)
  if menu.ProgressLabel then
    menu.ProgressLabel:setText(label or "")
  end
  if menu.ProgressStatus then
    menu.ProgressStatus:setText(status or "")
  end
end

updateProgressPanel = function(menu)
  if not menu then
    return
  end
  local st = getDownloadState()
  local sel = menu.wsSel
  local selId = sel and sel.id or ""
  local active = st ~= nil and isTruthy(st.active)
  local dlId = st and tostring(st.id or "") or ""

  if selId ~= "" and active and dlId == selId then
    local frac = tonumber(st.fraction) or 0
    if frac + 0.01 < (menu.smoothFrac or 0) then
      frac = menu.smoothFrac
    else
      menu.smoothFrac = (menu.smoothFrac or 0) * 0.45 + frac * 0.55
      frac = menu.smoothFrac
    end
    setProgressBar(menu, frac)
    local line = string.format("%d%%", math.floor(frac * 100 + 0.5))
    local have = formatSize(st.downloaded_bytes)
    local total = formatSize(st.total_bytes)
    if have ~= "" then
      line = line .. "  " .. have
      if total ~= "-" and total ~= "" then
        line = line .. " / " .. total
      end
    end
    local speed = formatSpeed(st.speed_bps)
    if speed ~= "" then
      line = line .. "  " .. speed
    end
    local eta = formatEta(st.eta_seconds)
    if eta ~= "" then
      line = line .. "  ETA " .. eta
    end
    setInfo(menu, line, truncate(st.status_line or st.item_name or "", 52))
    return
  end

  menu.smoothFrac = 0
  setProgressBar(menu, 0)

  local busyHint = ""
  if active then
    busyHint = "Another item is downloading (see QUEUE)."
  end

  if selId == "" then
    setInfo(menu, "", busyHint)
    return
  end

  if menu.pendingDeleteId == selId then
    setInfo(menu, "Click DELETE again to confirm.", "")
    return
  end

  if menu.flash and menu.flash.id == selId and menu.flash.ticks > 0 then
    setInfo(menu, menu.flash.label, menu.flash.status)
    return
  end

  local pos = queuePosition(selId)
  if pos == 0 then
    setInfo(menu, "Starting download...", "")
    return
  end
  if pos > 0 then
    setInfo(
      menu,
      "In queue: position " .. tostring(pos),
      active and "Waits for the current download to finish." or "Starting soon..."
    )
    return
  end

  local history = queueHistoryState(selId)
  if history == "done" and (sel.installed or isInstalled(selId)) then
    setProgressBar(menu, 1)
    setInfo(menu, "Download finished", "")
  elseif history == "failed" then
    setInfo(menu, "Last download failed.", "Press DOWNLOAD to try again.")
  elseif history == "cancelled" then
    setInfo(menu, "Download cancelled.", busyHint)
  else
    setInfo(menu, "", busyHint)
  end
end

local function tickWorkshopDownload(menu)
  if not menu then
    return
  end
  if menu.flash and menu.flash.ticks > 0 then
    menu.flash.ticks = menu.flash.ticks - 1
  end

  local qgen = queueGeneration()
  if menu.queueGeneration ~= qgen then
    local first = menu.queueGeneration == nil
    menu.queueGeneration = qgen
    if not first then
      rebuildList(menu)
    end
  end

  local cat = catalogStatus()
  local gen = tonumber(cat.generation) or 0
  if menu.catalogGeneration ~= gen then
    menu.catalogGeneration = gen
    rebuildList(menu)
    if menu.refreshPages then
      menu.refreshPages()
    end
  end

  if menu.wsSel and not visibleIds[menu.wsSel.id] then
    updateDetails(menu, nil)
  end

  updateProgressPanel(menu)
  refreshActionButton(menu)
  bindWorkshopCover(menu)
end

local FRONTEND_SCENE_MENU = "MegaChewFactory"

local function notifyMenuScene(controller, menuName, opened)
  pcall(function()
    if SendCustomClientScriptMenuChangeNotify then
      SendCustomClientScriptMenuChangeNotify(controller, menuName, opened)
      return
    end
  end)
  pcall(function()
    local localClient = 0
    if CoD and CoD.GetLocalClientAdjustedNum then
      localClient = CoD.GetLocalClientAdjustedNum(controller) or 0
    end
    local state = opened and "opened" or "closed"
    Engine.SendClientScriptNotify(controller, "menu_change" .. tostring(localClient), menuName, state)
  end)
end

local function applyZmFrontend(menu, controller)
  if not menu then
    return
  end
  menu.frontendSceneArmed = true
  notifyMenuScene(controller, FRONTEND_SCENE_MENU, true)
  pcall(function()
    if Engine.PlayMenuMusic then
      Engine.PlayMenuMusic("zm_frontend")
    end
  end)
end

local function restoreFrontendRoom(menu, controller)
  if not menu then
    return
  end
  if menu.frontendSceneArmed then
    notifyMenuScene(controller, FRONTEND_SCENE_MENU, false)
    menu.frontendSceneArmed = false
  end
  pcall(function()
    if RefreshLobbyRoom then
      RefreshLobbyRoom(menu, controller)
    end
  end)
end

local function getLuiRoot()
  local root
  pcall(function()
    if Engine.GetLuiRoot then
      root = Engine.GetLuiRoot()
    end
  end)
  if not root and LUI and LUI.roots then
    root = LUI.roots.UIRoot0 or LUI.roots.UIRoot
  end
  return root
end

local function forEachRootChild(fn)
  local root = getLuiRoot()
  if not root or not root.getFirstChild then
    return
  end
  local child = root:getFirstChild()
  local guard = 0
  while child and guard < 64 do
    guard = guard + 1
    fn(child)
    if child.getNextSibling then
      child = child:getNextSibling()
    else
      child = nil
    end
  end
end

local function isWorkshopMenu(node)
  if not node then
    return false
  end
  return node.menuName == "BoiiiWorkshopMenu" or node.id == "BoiiiWorkshopMenu"
end

local function hideOccludedUi(menu)
  if not menu then
    return
  end
  menu.hiddenOccluded = menu.hiddenOccluded or {}
  local function hide(node)
    if not node or isWorkshopMenu(node) then
      return
    end
    for _, existing in ipairs(menu.hiddenOccluded) do
      if existing == node then
        return
      end
    end
    pcall(function()
      table.insert(menu.hiddenOccluded, node)
      node:setAlpha(0)
    end)
    if node.occludedBy then
      hide(node.occludedBy)
    end
  end
  hide(menu.occludedBy)
  hide(menu.occludedMenu)
  forEachRootChild(function(child)
    if child ~= menu and not isWorkshopMenu(child) then
      hide(child)
    end
  end)
end

local function showOccludedUi(menu)
  if menu and menu.hiddenOccluded then
    for _, node in ipairs(menu.hiddenOccluded) do
      pcall(function()
        node:setAlpha(1)
      end)
    end
    menu.hiddenOccluded = nil
  end
  forEachRootChild(function(child)
    if not isWorkshopMenu(child) then
      pcall(function()
        child:setAlpha(1)
      end)
    end
  end)
end

local function setWorldBlur(on)
  pcall(function()
    local client = 0
    if Engine.GetPrimaryController then
      client = Engine.GetPrimaryController() or 0
    end
    if Engine.BlurWorld then
      Engine.BlurWorld(client, on and 2 or 0)
    end
  end)
end

LUI.createMenu.BoiiiWorkshopMenu = function(controller)
  local self = CoD.Menu.NewForUIEditor("BoiiiWorkshopMenu")
  self.soundSet = "default"
  self:setOwner(controller)
  self:setLeftRight(true, true, 0, 0)
  self:setTopBottom(true, true, 0, 0)
  self:playSound("menu_open", controller)
  self.buttonModel = Engine.CreateModel(Engine.GetModelForController(controller), "BoiiiWorkshopMenu.buttonPrompts")
  self.anyChildUsesUpdateState = true
  requestCatalog(currentPage)
  applyZmFrontend(self, controller)
  setWorldBlur(true)
  self.disableBlur = false

  local veil = LUI.UIImage.new()
  veil:setLeftRight(true, true, 0, 0)
  veil:setTopBottom(true, true, 0, 0)
  veil:setRGB(0, 0, 0)
  veil:setAlpha(0.72)
  self:addElement(veil)
  self.MPBackground = veil

  if CoD.FE_Menu_LeftGraphics then
    local leftGfx = CoD.FE_Menu_LeftGraphics.new(self, controller)
    leftGfx:setLeftRight(true, false, 19, 71)
    leftGfx:setTopBottom(true, false, 86, 703.25)
    self:addElement(leftGfx)
    self.FEMenuLeftGraphics = leftGfx
  end

  if CoD.GenericMenuFrame then
    local frame = CoD.GenericMenuFrame.new(self, controller)
    frame:setLeftRight(true, true, 0, 0)
    frame:setTopBottom(true, true, 0, 0)
    frame.titleLabel:setText(Engine.Localize("WORKSHOP"))
    pcall(function()
      frame.cac3dTitleIntermediary0.FE3dTitleContainer0.MenuTitle.TextBox1.Label0:setText(Engine.Localize("WORKSHOP"))
    end)
    frame:setModel(self.buttonModel, controller)
    self:addElement(frame)
    self.MenuFrame = frame
  end

  self.TabButtons = {}
  local tabDefs = {
    { label = "ALL", filter = "all", x1 = 64, x2 = 148 },
    { label = "MAPS", filter = "map", x1 = 152, x2 = 248 },
    { label = "MODS", filter = "mod", x1 = 252, x2 = 348 },
    { label = "INSTALLED", filter = "installed", x1 = 352, x2 = 520 },
    { label = "QUEUE", filter = "queue", x1 = 524, x2 = 640 },
  }
  local function refreshTabs()
    local catalogTab = currentFilter ~= "installed" and currentFilter ~= "queue"
    for _, widget in ipairs(self.PagerWidgets or {}) do
      widget:setAlpha(catalogTab and 1 or 0)
    end
    for _, widget in ipairs(self.QueueWidgets or {}) do
      widget:setAlpha(currentFilter == "queue" and 1 or 0)
    end
    for _, tab in ipairs(self.TabButtons) do
      if tab.filter == currentFilter then
        tab.bg:setRGB(1.0, 0.45, 0.0)
        tab.bg:setAlpha(1)
        tab.label:setRGB(0, 0, 0)
      elseif tab.hovered then
        tab.bg:setRGB(0.26, 0.26, 0.26)
        tab.bg:setAlpha(1)
        tab.label:setRGB(1, 1, 1)
      else
        tab.bg:setRGB(0.12, 0.12, 0.12)
        tab.bg:setAlpha(0.9)
        tab.label:setRGB(0.75, 0.75, 0.75)
      end
    end
  end
  self.refreshTabs = refreshTabs
  for _, def in ipairs(tabDefs) do
    local tab = LUI.UIElement.new()
    tab:setLeftRight(true, false, def.x1, def.x2)
    tab:setTopBottom(true, false, 88, 122)
    enableMouse(tab)
    local bg = LUI.UIImage.new()
    bg:setLeftRight(true, true, 0, 0)
    bg:setTopBottom(true, true, 0, 0)
    tab:addElement(bg)
    local label = makeText(8, def.x2 - def.x1 - 4, 8, 28, "fonts/FoundryGridnik-Medium.ttf", { 1, 1, 1 })
    label:setText(def.label)
    tab:addElement(label)
    local info = { filter = def.filter, bg = bg, label = label }
    local function selectTab()
      if currentFilter ~= def.filter then
        currentFilter = def.filter
        updateDetails(self, nil)
      end
      refreshTabs()
      rebuildList(self)
      return true
    end
    tab:registerEventHandler("leftmouseup", selectTab)
    tab:registerEventHandler("button_action", selectTab)
    makeHoverable(tab, function(hovered)
      info.hovered = hovered
      refreshTabs()
    end)
    self:addElement(tab)
    table.insert(self.TabButtons, info)
  end
  refreshTabs()

  local searchBtn = makeFrontendButton(self, controller, 64, 250, 128, 168, "SEARCH")
  searchBtn:registerEventHandler("button_action", function()
    openSearchKeyboard(self, searchBtn, controller)
    return true
  end)
  searchBtn:registerEventHandler("leftmouseup", function()
    openSearchKeyboard(self, searchBtn, controller)
    return true
  end)
  self:addElement(searchBtn)
  self.SearchButton = searchBtn

  local searchField = LUI.UIElement.new()
  searchField:setLeftRight(true, false, 258, 764)
  searchField:setTopBottom(true, false, 128, 168)
  enableMouse(searchField)
  local searchFieldBg = LUI.UIImage.new()
  searchFieldBg:setLeftRight(true, true, 0, 0)
  searchFieldBg:setTopBottom(true, true, 0, 0)
  searchFieldBg:setRGB(0.08, 0.08, 0.08)
  searchFieldBg:setAlpha(0.95)
  searchField:addElement(searchFieldBg)
  local searchText = makeText(12, 530, 12, 32, "fonts/default.ttf", { 1, 1, 1 })
  searchField:addElement(searchText)
  self.SearchText = searchText
  searchField:registerEventHandler("leftmouseup", function()
    openSearchKeyboard(self, searchField, controller)
    return true
  end)
  makeHoverable(searchField, function(hovered)
    searchFieldBg:setRGB(hovered and 0.16 or 0.08, hovered and 0.16 or 0.08, hovered and 0.16 or 0.08)
  end)
  self:addElement(searchField)
  self.SearchBox = searchField

  local header = LUI.UIElement.new()
  header:setLeftRight(true, false, 64, 764)
  header:setTopBottom(true, false, 174, 198)
  local headerBg = LUI.UIImage.new()
  headerBg:setLeftRight(true, true, 0, 0)
  headerBg:setTopBottom(true, true, 0, 0)
  headerBg:setRGB(0.12, 0.12, 0.12)
  headerBg:setAlpha(0.9)
  header:addElement(headerBg)
  header:addElement(makeNamedColumn(self, controller, 8, 420, "NAME", "name"))
  header:addElement(makeNamedColumn(self, controller, 428, 540, "TYPE", "kind"))
  header:addElement(makeNamedColumn(self, controller, 548, 692, "SIZE", "size"))
  self:addElement(header)
  self.Header = header

  local list = LUI.UIList.new(self, controller, 2, 0, nil, false, false, 0, 0, false, false)
  list:makeFocusable()
  list:setLeftRight(true, false, 64, 764)
  list:setTopBottom(true, false, 200, 536)
  list:setWidgetType(CoD.WorkshopBrowserRow)
  list:setVerticalCount(13)
  list:setSpacing(2)
  if CoD.verticalScrollbar then
    pcall(function()
      list:setVerticalScrollbar(CoD.verticalScrollbar)
    end)
  end
  list:setDataSource("BoiiiWorkshopItems")
  list:registerEventHandler("list_item_gain_focus", function(element)
    updateDetails(self, element)
    return true
  end)
  self:addElement(list)
  self.Items = list

  local function changePage(delta)
    if currentFilter == "installed" or currentFilter == "queue" then
      return
    end
    local nextPage = currentPage + delta
    if nextPage < 1 then
      return
    end
    requestCatalog(nextPage)
    rebuildList(self)
    if self.refreshPages then
      self.refreshPages()
    end
  end

  local function makePagerButton(x1, x2, label, delta)
    local btn = LUI.UIElement.new()
    btn:setLeftRight(true, false, x1, x2)
    btn:setTopBottom(true, false, 544, 580)
    enableMouse(btn)
    local bg = LUI.UIImage.new()
    bg:setLeftRight(true, true, 0, 0)
    bg:setTopBottom(true, true, 0, 0)
    bg:setRGB(0.12, 0.12, 0.12)
    bg:setAlpha(0.9)
    btn:addElement(bg)
    local text = makeText(4, x2 - x1 - 4, 8, 28, "fonts/FoundryGridnik-Medium.ttf", { 0.9, 0.9, 0.9 })
    text:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)
    text:setText(label)
    btn:addElement(text)
    local function onClick()
      changePage(delta)
      return true
    end
    btn:registerEventHandler("leftmouseup", onClick)
    btn:registerEventHandler("button_action", onClick)
    makeHoverable(btn, function(hovered)
      local level = hovered and 0.28 or 0.12
      bg:setRGB(level, level, level)
      text:setRGB(hovered and 1 or 0.9, hovered and 1 or 0.9, hovered and 1 or 0.9)
    end)
    self:addElement(btn)
    return btn
  end
  local prevBtn = makePagerButton(64, 170, "PREV", -1)
  local pageLabel = makeText(180, 390, 544, 580, "fonts/FoundryGridnik-Medium.ttf", { 1, 1, 1 })
  pageLabel:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)
  pageLabel:setText("PAGE 1")
  self:addElement(pageLabel)
  self.PageLabel = pageLabel
  local nextBtn = makePagerButton(400, 506, "NEXT", 1)
  self.PagerWidgets = { prevBtn, pageLabel, nextBtn }

  self.QueueWidgets = {}
  local function makeQueueButton(x1, x2, label, onClick)
    local btn = LUI.UIElement.new()
    btn:setLeftRight(true, false, x1, x2)
    btn:setTopBottom(true, false, 544, 580)
    enableMouse(btn)
    local bg = LUI.UIImage.new()
    bg:setLeftRight(true, true, 0, 0)
    bg:setTopBottom(true, true, 0, 0)
    bg:setRGB(0.12, 0.12, 0.12)
    bg:setAlpha(0.9)
    btn:addElement(bg)
    local text = makeText(4, x2 - x1 - 4, 8, 28, "fonts/FoundryGridnik-Medium.ttf", { 0.9, 0.9, 0.9 })
    text:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)
    text:setText(label)
    btn:addElement(text)
    local function handler()
      if currentFilter ~= "queue" then
        return true
      end
      onClick()
      rebuildList(self)
      refreshActionButton(self)
      return true
    end
    btn:registerEventHandler("leftmouseup", handler)
    btn:registerEventHandler("button_action", handler)
    makeHoverable(btn, function(hovered)
      local level = hovered and 0.28 or 0.12
      bg:setRGB(level, level, level)
      text:setRGB(hovered and 1 or 0.9, hovered and 1 or 0.9, hovered and 1 or 0.9)
    end)
    self:addElement(btn)
    table.insert(self.QueueWidgets, btn)
    return btn
  end
  local function moveSelected(delta)
    local sel = self.wsSel
    if sel and queuePosition(sel.id) > 0 and game.moveworkshopqueue then
      pcall(function()
        game.moveworkshopqueue(sel.id, delta)
      end)
    end
  end
  makeQueueButton(516, 590, "UP", function()
    moveSelected(-1)
  end)
  makeQueueButton(596, 676, "DOWN", function()
    moveSelected(1)
  end)
  makeQueueButton(682, 764, "CLEAR", function()
    if game.clearworkshopqueue then
      pcall(function()
        game.clearworkshopqueue()
      end)
    end
  end)
  refreshTabs()
  local function refreshPages()
    if self.PageLabel then
      local st = catalogStatus()
      local extra = ""
      if st.loading and st.loading ~= 0 and st.loading ~= "false" then
        extra = " ..."
      end
      self.PageLabel:setText("PAGE " .. tostring(currentPage) .. extra)
    end
  end
  self.refreshPages = refreshPages
  refreshPages()

  local detailsBg = LUI.UIImage.new()
  detailsBg:setLeftRight(true, false, 780, 1216)
  detailsBg:setTopBottom(true, false, 128, 572)
  detailsBg:setRGB(0.07, 0.07, 0.07)
  detailsBg:setAlpha(0.92)
  self:addElement(detailsBg)
  self.DetailsBg = detailsBg

  local detailsTitle = makeText(796, 1200, 140, 162, "fonts/RefrigeratorDeluxe-Regular.ttf", { 1, 1, 1 })
  detailsTitle:setText("WORKSHOP")
  self:addElement(detailsTitle)
  self.DetailsTitle = detailsTitle

  local detailsKind = makeText(796, 980, 168, 186, "fonts/default.ttf", { 0.75, 0.75, 0.75 })
  detailsKind:setText("")
  self:addElement(detailsKind)
  self.DetailsKind = detailsKind

  local detailsSize = makeText(990, 1200, 168, 186, "fonts/default.ttf", { 0.75, 0.75, 0.75 })
  detailsSize:setText("")
  self:addElement(detailsSize)
  self.DetailsSize = detailsSize

  local detailsId = makeText(796, 1200, 188, 206, "fonts/default.ttf", { 0.55, 0.55, 0.55 })
  detailsId:setText("")
  self:addElement(detailsId)
  self.DetailsId = detailsId

  local coverBg = LUI.UIImage.new()
  coverBg:setLeftRight(true, false, 796, 972)
  coverBg:setTopBottom(true, false, 214, 390)
  coverBg:setRGB(0.1, 0.1, 0.1)
  coverBg:setAlpha(1)
  pcall(function()
    tintImage(coverBg, "uie_t7_menu_frontend_buttonidlefull")
  end)
  self:addElement(coverBg)
  self.CoverBg = coverBg

  local coverImage = LUI.UIImage.new()
  coverImage:setLeftRight(true, false, 796, 972)
  coverImage:setTopBottom(true, false, 214, 390)
  coverImage:setRGB(1, 1, 1)
  coverImage:setAlpha(1)
  pcall(function()
    coverImage:setImage(RegisterImage("img_t7_mod_preview"))
  end)
  self:addElement(coverImage)
  self.CoverImage = coverImage

  local descHeader = makeText(984, 1160, 214, 230, "fonts/FoundryGridnik-Medium.ttf", { 0.7, 0.7, 0.7 })
  descHeader:setText("DESCRIPTION")
  self:addElement(descHeader)
  local descHint = makeText(1160, 1200, 214, 230, "fonts/default.ttf", { 0.55, 0.55, 0.55 })
  descHint:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_RIGHT)
  descHint:setText("")
  self:addElement(descHint)
  self.DescScrollHint = descHint

  local descClip = LUI.UIElement.new()
  descClip:setLeftRight(true, false, 984, 1200)
  descClip:setTopBottom(true, false, 234, 390)
  enableMouse(descClip)
  self:addElement(descClip)
  self.DescClip = descClip

  self.DescLines = {}
  for i = 1, DESC_VISIBLE do
    local top = (i - 1) * 18
    local line = makeText(0, 216, top, top + 16, "fonts/default.ttf", { 0.9, 0.9, 0.9 })
    descClip:addElement(line)
    self.DescLines[i] = line
  end
  descClip:registerEventHandler("leftmouseup", function()
    scrollDescription(self, 1)
    return true
  end)
  descClip:registerEventHandler("rightmouseup", function()
    scrollDescription(self, -1)
    return true
  end)
  descClip:registerEventHandler("mousewheel", function(element, event)
    local delta = 1
    pcall(function()
      delta = tonumber(event.wheelDelta or event.delta or event.y) or 1
      if delta > 0 then
        delta = -1
      else
        delta = 1
      end
    end)
    scrollDescription(self, delta)
    return true
  end)
  setDescription(self, "Select an item from the list.")

  local progressHeader = makeText(796, 1200, 404, 420, "fonts/FoundryGridnik-Medium.ttf", { 0.7, 0.7, 0.7 })
  progressHeader:setText("DOWNLOAD")
  self:addElement(progressHeader)

  local progressLabel = makeText(796, 1200, 422, 438, "fonts/default.ttf", { 0.95, 0.95, 0.95 })
  progressLabel:setText("")
  self:addElement(progressLabel)
  self.ProgressLabel = progressLabel

  local function makeBarImage(r, g, b)
    local img = LUI.UIImage.new()
    img:setRGB(r, g, b)
    img:setAlpha(1)
    pcall(function()
      img:setImage(RegisterImage("white"))
    end)
    return img
  end

  local progressTrack = makeBarImage(0.28, 0.28, 0.28)
  progressTrack:setLeftRight(true, false, PROGRESS_BAR_LEFT, PROGRESS_BAR_RIGHT)
  progressTrack:setTopBottom(true, false, 444, 468)
  self:addElement(progressTrack)
  self.ProgressTrack = progressTrack

  local progressFill = makeBarImage(1.0, 0.45, 0.0)
  progressFill:setLeftRight(true, false, PROGRESS_BAR_LEFT, PROGRESS_BAR_LEFT)
  progressFill:setTopBottom(true, false, 444, 468)
  self:addElement(progressFill)
  self.ProgressFill = progressFill

  local progressStatus = makeText(796, 1200, 474, 490, "fonts/default.ttf", { 0.7, 0.7, 0.7 })
  progressStatus:setText("Press ENTER or DOWNLOAD")
  self:addElement(progressStatus)
  self.ProgressStatus = progressStatus

  local downloadBtn = makeFrontendButton(self, controller, 796, 1200, 498, 544, "DOWNLOAD")
  downloadBtn:registerEventHandler("button_action", function()
    runSelectionAction(self)
    return true
  end)
  downloadBtn:registerEventHandler("leftmouseup", function()
    runSelectionAction(self)
    return true
  end)
  self:addElement(downloadBtn)
  self.DownloadButton = downloadBtn

  setKeywordLabel(self, getSearchQuery())

  local timerOk = pcall(function()
    self.DownloadTimer = LUI.UITimer.newElementTimer(250, false, function()
      tickWorkshopDownload(self)
    end)
    self:addElement(self.DownloadTimer)
  end)
  if not timerOk then
    pcall(function()
      self.DownloadTimer = LUI.UITimer.new(250, "workshop_dl_tick", false, self)
      self:addElement(self.DownloadTimer)
      self:registerEventHandler("workshop_dl_tick", function()
        tickWorkshopDownload(self)
        return true
      end)
    end)
  end
  tickWorkshopDownload(self)

  local function handleKeyboard(element, event)
    local isOurs = self.workshopSearchPending
    local typeOk = false
    pcall(function()
      if event and event.type and Enum and Enum.KeyboardType then
        typeOk = event.type == Enum.KeyboardType.KEYBOARD_TYPE_SERVER_FILTER_KEYWORDS
          or event.type == Enum.KeyboardType.KEYBOARD_TYPE_NORMAL
      end
    end)
    if not isOurs and not typeOk then
      return false
    end
    self.workshopSearchPending = false
    local text = ""
    if event then
      text = event.input or event.text or ""
    end
    pcall(function()
      local fromDvar = Engine.DvarString(nil, "ui_keyboard_dvar_edit")
      if fromDvar ~= nil then
        text = fromDvar
      end
    end)
    applySearch(self, text)
    return true
  end
  self:registerEventHandler("ui_keyboard_input", handleKeyboard)
  self:registerEventHandler("ui_keyboard_complete", handleKeyboard)

  self:AddButtonCallbackFunction(
    list,
    controller,
    Enum.LUIButton.LUI_KEY_XBA_PSCROSS,
    "ENTER",
    function(element, menu, actionController)
      actOnElement(self, element)
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
    Enum.LUIButton.LUI_KEY_START,
    "F",
    function(element, menu, actionController)
      openSearchKeyboard(self, element, actionController)
      return true
    end,
    function(element, menu)
      CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_START, "PLATFORM_KEYWORDS_CAPS")
      return true
    end,
    false
  )

  list.id = "Items"
  self:processEvent({ name = "menu_loaded", controller = controller })
  self:processEvent({ name = "update_state", menu = self })
  hideOccludedUi(self)
  pcall(function()
    self.HideLobbyTimer = LUI.UITimer.newElementTimer(80, true, function()
      hideOccludedUi(self)
    end)
    self:addElement(self.HideLobbyTimer)
  end)
  pcall(function()
    LUI.OverrideFunction_CallOriginalFirst(self, "setOccludedBy", function(element, other)
      hideOccludedUi(element)
    end)
  end)
  self:registerEventHandler("occlusion_change", function(element, event)
    local occluded = event and event.occluded
    if occluded ~= true then
      hideOccludedUi(element)
    end
    return false
  end)
  if not self:restoreState() then
    list:processEvent({ name = "gain_focus", controller = controller })
  end

  LUI.OverrideFunction_CallOriginalSecond(self, "close", function(element)
    pcall(function()
      setWorldBlur(false)
    end)
    pcall(function()
      showOccludedUi(element)
    end)
    pcall(function()
      restoreFrontendRoom(element, controller)
    end)
    pcall(function()
      if game.setworkshopcover then
        game.setworkshopcover("")
      end
    end)
    if element.MenuFrame then
      element.MenuFrame:close()
    end
    if element.FEMenuLeftGraphics then
      element.FEMenuLeftGraphics:close()
    end
    if element.Items then
      element.Items:close()
    end
    if element.DownloadTimer then
      pcall(function()
        element:removeElement(element.DownloadTimer)
      end)
    end
    if element.DownloadButton and element.DownloadButton.close then
      element.DownloadButton:close()
    end
    if element.SearchButton and element.SearchButton.close then
      element.SearchButton:close()
    end
    if element.MPBackground and element.MPBackground.close then
      element.MPBackground:close()
    end
    Engine.UnsubscribeAndFreeModel(
      Engine.GetModel(Engine.GetModelForController(controller), "BoiiiWorkshopMenu.buttonPrompts")
    )
  end)

  return self
end
