if CoD.isFrontend or Engine.CurrentSessionMode() ~= Enum.eModes.MODE_ZOMBIES then
  return
end

-- Revelations and Gorod Krovi send the match start time in a 20 bit clientfield,
-- which caps it at this value once a server has been up for about 17 minutes
local CAPPED_START = 1048575

for controller = 0, Engine.GetMaxLocalControllers() - 1 do
  local root = LUI.roots["UIRoot" .. controller]
  if root then
    local model = Engine.CreateModel(Engine.GetModelForController(controller), "hudItems.time.game_start_time")
    local joined = true
    root:subscribeToModel(model, function()
      -- A capped value arriving now means the match started now
      if not joined and Engine.GetModelValue(model) == CAPPED_START then
        Engine.SetModelValue(model, Engine.CurrentGameTime())
      end
      joined = false
    end)
  end
end
