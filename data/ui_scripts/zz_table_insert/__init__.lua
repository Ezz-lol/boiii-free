-- Make table.insert tolerate a nil value instead of blocking the round.
--
-- WHY
-- BOIII's table.insert is native and RAISES "inserted value must be specified"
-- when the value is nil. With three local players the stock Zombies HUD path
-- hits exactly that, and because a raised LUI error paints a full-screen ERROR
-- overlay the round becomes unplayable even though the game itself is fine
-- (measured 2026-08-28: 106 FPS still running behind the overlay).
--
-- Observed as UI Error 78115 / 12167 / 9560 - the id changes, the traceback
-- does not: [C]: in function 'table.insert' under HUD_FirstSnapshot_Zombie.
-- It is NOT caused by this project's scripts: it still fires with our
-- zombie_toast wrapper removed and no line of ours in the traceback.
--
-- WHY HERE AND NOT AT THE CALL SITE
-- The failing call is inside stripped stock LUI code, so there is no call site
-- to guard. It also cannot be wrapped: BOIII's LUI sandbox does not expose
-- pcall - trying that raised "Attempt to call a nil value" and killed the
-- lobby (UI Error 69721, 2026-08-25). Overriding the global is the mechanism
-- BOIII itself uses for this class of problem (see ui_scripts/lua_fixes).
--
-- WHAT IT CHANGES
-- Only the nil case, which today is a hard error, becomes a no-op. Every call
-- with a real value is forwarded to the original function untouched, so
-- ordering, positional inserts and return values are unaffected.
--
-- SAFETY
-- Nothing here calls a function that might be absent. `select` is probed once
-- at load and a positional fallback is installed if it is missing, because
-- assuming a base function exists in this sandbox is exactly what broke the
-- lobby last time.

if table ~= nil and table.insert ~= nil then
  local rawinsert = table.insert

  if select ~= nil then
    -- Preferred form: select('#', ...) distinguishes table.insert(t, v)
    -- from table.insert(t, pos, v), including an explicit nil value.
    table.insert = function(t, ...)
      if t == nil then
        return
      end
      local n = select("#", ...)
      if n == 0 then
        return
      end
      if n == 1 then
        local v = ...
        if v == nil then
          return
        end
        return rawinsert(t, v)
      end
      local pos, v = ...
      if v == nil then
        return
      end
      return rawinsert(t, pos, v)
    end
  else
    -- Fallback when select is not exposed. A 3-arg call whose value is
    -- explicitly nil cannot be told apart from a 2-arg call here, so it is
    -- dropped rather than guessed at - dropping matches what the nil case
    -- does anyway, and the alternative would insert the POSITION as a value.
    table.insert = function(t, a, b)
      if t == nil then
        return
      end
      if b ~= nil then
        return rawinsert(t, a, b)
      end
      if a == nil then
        return
      end
      return rawinsert(t, a)
    end
  end
end
