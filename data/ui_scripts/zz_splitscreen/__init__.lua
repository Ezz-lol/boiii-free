-- Console-style local splitscreen on PC, using the game's OWN console code
-- (2026-09-27).
--
-- The stock Lua is the same on PS4 and PC (T7LuaRepo ship dumps) and holds the
-- console behaviour; the PC build only switches it off:
--   * CoDMenu.lua CoD.Menu.HandleButtonPress: an unused controller's button becomes
--     "unused_gamepad_button" -> Lobby.lua -> LobbyAddLocalClient(menu, c) ->
--     Engine.SigninLocalClient(c)  (press A to join) - but only
--     `elseif not CoD.isPC then`; on PC the press is dropped (measured: controller
--     1's A reached UIRootFull and nothing happened).
--   * B on a non-primary controller -> LobbyRemoveLocalClientFromLobby(c)
--     (Lobby.lua), wired through CoDMenu's button-model subscriptions for
--     controllers 0..GetMaxLocalControllers()-1.
--   * Offline room: Lobby_SetMaxLocalPlayers(4) capped by GetMaxLocalControllers.
-- The component sets GetMaxControllerCount / GetMaxLocalControllers to 3 (the
-- controllers that have seats; PS4 4). This script takes the console branch of
-- CoD.Menu.HandleButtonPress on PC as well - the stock body, unchanged otherwise.
--
-- The PC-only button (LobbySplitscreenToggle, FE_ListAdditonal) only ever adds
-- or removes CONTROLLER 1. Its label (SplitscreenLobbyButtonPC) reads ACTIVATE
-- while there is room (used controllers < lobby_maxLocalPlayers) and DEACTIVATE
-- once the lobby is full. The override makes the click do what the label says:
-- ACTIVATE adds the next unused controller through the stock join, DEACTIVATE
-- removes every extra local player.
--
-- BOIII's LUI sandbox has no pcall: every global is checked for nil first.
-- The overrides are (re)installed from a timer, so load order does not matter.

if rawget(_G, "__zz_splitscreen_loaded") then
  return
end
rawset(_G, "__zz_splitscreen_loaded", true)

if Engine == nil or Dvar == nil or LUI == nil or LUI.roots == nil or LUI.UIElement == nil or LUI.UITimer == nil then
  return
end

local SUPPORTED = 4 -- local players the component can seat (seat records 0..3)

-- The log is for DEVELOPMENT ONLY. In a player's BOIII (started without
-- -unsafe-lua) io and os are NOT nil: every one of these functions is a stub
-- that pops BOIII's "Unsafe lua function called" warning
-- (ui_scripting.cpp patch_unsafe_lua_functions). So they must not even be
-- called unless the development-only zz_probe script, which is never shipped,
-- has loaded (it sets __zz_probe_loaded first and sorts before this file).
local LOG = nil
if
  rawget(_G, "__zz_probe_loaded")
  and io ~= nil
  and io.open ~= nil
  and os ~= nil
  and os.getenv ~= nil
  and os.getenv("LOCALAPPDATA") ~= nil
then
  LOG = os.getenv("LOCALAPPDATA") .. "\\boiii\\splitscreen_lua.txt"
end
local function log(line)
  if LOG == nil then
    return
  end
  local f = io.open(LOG, "a")
  if f then
    f:write(line .. "\n")
    f:close()
  end
end

local wrapped_toggle = nil
local wrapped_add = nil
local wrapped_handle_press = nil

-- Lobby.lua's PostLoadFunc runs `if CoD.isPC then f0_local0(menu) end`, and the
-- PC f0_local0 registers `unused_gamepad_button -> return true` on the Lobby
-- menu, replacing the console handler created a few lines earlier (Lobby.lua
-- line 1770). The PS4 f0_local0 has no such line. Put the stock console handler
-- back after the menu is built - its body, verbatim.
local function install_lobby_join_handler(menu, controller)
  if menu == nil or menu.registerEventHandler == nil or menu.__zz_console_join then
    return
  end
  menu.__zz_console_join = true
  log("Lobby: console join handler restored")
  menu:registerEventHandler("unused_gamepad_button", function(self, event)
    local handled = nil
    log("Lobby unused_gamepad_button c" .. tostring(event.controller) .. " -> LobbyAddLocalClient")
    LobbyAddLocalClient(self, event.controller or controller)
    if not handled then
      handled = self:dispatchEventToChildren(event)
    end
    return handled
  end)
end

