if Engine.GetCurrentMap() ~= "core_frontend" then
	return
end

require("ui.uieditor.widgets.PC.ServerBrowser.ServerBrowserRowInternal")

function IsServerBrowserEnabled()
	return true
end

local filters = {
	map = "",
	modName = "",
	gameType = "",
	minPing = 0,
	maxPing = 999,
	notFull = false,
	notEmpty = false,
	hasPassword = "any",
	dedicated = "any",
	hardcore = "any"
}

local currentSort = {field = "ping", desc = false}

-- Server info cache: offset -> serverInfo from engine
local serverCache = {}
-- Filtered + sorted list of offsets into serverCache
local filteredIndices = {}
-- Total server count from engine
local totalServerCount = 0

-- Map native serverListSortType values to sort fields
local nativeSortMapping = {
	[0] = {field = "ping", desc = false},
	[1] = {field = "name", desc = false},
	[2] = {field = "mapName", desc = false},
	[3] = {field = "gameType", desc = false},
	[4] = {field = "playerCount", desc = true},
}

local function applyFilters(serverInfo)
	if not serverInfo then
		return false
	end

	if filters.minPing > 0 and serverInfo.ping < filters.minPing then
		return false
	end

	if filters.maxPing < 999 and serverInfo.ping > filters.maxPing then
		return false
	end

	if filters.modName ~= "" and not string.find(string.lower(serverInfo.modName or ""), string.lower(filters.modName)) then
		return false
	end

	if filters.gameType ~= "" and (serverInfo.gametype or "") ~= filters.gameType then
		return false
	end

	if filters.map ~= "" and not string.find(string.lower(serverInfo.map or ""), string.lower(filters.map)) then
		return false
	end

	if filters.notFull and serverInfo.playerCount >= (serverInfo.maxPlayers or 0) then
		return false
	end

	if filters.notEmpty and serverInfo.playerCount <= 0 then
		return false
	end

	if filters.hasPassword ~= "any" then
		local hasPassword = serverInfo.password
		if (filters.hasPassword == "yes" and not hasPassword) or (filters.hasPassword == "no" and hasPassword) then
			return false
		end
	end

	if filters.dedicated ~= "any" then
		local isDedicated = serverInfo.dedicated
		if (filters.dedicated == "yes" and not isDedicated) or (filters.dedicated == "no" and isDedicated) then
			return false
		end
	end

	if filters.hardcore ~= "any" then
		local isHardcore = serverInfo.hardcore
		if (filters.hardcore == "yes" and not isHardcore) or (filters.hardcore == "no" and isHardcore) then
			return false
		end
	end

	return true
end

