if CoD.StartMenu_ConnectionMeterContainer then
  return
end

local function localize(key, fallback)
  local text = Engine.Localize(key)
  if text == nil or text == "" or text == key then
    return fallback
  end
  return text
end

local function networkIcon(kind, level)
  if Engine.CurrentSessionMode() == Enum.eModes.MODE_MULTIPLAYER then
    local image = LUI.UIImage.new()
    image:setImage(RegisterImage("uie_t7_icon_network_" .. kind .. level))
    return image
  end
  local icon = LUI.UIElement.new()
  local count = kind == "packet" and 3 or 4
  local color = { 0.9, 0.2, 0.15 }
  if level >= count then
    color = { 0.3, 0.85, 0.3 }
  elseif level == count - 1 then
    color = { 1, 0.55, 0.1 }
  end
  for bar = 1, count do
    local image = LUI.UIImage.new()
    local left = 3 + (bar - 1) * 7
    image:setLeftRight(true, false, left, left + 5)
    image:setTopBottom(true, false, 29 - bar * 26 / count, 29)
    if bar <= level then
      image:setRGB(color[1], color[2], color[3])
    else
      image:setRGB(1, 1, 1)
      image:setAlpha(0.25)
    end
    icon:addElement(image)
  end
  return icon
end

CoD.StartMenu_ConnectionMeter_PingImage = InheritFrom(LUI.UIElement)
CoD.StartMenu_ConnectionMeter_PingImage.new = function(menu, controller)
  local self = LUI.UIElement.new()
  if PreLoadFunc then
    PreLoadFunc(self, controller)
  end
  self:setUseStencil(true)
  self:setClass(CoD.StartMenu_ConnectionMeter_PingImage)
  self.id = "StartMenu_ConnectionMeter_PingImage"
  self.soundSet = "default"
  self:setLeftRight(true, false, 0, 29)
  self:setTopBottom(true, false, 0, 31)

  local Level1Image = networkIcon("wired", 1)
  Level1Image:setLeftRight(true, false, 0, 29)
  Level1Image:setTopBottom(true, false, 0, 31)
  Level1Image:setAlpha(0)
  self:addElement(Level1Image)
  self.Level1Image = Level1Image

  local Level2Image = networkIcon("wired", 2)
  Level2Image:setLeftRight(true, false, 0, 29)
  Level2Image:setTopBottom(true, false, 0, 31)
  Level2Image:setAlpha(0)
  self:addElement(Level2Image)
  self.Level2Image = Level2Image

  local Level3Image = networkIcon("wired", 3)
  Level3Image:setLeftRight(true, false, 0, 29)
  Level3Image:setTopBottom(true, false, 0, 31)
  Level3Image:setAlpha(0)
  self:addElement(Level3Image)
  self.Level3Image = Level3Image

  local Level4Image = networkIcon("wired", 4)
  Level4Image:setLeftRight(true, false, 0, 29)
  Level4Image:setTopBottom(true, false, 0, 31)
  Level4Image:setAlpha(0)
  self:addElement(Level4Image)
  self.Level4Image = Level4Image

  local WirelessLevel4Image = networkIcon("wireless", 4)
  WirelessLevel4Image:setLeftRight(true, false, 0, 29)
  WirelessLevel4Image:setTopBottom(true, false, 0, 31)
  WirelessLevel4Image:setAlpha(0)
  self:addElement(WirelessLevel4Image)
  self.WirelessLevel4Image = WirelessLevel4Image

  local WirelessLevel3Image = networkIcon("wireless", 3)
  WirelessLevel3Image:setLeftRight(true, false, 0, 29)
  WirelessLevel3Image:setTopBottom(true, false, 0, 31)
  WirelessLevel3Image:setAlpha(0)
  self:addElement(WirelessLevel3Image)
  self.WirelessLevel3Image = WirelessLevel3Image

  local WirelessLevel2Image = networkIcon("wireless", 2)
  WirelessLevel2Image:setLeftRight(true, false, 0, 29)
  WirelessLevel2Image:setTopBottom(true, false, 0, 31)
  WirelessLevel2Image:setAlpha(0)
  self:addElement(WirelessLevel2Image)
  self.WirelessLevel2Image = WirelessLevel2Image

  local WirelessLevel1Image = networkIcon("wireless", 1)
  WirelessLevel1Image:setLeftRight(true, false, 0, 29)
  WirelessLevel1Image:setTopBottom(true, false, 0, 31)
  WirelessLevel1Image:setAlpha(0)
  self:addElement(WirelessLevel1Image)
  self.WirelessLevel1Image = WirelessLevel1Image

  self.clipsPerState = {
    DefaultState = {
      DefaultClip = function()
        self:setupElementClipCounter(8)
        Level1Image:completeAnimation()
        self.Level1Image:setAlpha(0)
        self.clipFinished(Level1Image, {})
        Level2Image:completeAnimation()
        self.Level2Image:setAlpha(0)
        self.clipFinished(Level2Image, {})
        Level3Image:completeAnimation()
        self.Level3Image:setAlpha(0)
        self.clipFinished(Level3Image, {})
        Level4Image:completeAnimation()
        self.Level4Image:setAlpha(0)
        self.clipFinished(Level4Image, {})
        WirelessLevel4Image:completeAnimation()
        self.WirelessLevel4Image:setAlpha(0)
        self.clipFinished(WirelessLevel4Image, {})
        WirelessLevel3Image:completeAnimation()
        self.WirelessLevel3Image:setAlpha(0)
        self.clipFinished(WirelessLevel3Image, {})
        WirelessLevel2Image:completeAnimation()
        self.WirelessLevel2Image:setAlpha(0)
        self.clipFinished(WirelessLevel2Image, {})
        WirelessLevel1Image:completeAnimation()
        self.WirelessLevel1Image:setAlpha(0)
        self.clipFinished(WirelessLevel1Image, {})
      end,
    },
    Level1 = {
      DefaultClip = function()
        self:setupElementClipCounter(8)
        Level1Image:completeAnimation()
        self.Level1Image:setAlpha(1)
        self.clipFinished(Level1Image, {})
        Level2Image:completeAnimation()
        self.Level2Image:setAlpha(0)
        self.clipFinished(Level2Image, {})
        Level3Image:completeAnimation()
        self.Level3Image:setAlpha(0)
        self.clipFinished(Level3Image, {})
        Level4Image:completeAnimation()
        self.Level4Image:setAlpha(0)
        self.clipFinished(Level4Image, {})
        WirelessLevel4Image:completeAnimation()
        self.WirelessLevel4Image:setAlpha(0)
        self.clipFinished(WirelessLevel4Image, {})
        WirelessLevel3Image:completeAnimation()
        self.WirelessLevel3Image:setAlpha(0)
        self.clipFinished(WirelessLevel3Image, {})
        WirelessLevel2Image:completeAnimation()
        self.WirelessLevel2Image:setAlpha(0)
        self.clipFinished(WirelessLevel2Image, {})
        WirelessLevel1Image:completeAnimation()
        self.WirelessLevel1Image:setAlpha(0)
        self.clipFinished(WirelessLevel1Image, {})
      end,
    },
    Level2 = {
      DefaultClip = function()
        self:setupElementClipCounter(8)
        Level1Image:completeAnimation()
        self.Level1Image:setAlpha(0)
        self.clipFinished(Level1Image, {})
        Level2Image:completeAnimation()
        self.Level2Image:setAlpha(1)
        self.clipFinished(Level2Image, {})
        Level3Image:completeAnimation()
        self.Level3Image:setAlpha(0)
        self.clipFinished(Level3Image, {})
        Level4Image:completeAnimation()
        self.Level4Image:setAlpha(0)
        self.clipFinished(Level4Image, {})
        WirelessLevel4Image:completeAnimation()
        self.WirelessLevel4Image:setAlpha(0)
        self.clipFinished(WirelessLevel4Image, {})
        WirelessLevel3Image:completeAnimation()
        self.WirelessLevel3Image:setAlpha(0)
        self.clipFinished(WirelessLevel3Image, {})
        WirelessLevel2Image:completeAnimation()
        self.WirelessLevel2Image:setAlpha(0)
        self.clipFinished(WirelessLevel2Image, {})
        WirelessLevel1Image:completeAnimation()
        self.WirelessLevel1Image:setAlpha(0)
        self.clipFinished(WirelessLevel1Image, {})
      end,
    },
    Level3 = {
      DefaultClip = function()
        self:setupElementClipCounter(8)
        Level1Image:completeAnimation()
        self.Level1Image:setAlpha(0)
        self.clipFinished(Level1Image, {})
        Level2Image:completeAnimation()
        self.Level2Image:setAlpha(0)
        self.clipFinished(Level2Image, {})
        Level3Image:completeAnimation()
        self.Level3Image:setAlpha(1)
        self.clipFinished(Level3Image, {})
        Level4Image:completeAnimation()
        self.Level4Image:setAlpha(0)
        self.clipFinished(Level4Image, {})
        WirelessLevel4Image:completeAnimation()
        self.WirelessLevel4Image:setAlpha(0)
        self.clipFinished(WirelessLevel4Image, {})
        WirelessLevel3Image:completeAnimation()
        self.WirelessLevel3Image:setAlpha(0)
        self.clipFinished(WirelessLevel3Image, {})
        WirelessLevel2Image:completeAnimation()
        self.WirelessLevel2Image:setAlpha(0)
        self.clipFinished(WirelessLevel2Image, {})
        WirelessLevel1Image:completeAnimation()
        self.WirelessLevel1Image:setAlpha(0)
        self.clipFinished(WirelessLevel1Image, {})
      end,
    },
    Level4 = {
      DefaultClip = function()
        self:setupElementClipCounter(8)
        Level1Image:completeAnimation()
        self.Level1Image:setAlpha(0)
        self.clipFinished(Level1Image, {})
        Level2Image:completeAnimation()
        self.Level2Image:setAlpha(0)
        self.clipFinished(Level2Image, {})
        Level3Image:completeAnimation()
        self.Level3Image:setAlpha(0)
        self.clipFinished(Level3Image, {})
        Level4Image:completeAnimation()
        self.Level4Image:setAlpha(1)
        self.clipFinished(Level4Image, {})
        WirelessLevel4Image:completeAnimation()
        self.WirelessLevel4Image:setAlpha(0)
        self.clipFinished(WirelessLevel4Image, {})
        WirelessLevel3Image:completeAnimation()
        self.WirelessLevel3Image:setAlpha(0)
        self.clipFinished(WirelessLevel3Image, {})
        WirelessLevel2Image:completeAnimation()
        self.WirelessLevel2Image:setAlpha(0)
        self.clipFinished(WirelessLevel2Image, {})
        WirelessLevel1Image:completeAnimation()
        self.WirelessLevel1Image:setAlpha(0)
        self.clipFinished(WirelessLevel1Image, {})
      end,
    },
    WirelessLevel1 = {
      DefaultClip = function()
        self:setupElementClipCounter(8)
        Level1Image:completeAnimation()
        self.Level1Image:setAlpha(0)
        self.clipFinished(Level1Image, {})
        Level2Image:completeAnimation()
        self.Level2Image:setAlpha(0)
        self.clipFinished(Level2Image, {})
        Level3Image:completeAnimation()
        self.Level3Image:setAlpha(0)
        self.clipFinished(Level3Image, {})
        Level4Image:completeAnimation()
        self.Level4Image:setAlpha(0)
        self.clipFinished(Level4Image, {})
        WirelessLevel4Image:completeAnimation()
        self.WirelessLevel4Image:setAlpha(0)
        self.clipFinished(WirelessLevel4Image, {})
        WirelessLevel3Image:completeAnimation()
        self.WirelessLevel3Image:setAlpha(0)
        self.clipFinished(WirelessLevel3Image, {})
        WirelessLevel2Image:completeAnimation()
        self.WirelessLevel2Image:setAlpha(0)
        self.clipFinished(WirelessLevel2Image, {})
        WirelessLevel1Image:completeAnimation()
        self.WirelessLevel1Image:setAlpha(1)
        self.clipFinished(WirelessLevel1Image, {})
      end,
    },
    WirelessLevel2 = {
      DefaultClip = function()
        self:setupElementClipCounter(8)
        Level1Image:completeAnimation()
        self.Level1Image:setAlpha(0)
        self.clipFinished(Level1Image, {})
        Level2Image:completeAnimation()
        self.Level2Image:setAlpha(0)
        self.clipFinished(Level2Image, {})
        Level3Image:completeAnimation()
        self.Level3Image:setAlpha(0)
        self.clipFinished(Level3Image, {})
        Level4Image:completeAnimation()
        self.Level4Image:setAlpha(0)
        self.clipFinished(Level4Image, {})
        WirelessLevel4Image:completeAnimation()
        self.WirelessLevel4Image:setRGB(1, 1, 1)
        self.clipFinished(WirelessLevel4Image, {})
        WirelessLevel3Image:completeAnimation()
        self.WirelessLevel3Image:setRGB(1, 1, 1)
        self.clipFinished(WirelessLevel3Image, {})
        WirelessLevel2Image:completeAnimation()
        self.WirelessLevel2Image:setAlpha(0)
        self.clipFinished(WirelessLevel2Image, {})
        WirelessLevel1Image:completeAnimation()
        self.WirelessLevel1Image:setAlpha(0)
        self.clipFinished(WirelessLevel1Image, {})
      end,
    },
    WirelessLevel3 = {
      DefaultClip = function()
        self:setupElementClipCounter(8)
        Level1Image:completeAnimation()
        self.Level1Image:setAlpha(0)
        self.clipFinished(Level1Image, {})
        Level2Image:completeAnimation()
        self.Level2Image:setAlpha(0)
        self.clipFinished(Level2Image, {})
        Level3Image:completeAnimation()
        self.Level3Image:setAlpha(0)
        self.clipFinished(Level3Image, {})
        Level4Image:completeAnimation()
        self.Level4Image:setAlpha(0)
        self.clipFinished(Level4Image, {})
        WirelessLevel4Image:completeAnimation()
        self.WirelessLevel4Image:setAlpha(0)
        self.clipFinished(WirelessLevel4Image, {})
        WirelessLevel3Image:completeAnimation()
        self.WirelessLevel3Image:setAlpha(1)
        self.clipFinished(WirelessLevel3Image, {})
        WirelessLevel2Image:completeAnimation()
        self.WirelessLevel2Image:setAlpha(0)
        self.clipFinished(WirelessLevel2Image, {})
        WirelessLevel1Image:completeAnimation()
        self.WirelessLevel1Image:setAlpha(0)
        self.clipFinished(WirelessLevel1Image, {})
      end,
    },
    WirelessLevel4 = {
      DefaultClip = function()
        self:setupElementClipCounter(8)
        Level1Image:completeAnimation()
        self.Level1Image:setAlpha(0)
        self.clipFinished(Level1Image, {})
        Level2Image:completeAnimation()
        self.Level2Image:setAlpha(0)
        self.clipFinished(Level2Image, {})
        Level3Image:completeAnimation()
        self.Level3Image:setAlpha(0)
        self.clipFinished(Level3Image, {})
        Level4Image:completeAnimation()
        self.Level4Image:setAlpha(0)
        self.clipFinished(Level4Image, {})
        WirelessLevel4Image:completeAnimation()
        self.WirelessLevel4Image:setAlpha(1)
        self.clipFinished(WirelessLevel4Image, {})
        WirelessLevel3Image:completeAnimation()
        self.WirelessLevel3Image:setAlpha(0)
        self.clipFinished(WirelessLevel3Image, {})
        WirelessLevel2Image:completeAnimation()
        self.WirelessLevel2Image:setAlpha(0)
        self.clipFinished(WirelessLevel2Image, {})
        WirelessLevel1Image:completeAnimation()
        self.WirelessLevel1Image:setAlpha(0)
        self.clipFinished(WirelessLevel1Image, {})
      end,
    },
  }
  self:mergeStateConditions({
    {
      stateName = "Level1",
      condition = function(menu, element, event)
        return IsSelfModelValueGreaterThanOrEqualTo(element, controller, "NETSTATS_LATENCY", 300)
      end,
    },
    {
      stateName = "Level2",
      condition = function(menu, element, event)
        return IsSelfModelValueGreaterThanOrEqualTo(element, controller, "NETSTATS_LATENCY", 200)
          and IsSelfModelValueLessThanOrEqualTo(element, controller, "NETSTATS_LATENCY", 299)
      end,
    },
    {
      stateName = "Level3",
      condition = function(menu, element, event)
        return IsSelfModelValueGreaterThanOrEqualTo(element, controller, "NETSTATS_LATENCY", 100)
          and IsSelfModelValueLessThanOrEqualTo(element, controller, "NETSTATS_LATENCY", 199)
      end,
    },
    {
      stateName = "Level4",
      condition = function(menu, element, event)
        return IsSelfModelValueGreaterThanOrEqualTo(element, controller, "NETSTATS_LATENCY", 0)
          and IsSelfModelValueLessThanOrEqualTo(element, controller, "NETSTATS_LATENCY", 99)
      end,
    },
    {
      stateName = "WirelessLevel1",
      condition = function(menu, element, event)
        return true
      end,
    },
    {
      stateName = "WirelessLevel2",
      condition = function(menu, element, event)
        return true
      end,
    },
    {
      stateName = "WirelessLevel3",
      condition = function(menu, element, event)
        return true
      end,
    },
    {
      stateName = "WirelessLevel4",
      condition = function(menu, element, event)
        return true
      end,
    },
  })
  self:linkToElementModel(self, "NETSTATS_LATENCY", true, function(model)
    menu:updateElementState(self, {
      name = "model_validation",
      menu = menu,
      modelValue = Engine.GetModelValue(model),
      modelName = "NETSTATS_LATENCY",
    })
  end)

  if PostLoadFunc then
    PostLoadFunc(self, controller, menu)
  end

  return self
