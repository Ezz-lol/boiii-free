-- Black Market weapons in Zombies weapon kits
if Engine.GetCurrentMap() ~= "core_frontend" then return end

local bmWeapons = {
	-- SMGs
	{16, "smg_ak74u", "weapon_smg", "primary"},
	{17, "smg_mp40", "weapon_smg", "primary"},
	{38, "smg_ppsh", "weapon_smg", "primary"},
	-- ARs
	{7,  "ar_peacekeeper", "weapon_assault", "primary"},
	{28, "ar_garand", "weapon_assault", "primary"},
	{29, "ar_famas", "weapon_assault", "primary"},
	{37, "ar_m16", "weapon_assault", "primary"},
	{126,"ar_galil", "weapon_assault", "primary"},
	{129,"ar_m14", "weapon_assault", "primary"},
	-- LMGs
	{128,"lmg_rpk", "weapon_lmg", "primary"},
	-- Shotguns
	{56, "shotgun_energy", "weapon_cqb", "primary"},
	-- Pistols
	{8,  "pistol_shotgun", "weapon_pistol", "secondary"},
	{47, "pistol_energy", "weapon_pistol", "secondary"},
	{48, "pistol_m1911", "weapon_pistol", "secondary"},
	-- Launchers
	{39, "launcher_ex41", "weapon_launcher", "secondary"},
	-- Melee
	{121,"melee_katana", "weapon_knife", "secondary"},
	{109,"melee_nunchuks", "weapon_knife", "secondary"},
	{68, "melee_mace", "weapon_knife", "secondary"},
	{84, "melee_dagger", "weapon_knife", "secondary"},
}

local bmRefSet, bmIndexSet, bmIndexToRef, bmIndexToName = {}, {}, {}, {}
for _, w in ipairs(bmWeapons) do
	bmRefSet[w[2]] = true
	bmIndexSet[w[1]] = true
	bmIndexToRef[w[1]] = w[2]
end

for _, w in ipairs(bmWeapons) do
	pcall(function()
		local val = Engine.GetItemName(w[1], Enum.eModes.MODE_MULTIPLAYER)
		if val and val ~= "" then bmIndexToName[w[1]] = val end
	end)
end

if not __bm_originals then
	__bm_originals = {
		IsItemLocked            = Engine.IsItemLocked,
		IsItemLockedForAll      = Engine.IsItemLockedForAll,
		BMIsItemLocked          = CoD.BlackMarketUtility and CoD.BlackMarketUtility.IsItemLocked,
		BMGetItemQuantity       = CoD.BlackMarketUtility and CoD.BlackMarketUtility.GetItemQuantity,
		GetUnlockablesTable     = CoD.GetUnlockablesTable,
		IsCACItemLocked         = IsCACItemLocked,
		IsCACItemLockedOrNP     = IsCACItemLockedOrNotPurchased,
		IsItemRefLocked         = IsItemRefLocked,
		GetCustomization        = CoD.GetCustomization,
		ListHelper_Prepare      = ListHelper_Prepare,
		-- Engine functions (saved to restore if old version overrode them)
		GetNumAttachments       = Engine.GetNumAttachments,
		IsOptic                 = Engine.IsOptic,
		GetItemAttachment       = Engine.GetItemAttachment,
		GetAttachmentAllocCost  = Engine.GetAttachmentAllocationCost,
		GetAttachmentRef        = Engine.GetAttachmentRef,
		GetAttachmentDesc       = Engine.GetAttachmentDesc,
		GetAttachUniqueImage    = Engine.GetAttachmentUniqueImageByAttachmentIndex,
		GetAttachmentName       = Engine.GetAttachmentNameByIndex,
	}
end
local O = __bm_originals

local ZM = Enum.eModes.MODE_ZOMBIES
local MP = Enum.eModes.MODE_MULTIPLAYER

Engine.GetNumAttachments = O.GetNumAttachments
Engine.IsOptic = O.IsOptic
Engine.GetItemAttachment = O.GetItemAttachment
Engine.GetAttachmentAllocationCost = O.GetAttachmentAllocCost
Engine.GetAttachmentRef = O.GetAttachmentRef
Engine.GetAttachmentDesc = O.GetAttachmentDesc
Engine.GetAttachmentUniqueImageByAttachmentIndex = O.GetAttachUniqueImage
Engine.GetAttachmentNameByIndex = O.GetAttachmentName

if O.IsItemLocked then
	Engine.IsItemLocked = function(ctrl, idx, ...)
		if idx and bmIndexSet[idx] then return false end
		return O.IsItemLocked(ctrl, idx, ...)
	end
