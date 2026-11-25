local MP = Enum.eModes.MODE_MULTIPLAYER
local hints = {
  active = "Equipped this match.",
  queued = "Saved for next match.",
  rejected = "The server rejected the change.",
  offline = "No server response. Select to retry.",
  waiting = "Waiting for the server...",
  retry = "Retry",
  points = "%d points",
}
local sessions = {}
local serial = 0

local function send(controller, slots)
  local session = sessions[controller]
  serial = serial % 999999 + 1
  session.serial = serial
  session.ticks = 0
  session.request = "ks2:" .. controller .. ":" .. serial
  if slots then
    session.request = session.request .. ":" .. table.concat(slots, ":")
  end
  local reply = "boiii_streakReply" .. controller
  Engine.SetDvar(reply, "waiting")
  Engine.SendMenuResponse(controller, "ingame_options", session.request)
end

local function decodeReply(value, serial)
  local id, state, a, b, c = value:match("^(%d+):(%a+):(%d+):(%d+):(%d+)$")
  if tonumber(id) ~= serial or (state ~= "active" and state ~= "queued" and state ~= "rejected") then
    return
  end
  local slots = { tonumber(a), tonumber(b), tonumber(c) }
  for _, item in ipairs(slots) do
    if item > 255 or (item ~= 0 and Engine.GetItemGroup(item, MP) ~= "killstreak") then
      return
    end
  end
  return slots, state
end

local function poll(menu, controller)
  local session = sessions[controller]
  if not session or not session.request then
    return
  end
  local value = Engine.DvarString(nil, "boiii_streakReply" .. controller) or ""
  local slots, state = decodeReply(value, session.serial)
  if slots then
    session.slots = slots
    session.state = state
    session.request = nil
    menu.Options:updateDataSource()
    return
  end
  session.ticks = session.ticks + 1
  if session.ticks == 10 then
    Engine.SendMenuResponse(controller, "ingame_options", session.request)
  elseif session.ticks >= 25 then
    session.request = nil
    session.slots = nil
    session.state = "offline"
    menu.Options:updateDataSource()
  end
end

local function bindIcon(image, element, suffix)
  image:linkToElementModel(element, "image", true, function(model)
    local base = Engine.GetModelValue(model)
    if base and base ~= "" then
      image:setImage(RegisterImage(base .. suffix))
      image:setAlpha(1)
    else
      image:setAlpha(0)
    end
  end)
end

local Tile = InheritFrom(LUI.UIElement)
Tile.new = function(menu, controller)
  local self = LUI.UIElement.new()
  self:setClass(Tile)
  self.id = "BoiiiScorestreakTile"
  self:makeFocusable()
  self:setHandleMouse(true)
  self:setLeftRight(true, false, 0, 122)
  self:setTopBottom(true, false, 0, 104)
  local background = LUI.UIImage.new()
  background:setLeftRight(true, true, 0, 0)
  background:setTopBottom(true, true, 0, 0)
  background:setRGB(0.12, 0.12, 0.12)
  self:addElement(background)
  local icon = LUI.UIImage.new()
  icon:setLeftRight(false, false, -39, 39)
  icon:setTopBottom(true, false, 2, 80)
  icon:setAlpha(0)
  self:addElement(icon)
  bindIcon(icon, self, "_menu")
  local name = LUI.UIText.new()
  name:setLeftRight(true, true, 3, -3)
  name:setTopBottom(false, true, -24, -6)
  name:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
  name:linkToElementModel(self, "displayText", true, function(model)
    name:setText(Engine.GetModelValue(model) or "")
  end)
  self:addElement(name)
  self.clipsPerState = {
    DefaultState = {
      DefaultClip = function()
        background:setRGB(0.12, 0.12, 0.12)
      end,
      Focus = function()
        background:setRGB(0.62, 0.28, 0.04)
      end,
    },
  }
  return self
end

return function(createListMenu, option)
  local title = Engine.Localize("MENU_SCORE_STREAKS_CAPS")
  local function entry(item, prefix, help, action)
    local name = Engine.Localize(item == 0 and "MENU_NONE" or Engine.GetItemName(item, MP))
    local row = option(prefix .. name, help, action)
    row.models.image = item ~= 0 and Engine.GetItemImage(item, MP) or ""
    return row
  end

  DataSources.BoiiiScorestreakSlots = ListHelper_SetupDataSource("BoiiiScorestreakSlots", function(controller)
    local session = sessions[controller]
    if not session or not session.slots or session.request then
      local offline = session and session.state == "offline" and not session.request
      local row = option(offline and hints.retry or "...", hints[offline and "offline" or "waiting"], function(menu)
        if offline then
          send(controller)
          menu.Options:updateDataSource()
        end
      end)
      row.models.image = ""
      return { row }
    end
    local rows = {}
    for slot, item in ipairs(session.slots) do
      table.insert(
        rows,
        entry(item, slot .. ". ", hints[session.state], function(menu)
          session.slot = slot
          OpenPopup(menu, "BoiiiScorestreakItems", controller)
        end)
      )
    end
    return rows
  end, true)

  DataSources.BoiiiScorestreakItems = ListHelper_SetupDataSource("BoiiiScorestreakItems", function(controller)
    local session = sessions[controller]
    local occupied = {}
    for slot, item in ipairs(session.slots) do
      if slot ~= session.slot then
        occupied[item] = true
      end
    end
    local rows = {}
    local function add(item)
      local cost = item ~= 0 and Engine.GetItemMomentumCost(item, MP)
      local help = cost and string.format(hints.points, cost) or ""
      table.insert(
        rows,
        entry(item, "", help, function(menu)
          local nextSlots = { session.slots[1], session.slots[2], session.slots[3] }
          nextSlots[session.slot] = item
          send(controller, nextSlots)
          GoBack(menu, controller)
          session.menu.Options:updateDataSource()
        end)
      )
    end
    add(0)
    local items = Engine.GetUnlockablesByGroupName("killstreak", MP)
    table.sort(items, CoD.CACRewardComparisonFunction)
    for _, item in ipairs(items) do
      if
        not occupied[item]
        and Engine.GetItemRestrictionState(item) ~= Enum.ItemRestrictionState.ITEM_RESTRICTION_STATE_RESTRICTED
        and (
          not IsProgressionEnabled(controller)
          or (not Engine.IsItemLocked(controller, item, MP) and Engine.IsItemPurchased(controller, item, MP))
        )
      then
        add(item)
      end
    end
    return rows
  end, true)

  local function grid(menu, list, controller)
    list:setLeftRight(true, false, 64, 702)
    list:setTopBottom(true, false, 136, 585)
    list:setWidgetType(Tile)
    list:setSpacing(6)
    list:setHorizontalCount(5)
    list:setVerticalCount(4)
    local preview = LUI.UIImage.new()
    preview:setLeftRight(true, false, 800, 1120)
    preview:setTopBottom(true, false, 232, 552)
    preview:setAlpha(0)
    menu:addElement(preview)
    bindIcon(preview, list, "_menu_large")
  end
  createListMenu("BoiiiScorestreakItems", title, "BoiiiScorestreakItems", grid)
  createListMenu("BoiiiScorestreakSlots", title, "BoiiiScorestreakSlots", function(menu, list, controller)
    sessions[controller] = { menu = menu }
    grid(menu, list, controller)
    list:setHorizontalCount(3)
    list:setVerticalCount(1)
    send(controller)
    menu:addElement(LUI.UITimer.newElementTimer(100, false, function()
      poll(menu, controller)
    end))
    LUI.OverrideFunction_CallOriginalFirst(menu, "close", function()
      sessions[controller] = nil
    end)
  end)
end
