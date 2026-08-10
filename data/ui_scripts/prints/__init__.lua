if not Engine.IsInGame() then
  return
end

if not pcall(require, "ui.uieditor.widgets.HUD.Console.Console") then
  return
end

if not CoD or not CoD.ConsoleEntry or type(CoD.ConsoleEntry.new) ~= "function" then
  return
end

local verticalOffset = -418
local printScale = 1.1

local function shiftElement(element, amount)
  if not element or not element.getLocalTopBottom then
    return
  end

  local topAnchor, bottomAnchor, top, bottom = element:getLocalTopBottom()
  if top == nil or bottom == nil then
    return
  end

  element:setTopBottom(topAnchor, bottomAnchor, top + amount, bottom + amount)
end

local function scaleElement(element, scale)
  if element and element.setScale then
    element:setScale(scale)
  end
end

local function positionConsoleEntry(instance)
  if not instance then
    return
  end

  local currentOffset = instance.boiiiConsolePositionOffset or 0
  local amount = verticalOffset - currentOffset
  if amount ~= 0 then
    shiftElement(instance.Panel, amount)
    shiftElement(instance.Text0, amount)
    shiftElement(instance.Text1, amount)
    shiftElement(instance.Icon0, amount)
    shiftElement(instance.Icon1, amount)
    instance.boiiiConsolePositionOffset = verticalOffset
  end

  scaleElement(instance.Text0 and instance.Text0.Text, 1)
  scaleElement(instance.Text1 and instance.Text1.Text, 1)
  scaleElement(instance.Text0 and instance.Text0.TextShadow, 1)
  scaleElement(instance.Text1 and instance.Text1.TextShadow, 1)
  scaleElement(instance.Icon0 and instance.Icon0.Icon, 1)
  scaleElement(instance.Icon1 and instance.Icon1.Icon, 1)
  scaleElement(instance.Panel, 1)
  scaleElement(instance, printScale)
end

local widget = CoD.ConsoleEntry
if widget.boiiiPrintsNew ~= widget.new then
  local originalNew = widget.new
  local positionedNew = function(menu, controller)
    local instance = originalNew(menu, controller)
    positionConsoleEntry(instance)
    return instance
  end

  widget.new = positionedNew
  widget.boiiiPrintsNew = positionedNew
end

local function ensureZombieConsole(hud, controller)
  if not CoD.isZombie or not hud or hud.Console or not hud.addElement then
    return
  end

  if not CoD.Console or type(CoD.Console.new) ~= "function" then
    return
  end

  local console = CoD.Console.new(hud, controller or 0)
  console:setLeftRight(true, false, 39, 809)
  console:setTopBottom(false, true, -160, -20)
  console:setAlpha(1)
  hud:addElement(console)
  hud.Console = console
end

if type(HUD_FirstSnapshot_Zombie) == "function" and boiiiPrintsFirstSnapshot ~= HUD_FirstSnapshot_Zombie then
  local originalFirstSnapshot = HUD_FirstSnapshot_Zombie
  local firstSnapshot = function(hud, event)
    originalFirstSnapshot(hud, event)
    ensureZombieConsole(hud, event and event.controller or 0)
  end

  HUD_FirstSnapshot_Zombie = firstSnapshot
  boiiiPrintsFirstSnapshot = firstSnapshot
end

local function positionExistingEntries(element, depth, controller)
  if not element or depth > 64 then
    return
  end

  if element.T7HudMenuGameMode then
    ensureZombieConsole(element, controller)
  end

  if element.id == "ConsoleEntry" then
    positionConsoleEntry(element)
  end

  if not element.getFirstChild then
    return
  end

  local child = element:getFirstChild()
  while child do
    positionExistingEntries(child, depth + 1, controller)
    if child.getNextSibling then
      child = child:getNextSibling()
    else
      child = nil
    end
  end
end

if LUI and LUI.roots then
  for name, root in pairs(LUI.roots) do
    if type(root) == "table" then
      local controller = tonumber(tostring(name):match("(%d+)$")) or 0
      positionExistingEntries(root, 0, controller)
    end
  end
end