end

if O.IsItemLockedForAll then
	Engine.IsItemLockedForAll = function(idx, ...)
		if idx and bmIndexSet[idx] then return false end
		return O.IsItemLockedForAll(idx, ...)
	end
end

if CoD.BlackMarketUtility then
	if O.BMIsItemLocked then
		CoD.BlackMarketUtility.IsItemLocked = function(ctrl, ref, ...)
			if ref and bmRefSet[ref] then return false end
			return O.BMIsItemLocked(ctrl, ref, ...)
		end
	end
	if O.BMGetItemQuantity then
		CoD.BlackMarketUtility.GetItemQuantity = function(ctrl, ref, ...)
			if ref and bmRefSet[ref] then return 1 end
			return O.BMGetItemQuantity(ctrl, ref, ...)
		end
	end
end

local function getRefFromElement(element)
	if not element or not element.getModel then return nil end
	local m = element:getModel()
	if not m then return nil end
	local rm = Engine.GetModel(m, "ref")
	return rm and Engine.GetModelValue(rm)
end

if O.IsCACItemLocked then
	IsCACItemLocked = function(element, ctrl, ...)
		local ok, ref = pcall(getRefFromElement, element)
		if ok and ref and bmRefSet[ref] then return false end
		return O.IsCACItemLocked(element, ctrl, ...)
	end
end

if O.IsCACItemLockedOrNP then
	IsCACItemLockedOrNotPurchased = function(element, ctrl, ...)
		local ok, ref = pcall(getRefFromElement, element)
		if ok and ref and bmRefSet[ref] then return false end
		return O.IsCACItemLockedOrNP(element, ctrl, ...)
	end
end

if O.IsItemRefLocked then
	IsItemRefLocked = function(ctrl, ref, ...)
		if ref and bmRefSet[ref] then return false end
		return O.IsItemRefLocked(ctrl, ref, ...)
	end
end

if O.GetUnlockablesTable then
	CoD.GetUnlockablesTable = function(ctrl, filter, mode)
		local result = O.GetUnlockablesTable(ctrl, filter, mode)
		if not result or not result.filterList then return result end

		local existing = {}
		for _, model in ipairs(result.filterList) do
			local ok, ref = pcall(function()
				return Engine.GetModelValue(Engine.GetModel(model, "ref"))
			end)
			if ok and ref then existing[ref] = true end
		end

		for _, w in ipairs(bmWeapons) do
			local idx, ref, group, slot = w[1], w[2], w[3], w[4]
			if not existing[ref] then
				local ok, model = pcall(Engine.GetUnlockableInfoModelByIndex, idx, "BMInject." .. idx, MP)
				if ok and model then
					pcall(function()
						Engine.SetModelValue(Engine.CreateModel(model, "isBMItem"), false)
						Engine.SetModelValue(Engine.CreateModel(model, "isBMClassified"), false)
						Engine.SetModelValue(Engine.CreateModel(model, "isContractClassified"), false)
						Engine.SetModelValue(Engine.CreateModel(model, "isLocked"), false)
						Engine.SetModelValue(Engine.CreateModel(model, "itemType"), Enum.VoteItemType.VOTE_ITEM_TYPE_ITEM)
						local km = Engine.GetModel(model, "name")
						if km then
							local key = Engine.GetModelValue(km)
							if key and key ~= "" then
								local loc = Engine.Localize(key)
								if loc and loc ~= key then
									Engine.SetModelValue(Engine.CreateModel(model, "displayName"), loc)
									bmIndexToName[idx] = key
								end
							end
						end
					end)
					if not result[group] then result[group] = {} end
					table.insert(result[group], model)
					if not result[slot] then result[slot] = {} end
					table.insert(result[slot], model)
					table.insert(result.filterList, model)
				end
			end
		end
		return result
	end
end

if CoD.CACUtility then
	for ref in pairs(bmRefSet) do
		if CoD.CACUtility.WeaponsWithNoZMCamos then CoD.CACUtility.WeaponsWithNoZMCamos[ref] = nil end
		if CoD.CACUtility.WeaponsWithNoCPCamos then CoD.CACUtility.WeaponsWithNoCPCamos[ref] = nil end
	end
end
if CoD.BlackMarketUtility and CoD.BlackMarketUtility.WeaponsWithNoBMCamos then
	for ref in pairs(bmRefSet) do
		CoD.BlackMarketUtility.WeaponsWithNoBMCamos[ref] = nil
	end
end

