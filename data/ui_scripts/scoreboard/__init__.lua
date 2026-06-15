pcall(function()
  local _rowFactoryHookInstalled = false
  local _scoreboardRows = {}
  local _scoreboardRowsFound = false
  local _pendingRows = {}

  local _origGetClientName = GetClientName
  local _origGetClientClantag = GetClientClantag

  local function getOverrideName(clientNum)
    if not game or not game.getclientoverridename then
      return ""
    end

    local ok, value = pcall(game.getclientoverridename, clientNum)
    if ok and value then
      return value
    end

    return ""
  end

  local function getOverrideTag(clientNum)
    if not game or not game.getclientoverridetag then
      return ""
    end

    local ok, value = pcall(game.getclientoverridetag, clientNum)
    if ok and value then
      return value
    end

    return ""
  end

  local function collectRoots()
    local roots = {}

    if LUI and LUI.roots then
      if LUI.roots.UIRoot0 then
        table.insert(roots, { node = LUI.roots.UIRoot0 })
      end
      if LUI.roots.UIRoot1 then
        table.insert(roots, { node = LUI.roots.UIRoot1 })
      end
      if LUI.roots.UIRootFull then
        table.insert(roots, { node = LUI.roots.UIRootFull })
      end
    end

    return roots
  end

  local function resolveTimerRoot()
    local roots = collectRoots()

    for _, rootInfo in ipairs(roots) do
      if rootInfo.node == LUI.roots.UIRootFull then
        return rootInfo.node
      end
    end

    if roots[1] then
      return roots[1].node
    end

    return nil
  end

  local function stripBrackets(s)
    if not s or s == "" then
      return ""
    end

    return s:gsub("^%[", ""):gsub("%]$", "")
  end

  local function decodeColors(s)
    if not s then
      return ""
    end

    return s:gsub("`(%d)", "^%1")
  end

  local function splitPacked(rawName)
    local pipe = rawName:find("|", 1, true)
    if pipe then
      return rawName:sub(1, pipe - 1), rawName:sub(pipe + 1)
    end

    return rawName, nil
  end

  local function buildDisplayText(ctrl, clientNum)
    local rawName = _origGetClientName(ctrl, clientNum) or ""
    local rawTag = _origGetClientClantag(ctrl, clientNum) or ""
    local namePart, tagPart = splitPacked(rawName)
    local overrideName = getOverrideName(clientNum)
    local overrideTag = getOverrideTag(clientNum)

    if overrideName ~= "" then
      namePart = overrideName
    end

    if overrideTag ~= "" then
      tagPart = overrideTag
    end

    local displayText = ""
    if tagPart and tagPart ~= "" then
      displayText = "^7[" .. decodeColors(stripBrackets(tagPart)) .. "^7]"
    else
      local tag = stripBrackets(rawTag)
      if tag ~= "" then
        displayText = "[" .. tag .. "]"
      end
    end

    displayText = displayText .. decodeColors(namePart)
    return displayText
  end

  GetClientNameAndClanTag = function(ctrl, clientNum)
    return buildDisplayText(ctrl, clientNum)
  end

  local function ensureRowFactoryHook()
    if _rowFactoryHookInstalled then
      return true
    end

    if not CoD or not CoD.ScoreboardRowWidget or type(CoD.ScoreboardRowWidget.new) ~= "function" then
      return false
    end

    local origRowNew = CoD.ScoreboardRowWidget.new
    CoD.ScoreboardRowWidget.new = function(menu, controller)
      local row = origRowNew(menu, controller)
      if row and row.Gamertag then
        table.insert(_pendingRows, row)
      end
      return row
    end

    _rowFactoryHookInstalled = true
    return true
  end

  local function findRows(element, results, depth)
    if depth > 30 or not element then
      return
    end

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
          if ok3 and child then
            findRows(child, results, depth + 1)
          end
        end
      end
    end
  end

  ensureRowFactoryHook()

  local root = resolveTimerRoot()
  if not root then
    return
  end

  if root._nameRefreshTimer then
    root:removeElement(root._nameRefreshTimer)
  end

  local timer = LUI.UITimer.newElementTimer(200, false, function()
    pcall(function()
      if not Engine.IsInGame() then
        return
      end

      ensureRowFactoryHook()

      if not _scoreboardRowsFound then
        for _, rootInfo in ipairs(collectRoots()) do
          findRows(rootInfo.node, _scoreboardRows, 0)
        end

        if #_scoreboardRows > 0 then
          _scoreboardRowsFound = true
        end
      end

      if #_pendingRows > 0 then
        for _, row in ipairs(_pendingRows) do
          table.insert(_scoreboardRows, row)
        end
        _pendingRows = {}
        _scoreboardRowsFound = true
      end

      if #_scoreboardRows == 0 then
        return
      end

      for _, row in ipairs(_scoreboardRows) do
        pcall(function()
          if not row or not row.Gamertag then
            return
          end

          local model = row:getModel()
          if not model then
            return
          end

          local clientNumModel = Engine.GetModel(model, "clientNum")
          if not clientNumModel then
            return
          end

          local clientNum = Engine.GetModelValue(clientNumModel)
          if clientNum == nil or clientNum < 0 then
            return
          end

          row.Gamertag:setText(buildDisplayText(0, clientNum))
          row.Gamertag:setRGB(1, 1, 1)
        end)
      end
    end)
  end)

  root:addElement(timer)
  root._nameRefreshTimer = timer
end)
