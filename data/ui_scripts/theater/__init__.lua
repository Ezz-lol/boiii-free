if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

local films, clips = {}, {}

local function refresh()
  CoD.FileshareUtility.SetFileshareReady(1)
  films, clips = {}, {}
  for _, film in ipairs(game.getfilms()) do
    table.insert(film.clip and clips or films, film)
  end
end

local function slotData(film, category)
  if not film then
    return { isValid = false }
  end
  return {
    fileId = Engine.StringToXUIDDecimal(film.id),
    fileSize = Engine.StringToXUIDDecimal(film.size),
    fileSummarySize = film.summarySize,
    fileCategory = category,
    isValid = true,
    isPooled = false,
    isLiveStream = false,
    mainMode = film.mode,
    mapName = film.map,
    gameTypeName = film.gametype,
    gameTypeImage = Engine.StructTableLookupString(CoDShared.gametypesStructTable, "name", film.gametype, "image"),
    duration = film.duration,
    fileCreateTime = film.created,
    fileAuthorName = film.author,
    fileName = "",
    fileDescription = "",
  }
end

local function isClipCategory(category)
  return category == "clip" or category == "clip_private"
end

local getTotalUsedCommunitySlotCount = Engine.FileshareGetTotalUsedCommunitySlotCount
Engine.FileshareGetTotalUsedCommunitySlotCount = function(controller, category, ...)
  if category ~= "film" then
    return getTotalUsedCommunitySlotCount(controller, category, ...)
  end
  refresh()
  return #films
end

local getCommunitySlotData = Engine.FileshareGetCommunitySlotData
Engine.FileshareGetCommunitySlotData = function(controller, category, filter, filterValue, index)
  if category ~= "film" then
    return getCommunitySlotData(controller, category, filter, filterValue, index)
  end
  return slotData(films[index + 1], "film")
end

local getSlotCount = Engine.FileshareGetSlotCount
Engine.FileshareGetSlotCount = function(controller, category, ...)
  if not isClipCategory(category) then
    return getSlotCount(controller, category, ...)
  end
  refresh()
  return #clips
end

local loadTheaterFile = CoD.FileshareUtility.LoadTheaterFile
CoD.FileshareUtility.LoadTheaterFile = function(...)
  local root = Engine.GetGlobalModel()
  Engine.SetModelValue(Engine.CreateModel(root, "lobbyRoot.theaterDownloadPercent"), 0)
  Engine.SetModelValue(Engine.CreateModel(root, "lobbyRoot.theaterDataDownloaded"), 0)
  return loadTheaterFile(...)
end

local function autoRecordLabel()
  if Engine.DvarBool(nil, "theater_autoRecord") then
    return "^2AUTO RECORD: ON"
  end
  return "^1AUTO RECORD: OFF"
end

local addButtonsForTarget = CoD.LobbyMenus.AddButtonsForTarget
CoD.LobbyMenus.AddButtonsForTarget = function(controller, id)
  local buttons = addButtonsForTarget(controller, id)
  if
    buttons
    and (id == LobbyData.UITargets.UI_MPLOBBYONLINETHEATER.id or id == LobbyData.UITargets.UI_ZMLOBBYONLINETHEATER.id)
  then
    if buttons[#buttons] then
      buttons[#buttons].isLastButtonInGroup = true
    end
    table.insert(buttons, {
      optionDisplay = autoRecordLabel(),
      customId = "btnTheaterAutoRecord",
      isLargeButton = false,
      isLastButtonInGroup = false,
      disabled = false,
      selected = false,
      action = function(self, element, controller)
        Engine.SetDvar("theater_autoRecord", Engine.DvarBool(nil, "theater_autoRecord") and 0 or 1)
        Engine.SetModelValue(Engine.GetModel(element:getModel(), "displayText"), autoRecordLabel())
      end,
    })
  end
  return buttons
end

local getSlotData = Engine.FileshareGetSlotData
Engine.FileshareGetSlotData = function(controller, category, filter, filterValue, index)
  if not isClipCategory(category) then
    return getSlotData(controller, category, filter, filterValue, index)
  end
  return slotData(clips[index + 1], "clip")
end
