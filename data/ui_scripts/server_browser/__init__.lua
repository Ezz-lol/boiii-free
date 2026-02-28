if Engine.GetCurrentMap() ~= "core_frontend" then
	return
end

function IsServerBrowserEnabled()
	return true
end

local skullSortedOrder = nil  
local skullSortAscending = nil
local activeServerList = nil

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
		activeServerList = list
		list.servers = {}
		local serversModel = Engine.CreateModel(list.serverBrowserRootModel, "servers")
		for i = 1, list.numElementsInList, 1 do
			list.servers[i] = {}
			list.servers[i].root = Engine.CreateModel(serversModel, "server_" .. i)
			list.servers[i].model = Engine.CreateModel(list.servers[i].root, "model")
		end
		list.updateModels = function(controller, list, offset, displayOffset)
			local serverInfo = Engine.SteamServerBrowser_GetServerInfo(offset)
			if serverInfo then
				local SetModelValue = function(model, key, value)
					local model = Engine.CreateModel(model, key)
					if model then
						Engine.SetModelValue(model, value)
					end
				end

				local elementIndex = (displayOffset or offset) % list.numElementsInList + 1
				local serverModel = list.servers[elementIndex].model
				SetModelValue(serverModel, "serverIndex", serverInfo.serverIndex)
				SetModelValue(serverModel, "connectAddr", serverInfo.connectAddr)
				SetModelValue(serverModel, "ping", serverInfo.ping)
				SetModelValue(serverModel, "modName", serverInfo.modName)
				SetModelValue(serverModel, "mapName", serverInfo.map)
				SetModelValue(serverModel, "desc", serverInfo.desc)
				-- Change the client count to be the actual player count
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
				-- Add the bot count
				SetModelValue(serverModel, "botCount", serverInfo.botCount)
				-- Add rounds played
				SetModelValue(serverModel, "rounds", serverInfo.rounds or 0)

				return serverModel
			else
				return nil
			end
		end

		if list.serverListUpdateSubscription then
			list:removeSubscription(list.serverListUpdateSubscription)
		end
		local serverListUpdateModel = Engine.CreateModel(list.serverBrowserRootModel, "serverListCount")
		list.serverListUpdateSubscription = list:subscribeToModel(serverListUpdateModel, function(model)
			skullSortedOrder = nil
			skullSortAscending = nil
			list:updateDataSource(false, false)
		end, false)
		if list.serverListSortTypeSubscription then
			list:removeSubscription(list.serverListSortTypeSubscription)
		end
		local serverListSortTypeModel = Engine.CreateModel(list.serverBrowserRootModel, "serverListSortType")
		list.serverListSortTypeSubscription = list:subscribeToModel(serverListSortTypeModel, function(model)
			skullSortedOrder = nil
			skullSortAscending = nil
			list:updateDataSource(false, false)
		end, false)
	end,
	getCount = function(list)
		return list.serverCount
	end,
	getItem = function(controller, list, index)
		local displayOffset = index - 1
		local engineOffset = displayOffset
		if skullSortedOrder then
			engineOffset = skullSortedOrder[index]
			if engineOffset == nil then return nil end
		end
		return list.updateModels(controller, list, engineOffset, displayOffset)
	end,
	cleanup = function(list)
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

	local roundText = LUI.UIText.new()
	roundText:setLeftRight(true, true, 0, 0)
	roundText:setTopBottom(true, false, 2, 20)
	roundText:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
	roundText:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_CENTER)
	roundText:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
	roundText:setAlpha(0)
	roundText:linkToElementModel(self, "rounds", true, function(model)
		local rounds = Engine.GetModelValue(model)
		if rounds and rounds > 0 then
			roundText:setText(tostring(rounds))
		else
			roundText:setText("0")
		end
	end)
	roundText:linkToElementModel(self, "zombies", true, function(model)
		local zombies = Engine.GetModelValue(model)
		if zombies ~= nil then
			roundText:setAlpha(zombies and 1 or 0)
			hardcoreFlag.icon:setAlpha(zombies and 0 or 1)
		end
	end)
	hardcoreFlag:addElement(roundText)
	self.roundText = roundText

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
	gametype:setRGB(0.78, 0.78, 0.78)
	self:addElement(gametype)
	self.gametype = gametype

	self._clientCount = 0
	self._maxClients = 0
	local playerCount

	local function updatePlayerCountColor()
		if self._maxClients > 0 then
			local ratio = self._clientCount / self._maxClients
			if ratio < 0.5 then
				playerCount:setRGB(0.2, 1.0, 0.2)
			elseif ratio < 0.8 then
				playerCount:setRGB(1.0, 0.85, 0.1)
			elseif ratio < 0.95 then
				playerCount:setRGB(1.0, 0.55, 0.1)
			else
				playerCount:setRGB(1.0, 0.2, 0.2)
			end
		else
			playerCount:setRGB(1.0, 1.0, 1.0)
		end
	end

	playerCount = LUI.UIText.new()
	playerCount:setLeftRight(true, false, 593, 613)
	playerCount:setTopBottom(true, false, 2, 20)
	playerCount:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
	playerCount:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_RIGHT)
	playerCount:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
	playerCount:linkToElementModel(self, "clientCount", true, function(model)
		local clientCount = Engine.GetModelValue(model)
		if clientCount then
			playerCount:setText(Engine.Localize(clientCount))
			self._clientCount = tonumber(clientCount) or 0
			updatePlayerCountColor()
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
	slash:setRGB(0.6, 0.6, 0.6)
	self:addElement(slash)
	self.slash = slash

	local maxPlayers = LUI.UIText.new()
	maxPlayers:setLeftRight(true, false, 626, 645)
	maxPlayers:setTopBottom(true, false, 2, 20)
	maxPlayers:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
	maxPlayers:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
	maxPlayers:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
	maxPlayers:setRGB(0.7, 0.7, 0.7)
	maxPlayers:linkToElementModel(self, "maxClients", true, function(model)
		local maxClients = Engine.GetModelValue(model)
		if maxClients then
			maxPlayers:setText(Engine.Localize(maxClients))
			self._maxClients = tonumber(maxClients) or 0
			updatePlayerCountColor()
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
	botCount:setRGB(0.6, 0.6, 0.6)
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
			local pingVal = tonumber(_ping) or 0
			
			if pingVal <= 100 then
				ping:setRGB(0.0, 1.0, 0.0)
			elseif pingVal <= 150 then
				ping:setRGB(1.0, 0.9, 0.0)
			elseif pingVal <= 200 then
				ping:setRGB(1.0, 0.5, 0.0)
			else
				ping:setRGB(1.0, 0.0, 0.0)
			end
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

local originalHeaderNew = CoD.ServerBrowserHeader.new
CoD.ServerBrowserHeader.new = function(menu, controller)
	local self = originalHeaderNew(menu, controller)

	if self.skull then
		self.skull:registerEventHandler("button_action", function(element, event)
			if skullSortAscending == nil then
				skullSortAscending = false -- descending
			elseif skullSortAscending == false then
				skullSortAscending = true -- ascending
			else
				skullSortAscending = nil
				skullSortedOrder = nil
				if activeServerList then
					activeServerList:updateDataSource(false, false)
				end
				return
			end

			local countModel = Engine.GetModel(Engine.GetGlobalModel(), "serverBrowser.serverListCount")
			local totalCount = countModel and Engine.GetModelValue(countModel) or 0

			local entries = {}
			for i = 0, totalCount - 1 do
				local info = Engine.SteamServerBrowser_GetServerInfo(i)
				if info then
					local sortVal
					if info.zombies then
						sortVal = info.rounds or 0
					else
						sortVal = info.hardcore and 1 or 0
					end
					table.insert(entries, {idx = i, val = sortVal})
				end
			end

			if skullSortAscending then
				table.sort(entries, function(a, b) return a.val < b.val end)
			else
				table.sort(entries, function(a, b) return a.val > b.val end)
			end

			skullSortedOrder = {}
			for i, entry in ipairs(entries) do
				skullSortedOrder[i] = entry.idx
			end

			if activeServerList then
				activeServerList:updateDataSource(false, false)
			end
		end)
	end

	return self
end