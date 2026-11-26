-- SPDX-License-Identifier: GPL-3.0-only
-- Constructor used around the actual clientfieldmodels chunk before execution.
-- Existing registrations and returns retain their order and ownership.
local function pack(...) return { n = select("#", ...), ... } end
local widths = { damage = 3, death_normal = 2, death_torso = 2,
  death_neck = 2, death_head = 2, death_melee = 2 }
local events = { "damage", "death_normal", "death_torso", "death_neck", "death_head", "death_melee" }

return function(original)
  assert(type(original) == "function", "Missing stock clientfieldmodels chunk")
  return function(...)
    local slots = Engine.DvarInt(nil, "zm_coopSchemaSlots")
    if type(slots) ~= "number" or slots ~= math.floor(slots) or slots < 5 or slots > 8 then
      return original(...)
    end
    local register = Engine.RegisterClientUIModelField
    assert(type(register) == "function", "Missing client UI model registration API")
    local seen, conflict = {}, nil
    local observe = function(name, kind, bits, version, ...)
      if type(name) == "string" then
        local slot, event = name:match("^PlayerList%.client([0-7])%.score_cf_([a-z_]+)$")
        if slot and widths[event] then
          if kind ~= Enum.UIModelClientFieldType.CF_COUNTER or bits ~= widths[event] or version ~= 1 then
            conflict = name
          end
          if seen[name] then conflict = name end
          seen[name] = true
        end
      end
      return register(name, kind, bits, version, ...)
    end
    Engine.RegisterClientUIModelField = observe
    local result = pack(pcall(original, ...))
    local owns = Engine.RegisterClientUIModelField == observe
    if owns then Engine.RegisterClientUIModelField = register end
    if not result[1] then error(result[2], 0) end
    if CoD.isFrontend or CoD.isCampaign or not CoD.isZombie then
      return unpack(result, 2, result.n)
    end
    assert(owns, "Client UI registration changed while the schema was loading")
    assert(not conflict, "Incompatible score clientfield contract: " .. tostring(conflict))
    -- Stock slots must exist before extending their exact known contract.
    -- An unrelated HUD/mod remains an explicit unsupported contract.
    for slot = 0, 3 do
      for _, event in ipairs(events) do
        assert(seen["PlayerList.client" .. slot .. ".score_cf_" .. event],
          "Missing stock score clientfield for slot " .. slot)
      end
    end
    -- A compatible mod may already register some extra fields. Keep those;
    -- create only absent entries after validating every observed signature.
    for slot = 4, 7 do
      for _, event in ipairs(events) do
        local name = "PlayerList.client" .. slot .. ".score_cf_" .. event
        if not seen[name] then
          register(name, Enum.UIModelClientFieldType.CF_COUNTER, widths[event], 1)
        end
      end
    end
    -- Diagnostics only. Native checksums remain authoritative for compatibility.
    Engine.SetDvar("zm_coopSchemaUIRevision", 3)
    return unpack(result, 2, result.n)
  end
end
