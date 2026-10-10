local getClientName = GetClientName
local getClientClantag = GetClientClantag

local function decodeColors(text)
  return (text:gsub("`(%d)", "^%1"))
end

local function stripBrackets(text)
  return (text:gsub("^%[", ""):gsub("%]$", ""))
end

GetClientNameAndClanTag = function(controller, clientNum)
  local name = getClientName(controller, clientNum) or ""
  local tag
  local pipe = name:find("|", 1, true)
  if pipe then
    name, tag = name:sub(1, pipe - 1), name:sub(pipe + 1)
  end

  local overrideName = game.getclientoverridename(clientNum)
  local overrideTag = game.getclientoverridetag(clientNum)
  if overrideName ~= "" then
    name = overrideName
  end
  if overrideTag ~= "" then
    tag = overrideTag
  end

  if tag and tag ~= "" then
    return "^7[" .. decodeColors(stripBrackets(tag)) .. "^7]" .. decodeColors(name)
  end

  local clantag = stripBrackets(getClientClantag(controller, clientNum) or "")
  return (clantag ~= "" and "[" .. clantag .. "]" or "") .. decodeColors(name)
end

pcall(require, "ui.uieditor.widgets.Scoreboard.ScoreboardRowWidget")
if not CoD.ScoreboardRowWidget or CoD.ScoreboardRowWidget.boiiiNames then
  return
end

local newRow = CoD.ScoreboardRowWidget.new
CoD.ScoreboardRowWidget.boiiiNames = true
CoD.ScoreboardRowWidget.new = function(menu, controller)
  local row = newRow(menu, controller)
  row:subscribeToModel(Engine.CreateModel(Engine.GetGlobalModel(), "boiiiNames.update"), function()
    local model = row:getModel(controller, "clientNum")
    local clientNum = model and Engine.GetModelValue(model)
    if clientNum and clientNum >= 0 then
      row.Gamertag:setText(GetClientNameAndClanTag(controller, clientNum))
    end
  end, false)
  return row
end
