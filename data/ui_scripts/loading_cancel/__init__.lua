-- Adds a Cancel button to the loading screen.
--
-- The loading menu already exists when UI scripts run, so the button is added to
-- it directly. UI scripts run twice per match: once while loading, and again
-- on the first snapshot once loading is done.

if game.ismodrawfile("ui_mp/T6/HUD/Loading.lua") then
  return
end

local BUTTON_LEFT = 64
local BUTTON_BOTTOM_OFFSET = 82 -- clears the loading bar and hint text
local BUTTON_WIDTH_MAX = 280
local BUTTON_HEIGHT = 32
local TEXT_LEFT = 12
local TEXT_RIGHT = 284

local FOCUS_TWEEN_MS = 50

local MAX_SCANNED_ELEMENTS = 200
local MAX_SCAN_DEPTH = 2

local uiActive = __boiiiLoadingCancelUiActive or {}
__boiiiLoadingCancelUiActive = uiActive

-- The mouse is captured during loading, so the UI has to be made active to use it
local function setUiActive(controller, active)
  if (uiActive[controller] or false) == active then
    return
  end
  uiActive[controller] = active or nil
  Engine.LockInput(controller, active)
  Engine.SetUIActive(controller, active)
end

-- The pending request lives on the Lua state so the second script run sees it
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

local function layerStates()
  return {
    overlay = {
      idle = { alpha = 0.4 },
      focus = { alpha = 0.4 },
    },
    idleFrame = {
      idle = { alpha = 1, zoom = 0 },
      focus = { alpha = 0, zoom = 10 },
    },
    focusFrame = {
      idle = { tb = { false, false, -7, 8 }, alpha = 0, zoom = 0 },
      focus = { tb = { false, false, -20, 20 }, alpha = 1, zoom = 10 },
    },
    glow = {
      idle = { alpha = 0 },
      focus = { alpha = 0.45 },
    },
    text = {
      idle = { rgb = { 0.79, 0.79, 0.79 } },
      focus = { rgb = { 0, 0, 0 } },
    },
    arrow = {
      idle = { lr = { true, false, 3, 11 }, alpha = 0 },
      focus = { lr = { true, false, -20, -12 }, alpha = 1 },
    },
    glow2 = {
      idle = { alpha = 0 },
      focus = { alpha = 1 },
    },
  }
end

local function applyState(element, state, animate)
  if animate then
    element:beginAnimation("keyframe", FOCUS_TWEEN_MS, false, false, CoD.TweenType.Linear)
  else
    element:completeAnimation()
  end
  if state.lr then
    element:setLeftRight(unpack(state.lr))
  end
  if state.tb then
    element:setTopBottom(unpack(state.tb))
  end
  if state.rgb then
    element:setRGB(unpack(state.rgb))
  end
  if state.alpha then
    element:setAlpha(state.alpha)
  end
  if state.zoom then
    element:setZoom(state.zoom)
  end
end

-- The stock button cross-fades two copies of the label, which looks bad here, so
-- this is one element that changes colour
local function createText(label)
  local text = LUI.UIText.new()
  text:setLeftRight(true, false, TEXT_LEFT, TEXT_RIGHT)
  text:setTopBottom(true, false, 6.92, 26.92)
  text:setTTF("fonts/escom.ttf")
  text:setLetterSpacing(1)
  text:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  text:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
  text:setText(label)
  return text
end

local function createImage(image, material)
  local element = LUI.UIImage.new()
  if image then
    element:setImage(RegisterImage(image))
  end
  element:setMaterial(LUI.UIImage.GetCachedMaterial(material))
  return element
end