end

CoD.StartMenu_ConnectionMeter_PacketLossImage = InheritFrom(LUI.UIElement)
CoD.StartMenu_ConnectionMeter_PacketLossImage.new = function(menu, controller)
  local self = LUI.UIElement.new()
  if PreLoadFunc then
    PreLoadFunc(self, controller)
  end
  self:setUseStencil(true)
  self:setClass(CoD.StartMenu_ConnectionMeter_PacketLossImage)
  self.id = "StartMenu_ConnectionMeter_PacketLossImage"
  self.soundSet = "default"
  self:setLeftRight(true, false, 0, 29)
  self:setTopBottom(true, false, 0, 31)

  local Level1Image = networkIcon("packet", 1)
  Level1Image:setLeftRight(true, false, 0, 29)
  Level1Image:setTopBottom(true, false, 0, 31)
  Level1Image:setAlpha(0)
  self:addElement(Level1Image)
  self.Level1Image = Level1Image

  local Level2Image = networkIcon("packet", 2)
  Level2Image:setLeftRight(true, false, 0, 29)
  Level2Image:setTopBottom(true, false, 0, 31)
  Level2Image:setAlpha(0)
  self:addElement(Level2Image)
  self.Level2Image = Level2Image

  local Level3Image = networkIcon("packet", 3)
  Level3Image:setLeftRight(true, false, 0, 29)
  Level3Image:setTopBottom(true, false, 0, 31)
  Level3Image:setAlpha(0)
  self:addElement(Level3Image)
  self.Level3Image = Level3Image

  self.clipsPerState = {
    DefaultState = {
      DefaultClip = function()
        self:setupElementClipCounter(3)
        Level1Image:completeAnimation()
        self.Level1Image:setAlpha(0)
        self.clipFinished(Level1Image, {})
        Level2Image:completeAnimation()
        self.Level2Image:setAlpha(0)
        self.clipFinished(Level2Image, {})
        Level3Image:completeAnimation()
        self.Level3Image:setAlpha(0)
        self.clipFinished(Level3Image, {})
      end,
    },
    Level1 = {
      DefaultClip = function()
        self:setupElementClipCounter(3)
        Level1Image:completeAnimation()
        self.Level1Image:setAlpha(1)
        self.clipFinished(Level1Image, {})
        Level2Image:completeAnimation()
        self.Level2Image:setAlpha(0)
        self.clipFinished(Level2Image, {})
        Level3Image:completeAnimation()
        self.Level3Image:setAlpha(0)
        self.clipFinished(Level3Image, {})
      end,
    },
    Level2 = {
      DefaultClip = function()
        self:setupElementClipCounter(3)
        Level1Image:completeAnimation()
        self.Level1Image:setAlpha(0)
        self.clipFinished(Level1Image, {})
        Level2Image:completeAnimation()
        self.Level2Image:setAlpha(1)
        self.clipFinished(Level2Image, {})
        Level3Image:completeAnimation()
        self.Level3Image:setAlpha(0)
        self.clipFinished(Level3Image, {})
      end,
    },
    Level3 = {
      DefaultClip = function()
        self:setupElementClipCounter(3)
        Level1Image:completeAnimation()
        self.Level1Image:setAlpha(0)
        self.clipFinished(Level1Image, {})
        Level2Image:completeAnimation()
        self.Level2Image:setAlpha(0)
        self.clipFinished(Level2Image, {})
        Level3Image:completeAnimation()
        self.Level3Image:setAlpha(1)
        self.clipFinished(Level3Image, {})
      end,
    },
  }
  self:mergeStateConditions({
    {
      stateName = "Level1",
      condition = function(menu, element, event)
        return IsSelfModelValueGreaterThanOrEqualTo(element, controller, "NETSTATS_PACKETLOSS", 5)
      end,
    },
    {
      stateName = "Level2",
      condition = function(menu, element, event)
        return IsSelfModelValueGreaterThanOrEqualTo(element, controller, "NETSTATS_PACKETLOSS", 2)
          and IsSelfModelValueLessThan(element, controller, "NETSTATS_PACKETLOSS", 5)
      end,
    },
    {
      stateName = "Level3",
      condition = function(menu, element, event)
        return IsSelfModelValueGreaterThanOrEqualTo(element, controller, "NETSTATS_PACKETLOSS", 0)
          and IsSelfModelValueLessThan(element, controller, "NETSTATS_PACKETLOSS", 2)
      end,
    },
  })
  self:linkToElementModel(self, "NETSTATS_PACKETLOSS", true, function(model)
    menu:updateElementState(self, {
      name = "model_validation",
      menu = menu,
      modelValue = Engine.GetModelValue(model),
      modelName = "NETSTATS_PACKETLOSS",
    })
  end)

  if PostLoadFunc then
    PostLoadFunc(self, controller, menu)
  end

  return self
