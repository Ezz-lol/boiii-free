-- SPDX-License-Identifier: GPL-3.0-only
-- Independently authored extension using the retail ZMScr / PlayerListZM APIs.
-- Display positions are NOT native client slots: the stock datasource puts the
-- local player first, compacts connected players, and publishes actual clientNum.
local rows = {}
local unpackValues = unpack
local function pack(...) return { n = select("#", ...), ... } end
local function integer(value, low, high)
  return type(value) == "number" and value == math.floor(value) and value >= low and value <= high
end

local function attach(widget, menu, controller, limit, capacity)
  if (type(widget) ~= "table" and type(widget) ~= "userdata")
    or widget.boiiiCoopRows or not widget.ListingUser or not widget.Listing2
    or not widget.Listing3 or not widget.Listing4 or type(widget.close) ~= "function"
    or widget.Listing5 or widget.ZMScrPlusPoints4
    or type(widget.Listing4.addElement) ~= "function" then return end
  local controllerModel = Engine.GetModelForController(controller)
  if not controllerModel then return end
  local playerList = Engine.CreateModel(controllerModel, "PlayerList")
  local owned = { closed = false, items = {} }
  widget.boiiiCoopRows = owned
  local function closePopups(item)
    for popup in pairs(item.popups) do
      item.popups[popup] = nil
      popup:close()
    end
    item.popupCount = 0
  end
  local function cleanup()
    if owned.closed then return end
    owned.closed = true
    for _, item in ipairs(owned.items) do
      closePopups(item)
      if item.listing then item.listing:close() end
      item.pane:close()
    end
  end
  owned.cleanup = cleanup
  local close = widget.close
  widget.close = function(self, ...)
    cleanup()
    return close(self, ...)
  end
  for displayIndex = 4, limit - 1 do
    local model = Engine.CreateModel(playerList, tostring(displayIndex))
    local pane = LUI.UIElement.new()
    pane:setLeftRight(true, false, 0, 85)
    local top = -26 * (displayIndex - 3)
    pane:setTopBottom(true, false, top, top + 35)
    pane:setAlpha(0)
    -- Listing4 receives the retail root's start/hide animations. Its own state
    -- animates its children, so the new sibling panes inherit the HUD root alpha
    -- without copying/replacing the retail clips or their completion counters.
    widget.Listing4:addElement(pane)
    local item = { pane = pane, popups = {}, popupCount = 0, sequence = 0 }
    owned.items[#owned.items + 1] = item
    local listing = CoD.ZMScr_ListingSm.new(menu, controller)
    listing:setLeftRight(true, false, 0, 85)
    listing:setTopBottom(true, false, 0, 35)
    pane:addElement(listing)
    item.listing = listing
    for _, key in ipairs({ "zombieInventoryIcon", "zombieWearableIcon" }) do
      local icon = Engine.CreateModel(model, key)
      if Engine.GetModelValue(icon) == nil then Engine.SetModelValue(icon, "blacktransparent") end
    end
    local connected = Engine.CreateModel(model, "playerConnected")
    local clientNum = Engine.CreateModel(model, "clientNum")
    local shown = Engine.CreateModel(model, "playerScoreShown")
    local score = Engine.CreateModel(model, "playerScore")
    local function refresh()
      if owned.closed then return end
      local slot = Engine.GetModelValue(clientNum)
      local currentLimit = capacity()
      local present = Engine.GetModelValue(connected)
      local scoreShown = Engine.GetModelValue(shown)
      local connectedCount = 0
      local actualPlayer
      if currentLimit then
        for slotIndex = 0, currentLimit - 1 do
          local data = Engine.GetPlayerListData(controller, slotIndex)
          if slotIndex == slot then actualPlayer = data end
          if type(data) == "table" and (data.playerConnected == true or data.playerConnected == 1) then
            connectedCount = connectedCount + 1
          end
        end
      end
      local visible = currentLimit and integer(slot, 0, currentLimit - 1)
        and displayIndex < connectedCount
        and type(actualPlayer) == "table" and (actualPlayer.playerConnected == true or actualPlayer.playerConnected == 1)
        and (present == true or present == 1) and (scoreShown == true or scoreShown == 1)
      if item.slot ~= slot or not visible then
        closePopups(item)
        item.lastScore = nil
      end
      item.slot = slot
      item.visible = visible and true or false
      pane:setAlpha(item.visible and 1 or 0)
    end
    -- Normal UIElement subscriptions are owned by this pane and are released on
    -- close; there is no timer, global subscription list or per-frame polling.
    for _, field in ipairs({ connected, clientNum, shown }) do
      pane:subscribeToModel(field, refresh, false)
    end
    for _, key in ipairs({ "playerConnected", "playerDisconnected" }) do
      pane:subscribeToModel(Engine.CreateModel(controllerModel, key), refresh, false)
    end
    listing:setModel(model, controller)
    refresh()
    local initialScore = Engine.GetModelValue(score)
    item.lastScore = item.visible and integer(initialScore, -2147483648, 2147483647) and initialScore or nil
    pane:subscribeToModel(score, function(changed)
      if owned.closed then return end
      refresh()
      local value = Engine.GetModelValue(changed)
      local before = item.lastScore
      item.lastScore = integer(value, -2147483648, 2147483647) and value or nil
      if not item.visible or before == nil or item.lastScore == nil then return end
      local delta = value - before
      -- Aggregate the engine's actual score delta (including bonuses/expenses),
      -- rather than replaying counters and double-counting damage/death awards.
      if delta == 0 or delta < -10000 or delta > 10000 or item.popupCount >= 8
        or type(CoD.ZMScr_PlusPointsContainer) ~= "table"
        or type(CoD.ZMScr_PlusPointsContainer.new) ~= "function" then return end
      local popup = CoD.ZMScr_PlusPointsContainer.new(menu, controller)
      local points = popup and popup.ZMScrPlusPoints
      if not points or not points.Label1 or not points.Label2 then
        if popup and type(popup.close) == "function" then popup:close() end
        return
      end
      popup:setLeftRight(true, false, 70.72, 155.72)
      popup:setTopBottom(true, false, -18.38, 47.38)
      popup:setScale(0.75)
      local text = delta > 0 and ("+" .. delta) or tostring(delta)
      points.Label1:setText(text)
      points.Label2:setText(text)
      points:playClip(delta > 0 and "DefaultClip" or "NegativeScore")
      item.sequence = item.sequence % 5 + 1
      item.popups[popup] = true
      item.popupCount = item.popupCount + 1
      popup:registerEventHandler("clip_over", function(element)
        if item.popups[element] then
          item.popups[element] = nil
          item.popupCount = item.popupCount - 1
          element:close()
        end
      end)
      pane:addElement(popup)
      popup:setState("DefaultState")
      popup:playClip("Anim" .. item.sequence)
    end, false)
  end
end

function rows.install(capacity)
  if type(capacity) ~= "function" or type(CoD) ~= "table" or type(Engine) ~= "table" then return end
  -- Load the existing retail widget explicitly before the first HUD constructor.
  -- No require wrapper: native loaders rely on its immediate calling script.
  if type(CoD.ZMScr) ~= "table" and not rows.preloadAttempted and capacity() then
    rows.preloadAttempted = true
    pcall(function() require("ui.uieditor.widgets.HUD.ZM_Score.ZMScr") end)
  end
  local class = CoD.ZMScr
  local original = type(class) == "table" and class.new or nil
  if type(original) ~= "function" or original == rows.wrapper then return end
  local wrapper = function(menu, controller, ...)
    local result = pack(original(menu, controller, ...))
    local limit = capacity()
    if limit and type(CoD.ZMScr_ListingSm) == "table" and type(CoD.ZMScr_ListingSm.new) == "function"
      and type(LUI) == "table" and type(LUI.UIElement) == "table" and type(LUI.UIElement.new) == "function"
      and type(Engine.GetModelForController) == "function" and type(Engine.CreateModel) == "function"
      and type(Engine.GetPlayerListData) == "function"
      and type(Engine.GetModelValue) == "function" and type(Engine.SetModelValue) == "function" then
      -- Constructor errors from the original are preserved; an unsupported mod
      -- widget is left intact and an extension failure cannot break HUD creation.
      local ok = pcall(attach, result[1], menu, controller, limit, capacity)
      if not ok then
        pcall(function()
          local owned = result[1] and result[1].boiiiCoopRows
          if owned and type(owned.cleanup) == "function" then owned.cleanup() end
        end)
      end
    end
    return unpackValues(result, 1, result.n)
  end
  rows.wrapper = wrapper
  class.new = wrapper
end

return rows