if O.GetCustomization then
	CoD.GetCustomization = function(ctrl, key, ...)
		local result = O.GetCustomization(ctrl, key, ...)
		if (result == nil or result == "") and key == "weapon_ref" then
			local idx = O.GetCustomization(ctrl, "weapon_index")
			if idx and bmIndexToRef[idx] then
				return bmIndexToRef[idx]
			end
		end
		return result
	end
end

do
	local function bmAttachmentBuilder(ctrl, weapIdx, weapRef)
		local numAtt = Engine.GetNumAttachments(weapIdx, MP)
		if not numAtt or numAtt <= 1 then return {} end

		local currentSlot, attType = nil, nil
		pcall(function()
			if CoD.perController[ctrl] then
				attType = CoD.perController[ctrl].gunsmithAttachmentType
				if CoD.perController[ctrl].gunsmithAttachmentModel then
					currentSlot = Engine.GetModelValue(CoD.perController[ctrl].gunsmithAttachmentModel)
				end
			end
		end)

		local baseStat = { damage = 0, range = 0, fireRate = 0, accuracy = 0 }
		pcall(function()
			O.GetUnlockablesTable(ctrl)
			local gm = Engine.GetGlobalModel()
			local um = Engine.GetModel(gm, "Unlockables")
			if um then
				local wam = Engine.GetModel(um, weapIdx .. ".weaponAttributes")
				if wam then
					for _, sn in ipairs({"damage", "range", "fireRate", "accuracy"}) do
						local sm = Engine.GetModel(wam, sn)
						if sm then
							local v = Engine.GetModelValue(sm)
							if v and v ~= "" then baseStat[sn] = tonumber(v) or 0 end
						end
					end
				end
			end
		end)

		local csv = "gamedata/weapons/mp/mp_attributesTable.csv"
		local REF   = Enum.WeaponAttributesColumn.WEAPONATTRIBUTES_REFERENCE
		local WNAME = Enum.WeaponAttributesColumn.WEAPONATTRIBUTES_WEAPON_NAME
		local DMG   = Enum.WeaponAttributesColumn.WEAPONATTRIBUTES_DAMAGE
		local RNG   = Enum.WeaponAttributesColumn.WEAPONATTRIBUTES_RANGE
		local FR    = Enum.WeaponAttributesColumn.WEAPONATTRIBUTES_FIRE_RATE
		local ACC   = Enum.WeaponAttributesColumn.WEAPONATTRIBUTES_ACCURACY

		local function safeLookup(col, ref)
			local ok, val = pcall(Engine.TableLookup, nil, csv, REF, ref, WNAME, weapRef, col)
			if ok and val and val ~= "" then return tonumber(val) or 0 end
			return 0
		end

		local accum = { damage = 0, range = 0, fireRate = 0, accuracy = 0 }
		pcall(function()
			local maxAtt = (CoD.CraftUtility and CoD.CraftUtility.Gunsmith
				and CoD.CraftUtility.Gunsmith.MAX_ATTACHMENTS) or 8
			if CoD.perController[ctrl] and CoD.perController[ctrl].gunsmithVariantModel then
				for i = 1, maxAtt do
					local sm = Engine.GetModel(CoD.perController[ctrl].gunsmithVariantModel, "attachment" .. i)
					if sm then
						local ai = Engine.GetModelValue(sm)
						if ai and (not currentSlot or currentSlot ~= ai) and ai > 0 then
							local aref = Engine.GetAttachmentRefByIndex(ai)
							if aref then
								accum.damage   = accum.damage   + safeLookup(DMG, aref)
								accum.range    = accum.range    + safeLookup(RNG, aref)
								accum.fireRate = accum.fireRate + safeLookup(FR, aref)
								accum.accuracy = accum.accuracy + safeLookup(ACC, aref)
							end
						end
					end
				end
			end
		end)

		local items = {}
		local emptyIdx = (CoD.CraftUtility and CoD.CraftUtility.Gunsmith
			and CoD.CraftUtility.Gunsmith.EMPTY_ITEM_INDEX) or 0

		for i = 1, numAtt - 1 do
			local ok, item = pcall(function()
				local isOptic = Engine.IsOptic(weapIdx, i, MP)
				if attType == "attachment" then
					if isOptic then return nil end
				elseif attType == "optic" then
					if not isOptic then return nil end
				end

				local allocCost = Engine.GetAttachmentAllocationCost(weapIdx, i, MP)
				if not allocCost or allocCost <= 0 then return nil end

				local attIdx  = Engine.GetItemAttachment(weapIdx, i, MP)
				if not attIdx or attIdx <= 0 then attIdx = i end

				local attRef  = Engine.GetAttachmentRef(weapIdx, i, MP) or ""
				local attName = Engine.GetAttachmentNameByIndex(attIdx) or ""
				local attImg  = Engine.GetAttachmentUniqueImageByAttachmentIndex(MP, weapIdx, attIdx) or ""
				local attDesc = Engine.GetAttachmentDesc(weapIdx, i, MP) or ""

				local function statStr(name, col)
					local base = baseStat[name] or 0
					local s = safeLookup(col, attRef)
					return base .. "," .. (base + accum[name] + s)
				end

				return {
					models = {
						name = attName,
						image = attImg,
						itemIndex = i,
						weaponIndex = weapIdx,
						attachmentIndex = attIdx,
						description = attDesc,
						acvIndex = emptyIdx,
						weaponAttributes = {
							damage   = statStr("damage", DMG),
							range    = statStr("range", RNG),
							fireRate = statStr("fireRate", FR),
							accuracy = statStr("accuracy", ACC),
						},
						hintText = nil,
					},
					properties = {
						selectIndex = (currentSlot == attIdx),
					},
				}
			end)
			if ok and item then
				table.insert(items, item)
			end
		end

		return items
	end

	local origLHP = O.ListHelper_Prepare or ListHelper_Prepare
	if origLHP then
		ListHelper_Prepare = function(context, ctrl, dsName, builder, mcb, postProcess)
			if dsName == "GunsmithWeaponAttachments" then
				local weapIdx = nil
				pcall(function() weapIdx = CoD.GetCustomization(ctrl, "weapon_index") end)
				if weapIdx and bmIndexSet[weapIdx] then
					local weapRef = bmIndexToRef[weapIdx]
					builder = function(c) return bmAttachmentBuilder(c, weapIdx, weapRef) end
				end
			end
			return origLHP(context, ctrl, dsName, builder, mcb, postProcess)
		end
	end