local function rebuildFilteredAndSortedList()
	filteredIndices = {}
	for offset, info in pairs(serverCache) do
		if applyFilters(info) then
			filteredIndices[#filteredIndices + 1] = offset
		end
	end

	table.sort(filteredIndices, function(a, b)
		local infoA = serverCache[a]
		local infoB = serverCache[b]
		if not infoA and not infoB then return a < b end
		if not infoA then return false end
		if not infoB then return true end

		local field = currentSort.field
		local valA, valB

		if field == "ping" then
			valA = infoA.ping or 999
			valB = infoB.ping or 999
		elseif field == "name" then
			valA = string.lower(infoA.name or "")
			valB = string.lower(infoB.name or "")
		elseif field == "playerCount" then
			valA = (infoA.playerCount or 0) - (infoA.botCount or 0)
			valB = (infoB.playerCount or 0) - (infoB.botCount or 0)
		elseif field == "mapName" then
			valA = string.lower(infoA.map or "")
			valB = string.lower(infoB.map or "")
		elseif field == "gameType" then
			valA = string.lower(infoA.gametype or "")
			valB = string.lower(infoB.gametype or "")
		else
			valA = infoA.ping or 999
			valB = infoB.ping or 999
		end

		if valA == valB then
			return a < b
		end

		if currentSort.desc then
			return valA > valB
		else
			return valA < valB
		end
	end)
end

local function refreshServerCache(count)
	serverCache = {}
	totalServerCount = count or 0
	for i = 0, totalServerCount - 1 do
		local info = Engine.SteamServerBrowser_GetServerInfo(i)
		if info then
			serverCache[i] = info
		end
	end
	rebuildFilteredAndSortedList()
end

local function clearServerCache()
	serverCache = {}
	filteredIndices = {}
	totalServerCount = 0
end

DataSources.LobbyServer = {
	prepare = function(controller, list, filter)
		list.numElementsInList = list.vCount
		list.controller = controller
		list.serverBrowserRootModel = Engine.CreateModel(Engine.GetGlobalModel(), "serverBrowser")
		local serverListCountModel = Engine.GetModel(list.serverBrowserRootModel, "serverListCount")
		if serverListCountModel then
			list.serverCount = Engine.GetModelValue(serverListCountModel)
		else
			list.serverCount = 0
		end
		list.servers = {}

		local filterModel = Engine.CreateModel(list.serverBrowserRootModel, "filters")
		for key, _ in pairs(filters) do
			Engine.CreateModel(filterModel, key)
		end

		local sortModel = Engine.CreateModel(list.serverBrowserRootModel, "sort")
		Engine.SetModelValue(Engine.CreateModel(sortModel, "field"), currentSort.field)
		Engine.SetModelValue(Engine.CreateModel(sortModel, "desc"), currentSort.desc)

		local serversModel = Engine.CreateModel(list.serverBrowserRootModel, "servers")
		for i = 1, list.numElementsInList, 1 do
			list.servers[i] = {}
			list.servers[i].root = Engine.CreateModel(serversModel, "server_" .. i)
			list.servers[i].model = Engine.CreateModel(list.servers[i].root, "model")
		end
		list.updateModels = function(controller, list, offset)
			local serverInfo = Engine.SteamServerBrowser_GetServerInfo(offset)
			if serverInfo then
				local SetModelValue = function(model, key, value)
					local model = Engine.CreateModel(model, key)
					if model then
						Engine.SetModelValue(model, value)
					end
				end

				local elementIndex = offset % list.numElementsInList + 1
				local serverModel = list.servers[elementIndex].model
				SetModelValue(serverModel, "serverIndex", serverInfo.serverIndex)
				SetModelValue(serverModel, "connectAddr", serverInfo.connectAddr)
				SetModelValue(serverModel, "ping", serverInfo.ping)
				SetModelValue(serverModel, "modName", serverInfo.modName)
				SetModelValue(serverModel, "mapName", serverInfo.map)
				SetModelValue(serverModel, "desc", serverInfo.desc)
				local clientCount = serverInfo.playerCount - serverInfo.botCount
				SetModelValue(serverModel, "clientCount", clientCount)
				SetModelValue(serverModel, "maxClients", serverInfo.maxPlayers)
				SetModelValue(serverModel, "passwordProtected", serverInfo.password)
				SetModelValue(serverModel, "secure", serverInfo.secure)
				SetModelValue(serverModel, "name", serverInfo.name)
				SetModelValue(serverModel, "gameType", serverInfo.gametype)
				SetModelValue(serverModel, "dedicated", serverInfo.dedicated)
				SetModelValue(serverModel, "ranked", serverInfo.ranked)
				SetModelValue(serverModel, "hardcore", serverInfo.hardcore)
				SetModelValue(serverModel, "zombies", serverInfo.zombies)
				SetModelValue(serverModel, "botCount", serverInfo.botCount)
				return serverModel
			else
				return nil
			end
		end

		-- Build initial cache
		refreshServerCache(list.serverCount or 0)

		-- Server count changes: new servers discovered or refresh completed
		-- Re-read all server info from engine and rebuild filtered list
		if list.serverListUpdateSubscription then
			list:removeSubscription(list.serverListUpdateSubscription)
		end
		local serverListUpdateModel = Engine.CreateModel(list.serverBrowserRootModel, "serverListCount")
		list.serverListUpdateSubscription = list:subscribeToModel(serverListUpdateModel, function(model)
			local count = Engine.GetModelValue(model)
			if count then
				list.serverCount = count
			end
			refreshServerCache(list.serverCount or 0)
			list:updateDataSource(false, false)
		end, false)

		-- Sort type changes (column header clicks): re-sort from cache
		if list.serverListSortTypeSubscription then
			list:removeSubscription(list.serverListSortTypeSubscription)
		end
		local serverListSortTypeModel = Engine.CreateModel(list.serverBrowserRootModel, "serverListSortType")
		list.serverListSortTypeSubscription = list:subscribeToModel(serverListSortTypeModel, function(model)
			local sortType = Engine.GetModelValue(model)
			if sortType ~= nil then
				local mapping = nativeSortMapping[sortType]
				if mapping then
					if currentSort.field == mapping.field then
						currentSort.desc = not currentSort.desc
					else
						currentSort.field = mapping.field
						currentSort.desc = mapping.desc
					end
				end
			end
			rebuildFilteredAndSortedList()
			list:updateDataSource(false, false)
		end, false)

		-- Filter changes: re-filter from cache
		if list.filterSubscription then
			list:removeSubscription(list.filterSubscription)
		end
		list.filterSubscription = list:subscribeToModel(filterModel, function(model)
			rebuildFilteredAndSortedList()
			list:updateDataSource(false, false)
		end, false)

		-- Sort model changes: re-sort from cache
		if list.sortSubscription then
			list:removeSubscription(list.sortSubscription)
		end
		list.sortSubscription = list:subscribeToModel(sortModel, function(model)
			local fieldModel = Engine.GetModel(sortModel, "field")
			local descModel = Engine.GetModel(sortModel, "desc")
			if fieldModel then
				local field = Engine.GetModelValue(fieldModel)
				if field then currentSort.field = field end
			end
			if descModel then
				local desc = Engine.GetModelValue(descModel)
				if desc ~= nil then currentSort.desc = desc end
			end
			rebuildFilteredAndSortedList()
			list:updateDataSource(false, false)
		end, false)
	end,
	getCount = function(list)
		return #filteredIndices
	end,
	getItem = function(controller, list, index)
		local realOffset = filteredIndices[index]
		if not realOffset then return nil end
		return list.updateModels(controller, list, realOffset)
	end,
	cleanup = function(list)
		clearServerCache()
		if list.serverBrowserRootModel then
			Engine.UnsubscribeAndFreeModel(list.serverBrowserRootModel)
			list.serverBrowserRootModel = nil
		end
	end
}

CoD.ServerBrowserRowInternal.new = function(menu, controller)
	local self = LUI.UIHorizontalList.new({
		left = 0,
		top = 0,
		right = 0,
		bottom = 0,
		leftAnchor = true,
		topAnchor = true,
		rightAnchor = true,
		bottomAnchor = true,
		spacing = 2
	})
	self:setAlignment(LUI.Alignment.Left)
	if PreLoadFunc then
		PreLoadFunc(self, controller)
	end
	self:setUseStencil(false)
	self:setClass(CoD.ServerBrowserRowInternal)
	self.id = "ServerBrowserRowInternal"
	self.soundSet = "default"
	self:setLeftRight(true, false, 0, 700)
	self:setTopBottom(true, false, 0, 22)
	self:makeFocusable()
	self.onlyChildrenFocusable = true
	self.anyChildUsesUpdateState = true

	local passwordFlag = CoD.ServerBrowserFlag.new(menu, controller)
	passwordFlag:setLeftRight(true, false, 0, 28)
	passwordFlag:setTopBottom(true, true, 0, 0)
	passwordFlag.icon:setImage(RegisterImage("uie_t7_icon_serverbrowser_protected"))
	passwordFlag:linkToElementModel(self, nil, false, function(model)
		passwordFlag:setModel(model, controller)
	end)
	passwordFlag:mergeStateConditions({
		{
			stateName = "FlagOn",
			condition = function(menu, element, event)
				return IsSelfModelValueTrue(element, controller, "passwordProtected")
			end
		}
	})
	passwordFlag:linkToElementModel(passwordFlag, "passwordProtected", true, function(model)
		menu:updateElementState(passwordFlag, {
			name = "model_validation",
			menu = menu,
			modelValue = Engine.GetModelValue(model),
			modelName = "passwordProtected"
		})
	end)
	self:addElement(passwordFlag)
	self.passwordFlag = passwordFlag

	local dedicatedFlag = CoD.ServerBrowserFlag.new(menu, controller)
	dedicatedFlag:setLeftRight(true, false, 30, 58)
	dedicatedFlag:setTopBottom(true, true, 0, 0)
	dedicatedFlag.icon:setImage(RegisterImage("uie_t7_icon_serverbrowser_dedicated"))
	dedicatedFlag:linkToElementModel(self, nil, false, function(model)
		dedicatedFlag:setModel(model, controller)
	end)
	dedicatedFlag:mergeStateConditions({
		{
			stateName = "FlagOn",
			condition = function(menu, element, event)
				return IsSelfModelValueTrue(element, controller, "dedicated")
			end
		}
	})
	dedicatedFlag:linkToElementModel(dedicatedFlag, "dedicated", true, function(model)
		menu:updateElementState(dedicatedFlag, {
			name = "model_validation",
			menu = menu,
			modelValue = Engine.GetModelValue(model),
			modelName = "dedicated"
		})
	end)
	self:addElement(dedicatedFlag)
	self.dedicatedFlag = dedicatedFlag

	local rankedFlag = CoD.ServerBrowserFlag.new(menu, controller)
	rankedFlag:setLeftRight(true, false, 60, 88)
	rankedFlag:setTopBottom(true, true, 0, 0)
	rankedFlag.icon:setImage(RegisterImage("uie_t7_icon_serverbrowser_ranked"))
	rankedFlag:linkToElementModel(self, nil, false, function(model)
		rankedFlag:setModel(model, controller)
	end)
	rankedFlag:mergeStateConditions({
		{
			stateName = "FlagOn",
			condition = function(menu, element, event)
				return IsSelfModelValueTrue(element, controller, "ranked")
			end
		}
	})
	rankedFlag:linkToElementModel(rankedFlag, "ranked", true, function(model)
		menu:updateElementState(rankedFlag, {
			name = "model_validation",
			menu = menu,
			modelValue = Engine.GetModelValue(model),
			modelName = "ranked"
		})
	end)
	self:addElement(rankedFlag)
	self.rankedFlag = rankedFlag

	local name = CoD.horizontalScrollingTextBox_18pt.new(menu, controller)
	name:setLeftRight(true, false, 90, 330)
	name:setTopBottom(true, false, 2, 20)
	name.textBox:setTTF("fonts/default.ttf")
	name.textBox:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
	name:linkToElementModel(self, "name", true, function(model)
		local _name = Engine.GetModelValue(model)
		if _name then
			name.textBox:setText(Engine.Localize(_name))
		end
	end)
	self:addElement(name)
	self.name = name

	local spacer = LUI.UIFrame.new(menu, controller, 0, 0, false)
	spacer:setLeftRight(true, false, 332, 339)
	spacer:setTopBottom(true, false, 0, 22)
	spacer:setAlpha(0)
	self:addElement(spacer)
	self.spacer = spacer

	local map = CoD.horizontalScrollingTextBox_18pt.new(menu, controller)
	map:setLeftRight(true, false, 341, 446)
	map:setTopBottom(true, false, 2, 20)
	map.textBox:setTTF("fonts/default.ttf")
	map.textBox:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
	map:linkToElementModel(self, "mapName", true, function(model)
		local mapName = Engine.GetModelValue(model)
		if mapName then
			map.textBox:setText(MapNameToLocalizedMapName(mapName))
		end
	end)
	self:addElement(map)
	self.map = map

	local hardcoreFlag = CoD.ServerBrowserFlag.new(menu, controller)
	hardcoreFlag:setLeftRight(true, false, 448, 470)
	hardcoreFlag:setTopBottom(true, true, 0, 0)
	hardcoreFlag.icon:setImage(RegisterImage("uie_t7_icon_serverbrowser_skull"))
	hardcoreFlag:linkToElementModel(self, nil, false, function(model)
		hardcoreFlag:setModel(model, controller)
	end)
	hardcoreFlag:mergeStateConditions({
		{
			stateName = "FlagOn",
			condition = function(menu, element, event)
				return IsSelfModelValueTrue(element, controller, "hardcore")
			end
		}
	})
	hardcoreFlag:linkToElementModel(hardcoreFlag, "hardcore", true, function(model)
		menu:updateElementState(hardcoreFlag, {
			name = "model_validation",
			menu = menu,
			modelValue = Engine.GetModelValue(model),
			modelName = "hardcore"
		})
	end)
	self:addElement(hardcoreFlag)
	self.hardcoreFlag = hardcoreFlag

	local gametype = LUI.UIText.new()
	gametype:setLeftRight(true, false, 472, 576)
	gametype:setTopBottom(true, false, 2, 20)
	gametype:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
	gametype:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
	gametype:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
	gametype:linkToElementModel(self, "gameType", true, function(model)
		local gameType = Engine.GetModelValue(model)
		if gameType then
			gametype:setText(Engine.Localize(GetGameTypeDisplayString(gameType)))
		end
	end)
	self:addElement(gametype)
	self.gametype = gametype

	local playerCount = LUI.UIText.new()
	playerCount:setLeftRight(true, false, 593, 613)
	playerCount:setTopBottom(true, false, 2, 20)
	playerCount:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
	playerCount:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_RIGHT)
	playerCount:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
	playerCount:linkToElementModel(self, "clientCount", true, function(model)
		local clientCount = Engine.GetModelValue(model)
		if clientCount then
			playerCount:setText(Engine.Localize(clientCount))
		end
	end)
	self:addElement(playerCount)
	self.playerCount = playerCount

	local slash = LUI.UIText.new()
	slash:setLeftRight(true, false, 615, 624)
	slash:setTopBottom(true, false, 2, 20)
	slash:setText(Engine.Localize("/"))
	slash:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
	slash:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
	slash:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
	self:addElement(slash)
	self.slash = slash

	local maxPlayers = LUI.UIText.new()
	maxPlayers:setLeftRight(true, false, 626, 645)
	maxPlayers:setTopBottom(true, false, 2, 20)
	maxPlayers:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
	maxPlayers:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
	maxPlayers:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
	maxPlayers:linkToElementModel(self, "maxClients", true, function(model)
		local maxClients = Engine.GetModelValue(model)
		if maxClients then
			maxPlayers:setText(Engine.Localize(maxClients))
		end
	end)
	self:addElement(maxPlayers)
	self.maxPlayers = maxPlayers

	local botCount = LUI.UIText.new()
	botCount:setLeftRight(true, false, 637, 659)
	botCount:setTopBottom(true, false, 2, 20)
	botCount:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
	botCount:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
	botCount:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
	botCount:linkToElementModel(self, "botCount", true, function(model)
		local _botCount = Engine.GetModelValue(model)
		if _botCount then
			botCount:setText("[" .. Engine.Localize(_botCount) .. "]")
		end
	end)
	botCount:linkToElementModel(self, "zombies", true, function(model)
		local zombies = Engine.GetModelValue(model)
		if zombies ~= nil then
			botCount:setAlpha(zombies and 0 or 1)
		end
	end)
	self:addElement(botCount)
	self.botCount = botCount

	local ping = LUI.UIText.new()
	ping:setLeftRight(true, false, 661, 699.37)
	ping:setTopBottom(true, false, 2, 20)
	ping:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
	ping:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)
	ping:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
	ping:linkToElementModel(self, "ping", true, function(model)
		local _ping = Engine.GetModelValue(model)
		if _ping then
			ping:setText(Engine.Localize(_ping))
		end
	end)
	self:addElement(ping)
	self.ping = ping

	spacer.id = "spacer"
	self:registerEventHandler("gain_focus", function(self, event)
		if self.m_focusable and self.spacer:processEvent(event) then
			return true
		else
			return LUI.UIElement.gainFocus(self, event)
		end
	end)
	LUI.OverrideFunction_CallOriginalSecond(self, "close", function(element)
		element.passwordFlag:close()
		element.dedicatedFlag:close()
		element.rankedFlag:close()
		element.name:close()
		element.map:close()
		element.hardcoreFlag:close()
		element.gametype:close()
		element.playerCount:close()
		element.maxPlayers:close()
		element.botCount:close()
		element.ping:close()
	end)

	if PostLoadFunc then
		PostLoadFunc(self, controller, menu)
	end

	return self
end
