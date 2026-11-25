local root = arg and arg[1] or "."
local source = assert(loadfile(root .. "/data/coop_scripts/score_models.lua"))
local events = { "damage", "death_normal", "death_torso", "death_neck", "death_head", "death_melee" }
local function fixture(slots, firstExtra, mutate, raises)
  local records, diagnostics = {}, {}
  CoD = { isFrontend = false, isCampaign = false, isZombie = true }
  Enum = { UIModelClientFieldType = { CF_COUNTER = 3 } }
  Engine = {
    DvarInt = function() return slots end,
    RegisterClientUIModelField = function(name, kind, bits, version, ...)
      assert(not records[name], "duplicate field: " .. name)
      records[name] = { kind, bits, version, n = select("#", ...), ... }
      return "registered", nil, 9
    end,
    SetDvar = function(name, value) diagnostics[name] = value end,
  }
  local originalRegister = Engine.RegisterClientUIModelField
  local calls = 0
  local wrapped = source()(function(a, b, tail)
    calls = calls + 1; assert(a == "arg" and b == false and tail == nil)
    if raises then error("original failed") end
    for slot = 0, (firstExtra or 3) do
      for _, event in ipairs(events) do
        local width = event == "damage" and 3 or 2
        if mutate and slot == 2 and event == "damage" then width = 4 end
        assert(select("#", Engine.RegisterClientUIModelField("PlayerList.client" .. slot .. ".score_cf_" .. event,
          3, width, 1, "extra", nil)) == 3)
      end
    end
    return 11, false, nil, "tail"
  end)
  local function invoke()
    assert(select("#", wrapped("arg", false, nil)) == 4)
    local a, b, c, d = wrapped("arg", false, nil) -- use only for failure fixtures below
    return a, b, c, d
  end
  return wrapped, records, diagnostics, originalRegister, function() return calls end
end
for limit = 5, 8 do
  for _, present in ipairs({3, 4, 7}) do
    local wrapped, records, diagnostics, register, calls = fixture(limit, present)
    local values = {n = select("#", wrapped("arg", false, nil))}
    assert(values.n == 4 and calls() == 1)
    local count = 0; for _ in pairs(records) do count = count + 1 end
    assert(count == 48 and diagnostics.zm_coopSchemaUIRevision == 1)
    assert(Engine.RegisterClientUIModelField == register)
    assert(records["PlayerList.client0.score_cf_damage"].n == 2)
    assert(records["PlayerList.client7.score_cf_death_melee"][2] == 2)
  end
end
for _, limit in ipairs({0, 1, 4, 9, 18, 5.5}) do
  local wrapped, records, diagnostics, register, calls = fixture(limit)
  local a, b, c, d = wrapped("arg", false, nil)
  assert(a == 11 and b == false and c == nil and d == "tail" and calls() == 1)
  assert(records["PlayerList.client4.score_cf_damage"] == nil and next(diagnostics) == nil)
  assert(Engine.RegisterClientUIModelField == register)
end
for _, mode in ipairs({"isFrontend", "isCampaign"}) do
  local wrapped, records, diagnostics = fixture(8)
  CoD[mode] = true
  assert(wrapped("arg", false, nil) == 11)
  assert(records["PlayerList.client4.score_cf_damage"] == nil and next(diagnostics) == nil)
end
for _, failure in ipairs({"signature", "stock"}) do
  local wrapped, records, diagnostics, register = fixture(8, 3, failure == "signature", failure == "stock")
  assert(not pcall(wrapped, "arg", false, nil))
  assert(Engine.RegisterClientUIModelField == register and next(diagnostics) == nil)
  assert(records["PlayerList.client4.score_cf_damage"] == nil)
end
print("PASS: early score models, fixed 48-field contract, existing extras, modes, original errors and registration ownership")
