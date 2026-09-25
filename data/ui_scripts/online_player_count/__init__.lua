if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

if not __online_count_originals then
  __online_count_originals = {
    GetPlayerGroupCount = Engine.GetPlayerGroupCount,
    FE3dTitleNumberNew = CoD.FE_3dTitleNumber and CoD.FE_3dTitleNumber.new,
  }
end
local original = __online_count_originals

local function classifyServer(info)
  if info.zombies then
    return "zm"
  end
  if info.campaign and info.campaign == 1 then
    return "cp"
  end
  return "mp"
end

local function computeOnlinePlayerCount(groupKey)
  local wantCategory = nil
  if groupKey == "online/cp" then
    wantCategory = "cp"
  elseif groupKey == "online/mp" then
    wantCategory = "mp"
  elseif groupKey == "online/zm" then
    wantCategory = "zm"
  end

  local total = 0
  local rawCount = game.getrawservercount()
  for i = 0, rawCount - 1 do
    local info = game.getrawserverinfo(i)
    if info and info.name and info.name ~= "" then
      if not wantCategory or classifyServer(info) == wantCategory then
        local humanCount = (info.playerCount or 0) - (info.botCount or 0)
        if humanCount > 0 then
          total = total + humanCount
        end
      end
    end
  end

  return tostring(total)
end

-- boiii has no Demonware-backed global stats service, so the stock
-- GetPlayerGroupCount call has nothing to return. Use the real player counts
-- from our own internet server list instead.
Engine.GetPlayerGroupCount = function(groupKey, ...)
  local ok, result = pcall(computeOnlinePlayerCount, groupKey)
  if ok then
    return result
  end
  if original.GetPlayerGroupCount then
    return original.GetPlayerGroupCount(groupKey, ...)
  end
  return ""
end

-- Keep the server list warm so the number is meaningful without the player
-- having to open the server browser first.
pcall(function()
  Engine.SteamServerBrowser_RequestServers(Enum.SteamServerRequestType.STEAM_SERVER_REQUEST_TYPE_INTERNET)
end)

-- Stock FE_3dTitleNumber hides itself (alpha 0) whenever IsLAN(), IsGameLobby(),
-- or the groupCountsVisible dvar isn't set -- all of which are effectively
-- always true for boiii. Keep it visible instead now that the number behind
-- it is real.
if original.FE3dTitleNumberNew then
  CoD.FE_3dTitleNumber.new = function(menu, controller)
    local self = original.FE3dTitleNumberNew(menu, controller)
    if not self or not self.clipsPerState then
      return self
    end

    local function forceVisibleClip()
      return function()
        self:setupElementClipCounter(2)
        self.FELabelSubHeadingA0:completeAnimation()
        self.FELabelSubHeadingA0:setAlpha(1)
        self.clipFinished(self.FELabelSubHeadingA0, {})
        self.FE3dTitleNumberGames0:completeAnimation()
        self.FE3dTitleNumberGames0:setAlpha(1)
        self.clipFinished(self.FE3dTitleNumberGames0, {})
      end
    end

    for _, stateName in ipairs({ "Hidden", "Hidden2", "Hidden3" }) do
      local state = self.clipsPerState[stateName]
      if state then
        state.DefaultClip = forceVisibleClip()
      end
    end

    return self
  end
end