-- Rebuilt from the stock FE_List1ButtonLarge_PH, which is not available in-game
local function createButton(menu, controller)
  local button = LUI.UIElement.new()
  button:setLeftRight(true, false, BUTTON_LEFT, BUTTON_LEFT + BUTTON_WIDTH_MAX)
  button:setTopBottom(false, true, -(BUTTON_BOTTOM_OFFSET + BUTTON_HEIGHT), -BUTTON_BOTTOM_OFFSET)
  button:setPriority(1000)
  button:setHandleMouse(true)
  if button.setHandleMouseMove then
    button:setHandleMouseMove(true)
  end
  button:setForceMouseEventDispatch(true)

  -- The stock blurred panel is left out: it needs the menu's scene blur pass and
  -- renders solid black here
  local overlay = LUI.UIImage.new()
  overlay:setLeftRight(true, true, 0, 0)
  overlay:setTopBottom(false, false, -14.2, 14.2)
  overlay:setRGB(0, 0, 0)
  overlay:setMaterial(LUI.UIImage.GetCachedMaterial("uie_feather_edges"))
  overlay:setShaderVector(0, 0.01, 0.03, 0.01, 0.025)
  button:addElement(overlay)

  local idleFrame = createImage("uie_t7_menu_frontend_buttonidlefull", "uie_nineslice_add")
  idleFrame:setLeftRight(true, true, -1, 1)
  idleFrame:setTopBottom(false, false, -16, 16)
  idleFrame:setShaderVector(0, 0.02837, 0.25, 0, 0)
  idleFrame:setupNineSliceShader(8, 8)
  button:addElement(idleFrame)

  local focusFrame = createImage("uie_t7_menu_frontend_buttonfocusfull", "uie_nineslice_add")
  focusFrame:setLeftRight(true, true, -8, 8)
  focusFrame:setShaderVector(0, 0.2027, 0.5, 0.38, 0.2)
  focusFrame:setupNineSliceShader(60, 20)
  button:addElement(focusFrame)

  local glow = createImage("uie_t7_cp_hud_enemytarget_glow", "ui_add")
  glow:setLeftRight(true, true, -40.5, 56.5)
  glow:setTopBottom(true, true, -25.29, 22.12)
  glow:setRGB(0.88, 1, 0.04)
  button:addElement(glow)

  local label = Engine.Localize("CANCEL")
  local text = createText(label)
  button:addElement(text)

  local arrow = createImage("uie_t7_menu_frontend_buttonfocusarrow", "ui_add")
  arrow:setTopBottom(true, false, 12, 20)
  button:addElement(arrow)

  local glow2 = createImage("uie_t7_cp_hud_enemytarget_glow", "ui_add")
  glow2:setLeftRight(true, true, -29, 49)
  glow2:setTopBottom(true, false, -15.29, 46.12)
  button:addElement(glow2)

  local layers = {
    overlay = overlay,
    idleFrame = idleFrame,
    focusFrame = focusFrame,
    glow = glow,
    text = text,
    arrow = arrow,
    glow2 = glow2,
  }
  local states = layerStates()
  local function setLayers(which, animate)
    for name, element in pairs(layers) do
      applyState(element, states[name][which], animate)
    end
  end
  setLayers("idle", false)

  local function fitToLabel()
    local ok, width = pcall(text.getTextWidthAndHeight, text, BUTTON_WIDTH_MAX)
    if not ok or type(width) ~= "number" or width <= 0 then
      return
    end
    local fitted = math.min(width + TEXT_LEFT * 2, BUTTON_WIDTH_MAX)
    button:setLeftRight(true, false, BUTTON_LEFT, BUTTON_LEFT + fitted)
  end
  fitToLabel()

  -- The loading menu has no focus handling, so hover follows the mouse
  local hovered = false
  local pending = false
  local function setHover(on)
    if pending or on == hovered then
      return
    end
    hovered = on
    setLayers(on and "focus" or "idle", true)
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
  function button.requestLeave()
    if pending or not requestLeave(controller) then
      return
    end
    setHover(false)
    pending = true
    text:setText(Engine.Localize("LEAVING..."))
    fitToLabel()
    button:setAlpha(0.6)
  end

  button:registerEventHandler("leftmousedown", function(element)
    element.requestLeave()
    return true
  end)

  return button
end

local function addCancelButton(menu, controller)
  -- Hosts are loading their own match
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

    if #found == 0 then
      setUiActive(controller, false)
    end
  end)
end

-- A run with a request pending is the first snapshot run, so loading is done
if __boiiiLoadingCancelPending ~= nil then
  leaveNow(__boiiiLoadingCancelPending)
else
  update()
end
