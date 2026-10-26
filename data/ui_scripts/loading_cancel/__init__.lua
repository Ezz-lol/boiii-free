-- The loading screen is built before UI scripts run and lives in the in-game
-- Lua context, which does not ship the frontend button widgets. So instead of
-- wrapping the menu, the button is attached to the existing menu when this
-- script runs, and assembled from the same pieces the server browser rows use.
--
-- In-game UI scripts run twice per map: once right after the UI is initialised,
-- while the loading screen is up, and again on the first snapshot, once it is
-- done. The first run adds the button, and the second releases menu input if
-- the loading menu went away without closing cleanly.
--
-- Aborting a map load part way is not safe (the engine frees level memory the
-- loader is still using, which crashed the game and then the GPU driver on the
-- next map). So pressing the button only records the request, and the actual
-- disconnect happens once loading has finished, before the match starts.

if game.ismodrawfile("ui_mp/T6/HUD/Loading.lua") then
  return
end

if not pcall(require, "ui.uieditor.widgets.Lobby.Common.FE_FocusBarContainer") then
  return
end

-- Placement: bottom left, sitting above the loading screen's bottom bar
local BUTTON_LEFT = 64
local BUTTON_WIDTH = 140
local BUTTON_HEIGHT = 32
local BUTTON_BOTTOM_OFFSET = 82 -- from the bottom edge, clears the hint and loading bar

local IDLE_SHADE = 0.1
local HOVER_SHADE = 0.3

local MAX_SCANNED_ELEMENTS = 200
local MAX_SCAN_DEPTH = 2

local uiActive = __boiiiLoadingCancelUiActive or {}
__boiiiLoadingCancelUiActive = uiActive

-- During loading the engine is still in gameplay input mode, so the mouse is
-- captured and never reaches the UI. Menus opened in-game switch it with this
-- pair of calls, and it has to be switched back once the loading menu is gone.
local function setUiActive(controller, active)
  if (uiActive[controller] or false) == active then
    return
  end
  uiActive[controller] = active or nil
  Engine.LockInput(controller, active)
  Engine.SetUIActive(controller, active)
end

-- Controller that asked to leave, kept on the Lua state so the second script
-- run (first snapshot) can see it. nil when nobody asked.
local function leaveNow(controller)
  __boiiiLoadingCancelPending = nil
  setUiActive(controller, false)
  Engine.PrintInfo(Enum.consoleLabel.LABEL_DEFAULT, "Loading finished, leaving as requested.\n")
  Engine.Exec(controller, "disconnect")
end

local function requestLeave(controller)
  if __boiiiLoadingCancelPending ~= nil then
    return false
  end
  __boiiiLoadingCancelPending = controller
  return true
end