end

local PreLoadFunc = function(self, controller)
  local f1_local0 = Engine.CreateModel(Engine.GetGlobalModel(), "NetStatsInfo", true)
  local f1_local1 = Engine.CreateModel(f1_local0, "NETSTATS_LATENCY", true)
  local f1_local2 = Engine.CreateModel(f1_local0, "NETSTATS_PACKETLOSS", true)
  local f1_local3 = Engine.CreateModel(Engine.GetGlobalModel(), "NetworkInfo", true)
end

CoD.StartMenu_ConnectionMeter = InheritFrom(LUI.UIElement)
CoD.StartMenu_ConnectionMeter.new = function(menu, controller)
  local self = LUI.UIElement.new()
  if PreLoadFunc then
    PreLoadFunc(self, controller)
  end
  self:setUseStencil(true)
  self:setClass(CoD.StartMenu_ConnectionMeter)
  self.id = "StartMenu_ConnectionMeter"
  self.soundSet = "default"
  self:setLeftRight(true, false, 0, 310)
  self:setTopBottom(true, false, 0, 165)
  self.anyChildUsesUpdateState = true

  local bg1 = LUI.UIImage.new()
  bg1:setLeftRight(true, false, 189.63, 307)
  bg1:setTopBottom(true, false, 0, 70)
  bg1:setRGB(0, 0, 0)
  bg1:setAlpha(0.7)
  self:addElement(bg1)
  self.bg1 = bg1

  local bg0 = LUI.UIImage.new()
  bg0:setLeftRight(true, false, 0, 190)
  bg0:setTopBottom(true, false, 0, 70)
  bg0:setRGB(0, 0, 0)
  bg0:setAlpha(0.4)
  self:addElement(bg0)
  self.bg0 = bg0

  local PingLabel = LUI.UITightText.new()
  PingLabel:setLeftRight(true, false, 200, 228)
  PingLabel:setTopBottom(true, false, 22.5, 47.5)
  PingLabel:setText(localize("MPUI_CONNECTION_METER_PING_CAPS", "PING"))
  PingLabel:setTTF("fonts/default.ttf")
  self:addElement(PingLabel)
  self.PingLabel = PingLabel

  local PingBarGraph = LUI.UIImage.new()
  PingBarGraph:setLeftRight(true, false, 0, 190)
  PingBarGraph:setTopBottom(true, false, 0, 70)
  PingBarGraph:setupRenderGraph()
  PingBarGraph:setGraphMode(1)
  self:addElement(PingBarGraph)
  self.PingBarGraph = PingBarGraph

  local bg10 = LUI.UIImage.new()
  bg10:setLeftRight(true, false, 191.63, 309)
  bg10:setTopBottom(true, false, 90, 164)
  bg10:setRGB(0, 0, 0)
  bg10:setAlpha(0.7)
  self:addElement(bg10)
  self.bg10 = bg10

  local bg00 = LUI.UIImage.new()
  bg00:setLeftRight(true, false, 2, 192)
  bg00:setTopBottom(true, false, 90, 164)
  bg00:setRGB(0, 0, 0)
  bg00:setAlpha(0.4)
  self:addElement(bg00)
  self.bg00 = bg00

  local PacketLossBarGraph = LUI.UIImage.new()
  PacketLossBarGraph:setLeftRight(true, false, 2, 192)
  PacketLossBarGraph:setTopBottom(true, false, 90, 164)
  PacketLossBarGraph:setupRenderGraph()
  PacketLossBarGraph:setGraphMode(2)
  self:addElement(PacketLossBarGraph)
  self.PacketLossBarGraph = PacketLossBarGraph

  local TextBox0 = LUI.UIText.new()
  TextBox0:setLeftRight(true, false, 200, 264)
  TextBox0:setTopBottom(true, false, 99, 124)
  TextBox0:setText(localize("MPUI_CONNECTION_METER_PACKET_LOSS_CAPS", "PACKET LOSS"))
  TextBox0:setTTF("fonts/default.ttf")
  TextBox0:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  TextBox0:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
  self:addElement(TextBox0)
  self.TextBox0 = TextBox0

  local PingTextBox = LUI.UIText.new()
  PingTextBox:setLeftRight(true, false, 262, 296)
  PingTextBox:setTopBottom(true, false, 35, 60)
  PingTextBox:setTTF("fonts/default.ttf")
  PingTextBox:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_RIGHT)
  PingTextBox:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
  PingTextBox:subscribeToGlobalModel(controller, "NetStatsInfo", "NETSTATS_LATENCY", function(model)
    local NETSTATS_LATENCY = Engine.GetModelValue(model)
    if NETSTATS_LATENCY then
      PingTextBox:setText(Engine.Localize(AppendString("ms", NETSTATS_LATENCY)))
    end
  end)
  self:addElement(PingTextBox)
  self.PingTextBox = PingTextBox

  local PingTextBox0 = LUI.UIText.new()
  PingTextBox0:setLeftRight(true, false, 262, 296)
  PingTextBox0:setTopBottom(true, false, 125, 150)
  PingTextBox0:setTTF("fonts/default.ttf")
  PingTextBox0:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_RIGHT)
  PingTextBox0:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_TOP)
  PingTextBox0:subscribeToGlobalModel(controller, "NetStatsInfo", "NETSTATS_PACKETLOSS", function(model)
    local NETSTATS_PACKETLOSS = Engine.GetModelValue(model)
    if NETSTATS_PACKETLOSS then
      PingTextBox0:setText(Engine.Localize(AppendString("%", NETSTATS_PACKETLOSS)))
    end
  end)
  self:addElement(PingTextBox0)
  self.PingTextBox0 = PingTextBox0

  local StartMenuConnectionMeterPingImage0 = CoD.StartMenu_ConnectionMeter_PingImage.new(menu, controller)
  StartMenuConnectionMeterPingImage0:setLeftRight(true, false, 262, 291)
  StartMenuConnectionMeterPingImage0:setTopBottom(true, false, 4, 35)
  StartMenuConnectionMeterPingImage0:subscribeToGlobalModel(controller, "NetStatsInfo", nil, function(model)
    StartMenuConnectionMeterPingImage0:setModel(model, controller)
  end)
  self:addElement(StartMenuConnectionMeterPingImage0)
  self.StartMenuConnectionMeterPingImage0 = StartMenuConnectionMeterPingImage0

  local StartMenuConnectionMeterPacketLossImage0 = CoD.StartMenu_ConnectionMeter_PacketLossImage.new(menu, controller)
  StartMenuConnectionMeterPacketLossImage0:setLeftRight(true, false, 264, 293)
  StartMenuConnectionMeterPacketLossImage0:setTopBottom(true, false, 94, 125)
  StartMenuConnectionMeterPacketLossImage0:subscribeToGlobalModel(controller, "NetStatsInfo", nil, function(model)
    StartMenuConnectionMeterPacketLossImage0:setModel(model, controller)
  end)
  self:addElement(StartMenuConnectionMeterPacketLossImage0)
  self.StartMenuConnectionMeterPacketLossImage0 = StartMenuConnectionMeterPacketLossImage0

  LUI.OverrideFunction_CallOriginalSecond(self, "close", function(element)
    element.StartMenuConnectionMeterPingImage0:close()
    element.StartMenuConnectionMeterPacketLossImage0:close()
    element.PingTextBox:close()
    element.PingTextBox0:close()
  end)

  if PostLoadFunc then
    PostLoadFunc(self, controller, menu)
  end

  return self
