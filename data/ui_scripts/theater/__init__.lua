if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

-- Recent games lists the films recorded on this machine instead of fileshare.
local films = {}

local getTotalUsedCommunitySlotCount = Engine.FileshareGetTotalUsedCommunitySlotCount
Engine.FileshareGetTotalUsedCommunitySlotCount = function(controller, category, ...)
  if category ~= "film" then
    return getTotalUsedCommunitySlotCount(controller, category, ...)
  end
  films = game.getfilms()
  return #films
end

local getCommunitySlotData = Engine.FileshareGetCommunitySlotData
Engine.FileshareGetCommunitySlotData = function(controller, category, filter, filterValue, index)
  if category ~= "film" then
    return getCommunitySlotData(controller, category, filter, filterValue, index)
  end
  local film = films[index + 1]
  if not film then
    return { isValid = false }
  end
  return {
    fileId = Engine.StringToXUIDDecimal(film.id),
    fileSize = Engine.StringToXUIDDecimal(film.size),
    fileCategory = "film",
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