end

local function getBMWeaponName(idx)
	local nameKey = bmIndexToName[idx]
	if nameKey then return nameKey end
	local ok, val = pcall(Engine.GetItemName, idx, MP)
	if ok and val and val ~= "" then
		bmIndexToName[idx] = val
		return val
	end
	return nil
end

if LocalizeWeaponNameIntoString then
	local origLWNIS = LocalizeWeaponNameIntoString
	LocalizeWeaponNameIntoString = function(stringKey, modeStr, ctrl, weapIdx)
		local idx = tonumber(weapIdx)
		if idx and bmIndexSet[idx] then
			local nameKey = getBMWeaponName(idx)
			if nameKey then
				local ok, result = pcall(function()
					local upper = Engine.ToUpper(Engine.Localize(nameKey))
					return LocalizeIntoString(stringKey, upper)
				end)
				if ok and result and result ~= "" then return result end
			end
		end
		return origLWNIS(stringKey, modeStr, ctrl, weapIdx)
	end
end

if GetItemNameFromIndexMP then
	local origGINMP = GetItemNameFromIndexMP
	GetItemNameFromIndexMP = function(idx)
		if idx and bmIndexSet[idx] then
			local nameKey = getBMWeaponName(idx)
			if nameKey then return nameKey end
		end
		return origGINMP(idx)
	end
end

if GetGunsmithItemNameByIndex then
	local origGSINBI = GetGunsmithItemNameByIndex
	GetGunsmithItemNameByIndex = function(idx)
		if idx and bmIndexSet[idx] then
			local nameKey = getBMWeaponName(idx)
			if nameKey then return nameKey end
		end
		return origGSINBI(idx)
	end
end

if GetGunsmithAttachmentVariantMenuTitle then
	local origVariantTitle = GetGunsmithAttachmentVariantMenuTitle
	GetGunsmithAttachmentVariantMenuTitle = function(ctrl, arg1)
		local weapIdx = nil
		pcall(function() weapIdx = CoD.GetCustomization(ctrl, "weapon_index") end)
		if weapIdx and bmIndexSet[weapIdx] then
			local ok, result = pcall(function()
				local locTitle = Engine.Localize(arg1)
				if CoD.perController[ctrl] and CoD.perController[ctrl].gunsmithAttachmentModel then
					local attSlot = Engine.GetModelValue(CoD.perController[ctrl].gunsmithAttachmentModel)
					local nameKey = getBMWeaponName(weapIdx) or ""
					local weapName = Engine.Localize(nameKey)
					local attName = LocalizeToUpperString(Engine.GetAttachmentNameByIndex(attSlot))
					return weapName .. " " .. attName .. " " .. locTitle
				end
				return ""
			end)
			if ok and result then return result end
		end
		return origVariantTitle(ctrl, arg1)
	end
end