-- The stock CoD.Menu.HandleButtonPress (CoDMenu.lua), with the console branch
-- taken on PC too. UI-editor menus such as Menu.Lobby swallow "gamepad_button"
-- (CoDMenu.lua registers `return true` on the instance) and read buttons through
-- the per-controller ButtonBits models instead (subscribed for controllers
-- 0..GetMaxLocalControllers()-1 on anyControllerAllowed menus); an unused
-- controller's press lands here and becomes "unused_gamepad_button" ->
-- Lobby.lua -> LobbyAddLocalClient(menu, c) - on console. Measured on PC
-- (2026-09-27): controller 1's A reached UIRootFull, Menu.Lobby (owner 0,
-- anyControllerAllowed, own gamepad_button handler) and nothing followed.
local function console_handle_button_press(menu, controller, button, model)
  if Engine.IsControllerBeingUsed(controller) or menu.unusedControllerAllowed then
    local list = menu:GetElementAndFunctionTableForButton(button, "buttonFunctions")
    for _, entry in ipairs(list) do
      if entry.fn(entry.element, menu, controller, model) then
        Engine.SetModelValue(model, 0)
        break
      end
    end
    if #list > 0 then
      Engine.SetModelValue(model, 0)
    end
  else
    if IsGameTypeDOA ~= nil and IsGameTypeDOA() and Engine.IsSplitscreen() then
      menu:setOwner(controller)
    end
    log("unused_gamepad_button c" .. tostring(controller) .. " on " .. tostring(menu.menuName))
    if menu.menuName == "Lobby" then
      -- Lazily, on the menu that is actually open: timers do not tick
      -- reliably in the frontend, so no load-time hook is relied upon.
      install_lobby_join_handler(menu, menu.m_ownerController)
    end
    menu:processEvent({
      name = "unused_gamepad_button",
      controller = controller,
    })
  end
end

local function room_left()
  if Dvar.lobby_maxLocalPlayers == nil then
    return false
  end
  return Engine.GetUsedControllerCount() < Dvar.lobby_maxLocalPlayers:get()
end

local function install()
  if
    CoD ~= nil
    and CoD.Menu ~= nil
    and CoD.Menu.HandleButtonPress ~= nil
    and CoD.Menu.HandleButtonPress ~= wrapped_handle_press
  then
    wrapped_handle_press = console_handle_button_press
    CoD.Menu.HandleButtonPress = wrapped_handle_press
    log("CoD.Menu.HandleButtonPress: console branch installed")
  end

  if LobbySplitscreenToggle ~= nil and LobbySplitscreenToggle ~= wrapped_toggle then
    local stock_toggle = LobbySplitscreenToggle
    wrapped_toggle = function(menu, controller)
      if LuaUtils ~= nil and LuaUtils.LobbyProcessQueueEmpty ~= nil and not LuaUtils.LobbyProcessQueueEmpty() then
        return
      end
      if not room_left() then
        -- DEACTIVATE: every extra local player leaves, in the engine's own
        -- order (LobbyRemoveAllLocalSplitscreenClient walks 1..n).
        log("toggle: deactivate all, used=" .. tostring(Engine.GetUsedControllerCount()))
        if LobbyRemoveLocalClientFromLobby ~= nil then
          for c = 1, SUPPORTED - 1 do
            if Engine.IsControllerBeingUsed(c) == true then
              LobbyRemoveLocalClientFromLobby(menu, c)
            end
          end
        end
        return
      end
      if Engine.IsControllerBeingUsed(1) ~= true then
        log("toggle: activate controller 1 (stock)")
        return stock_toggle(menu, controller)
      end
      -- ACTIVATE with player 2 already in: the next unused controller joins
      -- through the stock join (the same call its own A press makes).
      for c = 2, SUPPORTED - 1 do
        if Engine.IsControllerBeingUsed(c) ~= true then
          log("toggle: activate controller " .. c .. " (stock join)")
          if LobbyAddLocalClient ~= nil then
            LobbyAddLocalClient(menu, c)
          end
          return
        end
      end
    end
    LobbySplitscreenToggle = wrapped_toggle
  end

  if LobbyAddLocalClient ~= nil and LobbyAddLocalClient ~= wrapped_add then
    local stock_add = LobbyAddLocalClient
    wrapped_add = function(menu, controller)
      local max = Dvar.lobby_maxLocalPlayers ~= nil and Dvar.lobby_maxLocalPlayers:get() or -1
      log(
        "LobbyAddLocalClient c"
          .. tostring(controller)
          .. " used="
          .. tostring(Engine.GetUsedControllerCount())
          .. " max="
          .. tostring(max)
      )
      return stock_add(menu, controller)
    end
    LobbyAddLocalClient = wrapped_add
  end
end

local host = LUI.roots.UIRoot0
if host == nil then
  return
end

install()
log("loaded")

local watcher = LUI.UIElement.new()
watcher.id = "zz_splitscreen"
watcher:registerEventHandler("zz_splitscreen_tick", function(self, event)
  install()
  return true
end)
host:addElement(watcher)
watcher:addElement(LUI.UITimer.new(500, "zz_splitscreen_tick", false, watcher))