end

CoD.StartMenu_ConnectionMeterContainer = InheritFrom(LUI.UIElement)
CoD.StartMenu_ConnectionMeterContainer.new = function(menu, controller)
  local self = LUI.UIElement.new()
  if PreLoadFunc then
    PreLoadFunc(self, controller)
  end
  self:setUseStencil(true)
  self:setClass(CoD.StartMenu_ConnectionMeterContainer)
  self.id = "StartMenu_ConnectionMeterContainer"
  self.soundSet = "default"
  self:setLeftRight(true, false, 0, 310)
  self:setTopBottom(true, false, 0, 200)
  self.anyChildUsesUpdateState = true

  local StartMenuConnectionMeter0 = CoD.StartMenu_ConnectionMeter.new(menu, controller)
  StartMenuConnectionMeter0:setLeftRight(true, false, 0, 310)
  StartMenuConnectionMeter0:setTopBottom(true, false, 30, 195)
  self:addElement(StartMenuConnectionMeter0)
  self.StartMenuConnectionMeter0 = StartMenuConnectionMeter0

  local ConnectionMeterLabel = LUI.UITightText.new()
  ConnectionMeterLabel:setLeftRight(true, false, 0, 121)
  ConnectionMeterLabel:setTopBottom(true, false, 0, 25)
  ConnectionMeterLabel:setText(localize("MPUI_CONNECTION_METER_CAPS", "CONNECTION METER"))
  ConnectionMeterLabel:setTTF("fonts/default.ttf")
  self:addElement(ConnectionMeterLabel)
  self.ConnectionMeterLabel = ConnectionMeterLabel

  self.clipsPerState = {
    DefaultState = {
      DefaultClip = function()
        self:setupElementClipCounter(2)
        StartMenuConnectionMeter0:completeAnimation()
        self.StartMenuConnectionMeter0:setAlpha(0)
        self.clipFinished(StartMenuConnectionMeter0, {})
        ConnectionMeterLabel:completeAnimation()
        self.ConnectionMeterLabel:setAlpha(0)
        self.clipFinished(ConnectionMeterLabel, {})
      end,
    },
    Visible = {
      DefaultClip = function()
        self:setupElementClipCounter(2)
        StartMenuConnectionMeter0:completeAnimation()
        self.StartMenuConnectionMeter0:setAlpha(1)
        self.clipFinished(StartMenuConnectionMeter0, {})
        ConnectionMeterLabel:completeAnimation()
        self.ConnectionMeterLabel:setAlpha(1)
        self.clipFinished(ConnectionMeterLabel, {})
      end,
    },
  }
  self:mergeStateConditions({
    {
      stateName = "Visible",
      condition = function(menu, element, event)
        return ConnectionMeter_isEnabled(element, controller) and not IsLobbyGameHost()
      end,
    },
  })
  if self.m_eventHandlers.on_session_start then
    local f1_local3 = self.m_eventHandlers.on_session_start
    self:registerEventHandler("on_session_start", function(self, event)
      event.menu = event.menu or menu
      self:updateState(event)
      return f1_local3(self, event)
    end)
  else
    self:registerEventHandler("on_session_start", LUI.UIElement.updateState)
  end
  if self.m_eventHandlers.on_session_end then
    local f1_local3 = self.m_eventHandlers.on_session_end
    self:registerEventHandler("on_session_end", function(self, event)
      event.menu = event.menu or menu
      self:updateState(event)
      return f1_local3(self, event)
    end)
  else
    self:registerEventHandler("on_session_end", LUI.UIElement.updateState)
  end
  LUI.OverrideFunction_CallOriginalSecond(self, "close", function(element)
    element.StartMenuConnectionMeter0:close()
  end)
  if PostLoadFunc then
    PostLoadFunc(self, controller, menu)
  end

  return self
end
