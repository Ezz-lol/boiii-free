local _ok, _err = pcall(function()
  _nameRefreshInstalled = nil

  local _origGetClientName = GetClientName
  local _origGetClientClantag = GetClientClantag

  local function stripBrackets(s)
    if not s or s == "" then return "" end
    return s:gsub("^%[", ""):gsub("%]$", "")
  end

  local function decodeColors(s)
    return s:gsub("`(%d)", "^%1")
  end

  local function splitPacked(rawName)
    local pipe = rawName:find("|", 1, true)
    if pipe then
      return rawName:sub(1, pipe - 1), rawName:sub(pipe + 1)
    end
    return rawName, nil
  end

  GetClientNameAndClanTag = function(ctrl, clientNum)
    local rawName = _origGetClientName(ctrl, clientNum) or ""
    local namePart, tagPart = splitPacked(rawName)
    local coloredName = decodeColors(namePart)

    local result = ""
    if tagPart and tagPart ~= "" then
      result = "^7[" .. decodeColors(tagPart) .. "^7]"
    else
      local rawTag = _origGetClientClantag(ctrl, clientNum) or ""
      local tag = stripBrackets(rawTag)
      if tag ~= "" then
        result = "[" .. tag .. "]"
      end
    end
    return result .. coloredName
  end

  local origRowNew = CoD.ScoreboardRowWidget.new
  CoD.ScoreboardRowWidget.new = function(menu, controller)
    local row = origRowNew(menu, controller)
    if row and row.Gamertag then
      _scoreboardRowsNew = _scoreboardRowsNew or {}
      table.insert(_scoreboardRowsNew, row)
    end
    return row
  end

  local root = LUI.roots.UIRootFull
  if not root then return end

  if root._nameRefreshTimer then
    root:removeElement(root._nameRefreshTimer)
  end

  _sbRowsFound = false
  _sbRows = {}

  local function findRows(element, results, depth)
    if depth > 30 then return end
    if not element then return end
    pcall(function()
      if element.id == "ScoreboardRowWidget" and element.Gamertag then
        table.insert(results, element)
      end
    end)
    if element.getNumChildren then
      local ok2, n = pcall(element.getNumChildren, element)
      if ok2 and n and n > 0 then
        for i = 0, n - 1 do
          local ok3, child = pcall(element.getChildAt, element, i)
          if ok3 and child then findRows(child, results, depth + 1) end
        end
      end
    end
  end

  local timer = LUI.UITimer.newElementTimer(200, false, function()
    pcall(function()
      if not Engine.IsInGame() then return end

      if not _sbRowsFound then
        findRows(root, _sbRows, 0)
        if #_sbRows > 0 then _sbRowsFound = true end
      end

      if _scoreboardRowsNew then
        for _, r in ipairs(_scoreboardRowsNew) do
          table.insert(_sbRows, r)
        end
        _scoreboardRowsNew = nil
        _sbRowsFound = true
      end

      if not _sbRows or #_sbRows == 0 then return end

      for _, row in ipairs(_sbRows) do
        pcall(function()
          if not row or not row.Gamertag then return end
          local m = row:getModel()
          if not m then return end
          local cnModel = Engine.GetModel(m, "clientNum")
          if not cnModel then return end
          local cn = Engine.GetModelValue(cnModel)
          if cn == nil then return end

          local rawName = _origGetClientName(0, cn) or ""
          local namePart, tagPart = splitPacked(rawName)
          local coloredName = decodeColors(namePart)

          local displayText = ""
          if tagPart and tagPart ~= "" then
            displayText = "^7[" .. decodeColors(tagPart) .. "^7]"
          else
            local rawTag = _origGetClientClantag(0, cn) or ""
            local tag = stripBrackets(rawTag)
            if tag ~= "" then
              displayText = "[" .. tag .. "]"
            end
          end
          displayText = displayText .. coloredName

          row.Gamertag:setText(displayText)
          row.Gamertag:setRGB(1, 1, 1)
        end)
      end
    end)
  end)
  root:addElement(timer)
  root._nameRefreshTimer = timer
end)