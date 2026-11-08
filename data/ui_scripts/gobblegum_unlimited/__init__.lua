for _, widget in ipairs({
  "CAC/GridItemConsumableLabel",
  "CAC/GridItemButtonNew",
  "BubbleGumBuffs/BubbleGumBuff",
  "BubbleGumBuffs/BubbleGumBuffInGame",
  "BubbleGumBuffs/EquippedBubbleGumBuffButton",
  "BubbleGumBuffs/GobbleGumCookBookIcon",
  "Lobby/Lists/Members/LobbyMemberBubbleGumBuffs",
}) do
  if game.ismodrawfile("ui/uieditor/widgets/" .. widget .. ".lua") then
    return
  end
end

local LOBBY_COUNT_CAP = 255

local function isUnlimited(count)
  return count == LOBBY_COUNT_CAP or (count > LOBBY_COUNT_CAP and Engine.DvarBool(nil, "cg_unlockall_gobblegums"))
end

local function isGumCount(text)
  local parent = text:getParent()
  return parent and parent.ComsumableCountLabel == text
end

LUI.UIText.boiiiNativeSetText = LUI.UIText.boiiiNativeSetText or LUI.UIText.setText
local setText = LUI.UIText.boiiiNativeSetText
LUI.UIText.setText = function(self, value)
  local count = type(value) == "string" and tonumber(string.match(value, "^%D*(%d+)%D*$"))
  if count and count >= LOBBY_COUNT_CAP and isGumCount(self) and isUnlimited(count) then
    self:setLeftRight(false, false, -11, 10)
    self:setTopBottom(false, false, -13, 5)
    self:setScale(1.35)
    self.boiiiUnlimited = true
    return setText(self, Engine.Localize("∞"))
  end
  if self.boiiiUnlimited then
    self:setLeftRight(false, false, -10.5, 10.5)
    self:setTopBottom(false, false, -10, 8)
    self:setScale(1)
    self.boiiiUnlimited = nil
  end
  return setText(self, value)
end

local function refreshExisting(element)
  local text = element.ComsumableCountLabel
  local model = text and element:getModel()
  local remaining = model and Engine.GetModel(model, "remaining")
  if remaining and Engine.GetModelValue(remaining) then
    text:setText(Engine.Localize(Engine.GetModelValue(remaining)))
  end
  local child = element:getFirstChild()
  while child do
    refreshExisting(child)
    child = child:getNextSibling()
  end
end

for _, root in pairs(LUI.roots) do
  refreshExisting(root)
end