local function createButton(menu, controller)
  local button = LUI.UIElement.new()
  button:setLeftRight(true, false, BUTTON_LEFT, BUTTON_LEFT + BUTTON_WIDTH)
  button:setTopBottom(false, true, -(BUTTON_BOTTOM_OFFSET + BUTTON_HEIGHT), -BUTTON_BOTTOM_OFFSET)
  button:setPriority(1000)
  button:setHandleMouse(true)
  if button.setHandleMouseMove then
    button:setHandleMouseMove(true)
  end
  button:setForceMouseEventDispatch(true)

  local background = LUI.UIImage.new()
  background:setLeftRight(true, true, 0, 0)
  background:setTopBottom(true, true, 0, 0)
  background:setRGB(IDLE_SHADE, IDLE_SHADE, IDLE_SHADE)
  background:setAlpha(0.85)
  button:addElement(background)

  local label = LUI.UIText.new()
  label:setLeftRight(true, true, 0, 0)
  label:setTopBottom(true, false, 5, 27)
  label:setTTF("fonts/escom.ttf")
  label:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)
  label:setText(Engine.Localize("CANCEL"))
  button:addElement(label)

  local focusBarB = CoD.FE_FocusBarContainer.new(menu, controller)
  focusBarB:setLeftRight(true, true, -2, 2)
  focusBarB:setTopBottom(false, true, -1, 3)
  focusBarB:setAlpha(0)
  focusBarB:setZoom(1)
  button:addElement(focusBarB)

  local focusBarT = CoD.FE_FocusBarContainer.new(menu, controller)
  focusBarT:setLeftRight(true, true, -2, 2)
  focusBarT:setTopBottom(true, false, -2, 2)
  focusBarT:setAlpha(0)
  focusBarT:setZoom(1)
  button:addElement(focusBarT)

  -- The loading menu does not own focus, so the highlight follows the mouse
  local hovered = false
  local pending = false
  local function setHover(on)
    if pending or on == hovered then
      return
    end
    hovered = on
    local shade = on and HOVER_SHADE or IDLE_SHADE
    background:setRGB(shade, shade, shade)
    focusBarB:setAlpha(on and 1 or 0)
    focusBarT:setAlpha(on and 1 or 0)
  end

  button:registerEventHandler("mouseenter", function()
    setHover(true)
  end)
  button:registerEventHandler("mouseleave", function()
    setHover(false)
  end)
  button:registerEventHandler("mousemove", function(element, event)
    if element.IsMouseInsideElement then
      local ok, inside = pcall(element.IsMouseInsideElement, element, event)
      if ok then
        setHover(inside and true or false)
      end
    end
  end)
  -- Leaving waits for the load to finish, so show that the click was taken
  function button.requestLeave()
    if pending or not requestLeave(controller) then
      return
    end
    setHover(false)
    pending = true
    label:setText(Engine.Localize("LEAVING..."))
    label:setAlpha(0.6)
  end

  button:registerEventHandler("leftmousedown", function(element)
    element.requestLeave()
    return true
  end)

  LUI.OverrideFunction_CallOriginalSecond(button, "close", function()
    focusBarB:close()
    focusBarT:close()
  end)

  return button
end

local function addCancelButton(menu, controller)
  -- Hosts are loading their own match, so leaving would tear it down
  if Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_GAME) then
    return
  end

  local button = createButton(menu, controller)

  menu:setForceMouseEventDispatch(true)
  menu:AddButtonCallbackFunction(menu, controller, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE, nil, function()
    button.requestLeave()
    return true
  end, function()
    return true
  end, false)

  menu:addElement(button)
  menu.cancelButton = button

  LUI.OverrideFunction_CallOriginalSecond(menu, "close", function(element)
    element.cancelButton:close()
    setUiActive(controller, false)
    -- The loading menu closes once loading is done
    if __boiiiLoadingCancelPending ~= nil then
      leaveNow(__boiiiLoadingCancelPending)
    end
  end)

  setUiActive(controller, true)
  if ShowMouseCursor then
    pcall(ShowMouseCursor, menu, menu, controller, menu)
  end
end

local function findLoadingMenus(element, depth, found, count)
  local child = element:getFirstChild()
  while child and count.n < MAX_SCANNED_ELEMENTS do
    count.n = count.n + 1
    if child.menuName == "Loading" or child.id == "Loading" then
      found[#found + 1] = child
    end
    if depth < MAX_SCAN_DEPTH then
      findLoadingMenus(child, depth + 1, found, count)
    end
    child = child:getNextSibling()
  end
end

local function forEachRoot(fn)
  for controller = 0, Engine.GetMaxLocalControllers() - 1 do
    local root = LUI.roots["UIRoot" .. controller]
    if root then
      fn(root, controller)
    end
  end
end

local function update()
  forEachRoot(function(root, controller)
    local found = {}
    findLoadingMenus(root, 0, found, { n = 0 })

    for _, menu in ipairs(found) do
      if not menu.cancelButton and not menu.cancelButtonFailed then
        local ok = pcall(addCancelButton, menu, menu.m_ownerController or controller)
        if not ok then
          menu.cancelButtonFailed = true
        end
      end
    end

    -- Never leave menu input on once the loading screen is gone
    if #found == 0 then
      setUiActive(controller, false)
    end
  end)
end

-- A run that starts with a pending request is the one on the first snapshot,
-- so loading has finished
if __boiiiLoadingCancelPending ~= nil then
  leaveNow(__boiiiLoadingCancelPending)
else
  update()
end
