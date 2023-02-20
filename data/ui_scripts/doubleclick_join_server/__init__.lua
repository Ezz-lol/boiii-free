if Engine.GetCurrentMap() ~= "core_frontend" then
	return
end

require( "ui.uieditor.widgets.PC.ServerBrowser.ServerBrowserRowInternal" )
require( "ui.uieditor.widgets.Lobby.Common.FE_FocusBarContainer" )

CoD.ServerBrowserRow = InheritFrom( LUI.UIElement )
CoD.ServerBrowserRow.new = function ( menu, controller )
	local self = LUI.UIElement.new()
	if PreLoadFunc then
		PreLoadFunc( self, controller )
	end
	self:setUseStencil( false )
	self:setClass( CoD.ServerBrowserRow )
	self.id = "ServerBrowserRow"
	self.soundSet = "default"
	self:setLeftRight( true, false, 0, 700 )
	self:setTopBottom( true, false, 0, 22 )
	self:makeFocusable()
	self:setHandleMouse( true )

	if CoD.isPC then
		self:registerEventHandler( "leftmousedown", function ()
			if self:isInFocus() then
				ServerBrowserCancelRequest( self )
				JoinServerBrowser( self, self, controller, menu )
			end
		end )
	end
	self.anyChildUsesUpdateState = true
	
	local background = LUI.UIImage.new()
	background:setLeftRight( true, true, 0, 0 )
	background:setTopBottom( true, true, 0, 0 )
	background:setRGB( 0.2, 0.2, 0.2 )
	background:setAlpha( 0.8 )
	self:addElement( background )
	self.background = background
	
	local rowItems = CoD.ServerBrowserRowInternal.new( menu, controller )
	rowItems:setLeftRight( true, false, 0, 700 )
	rowItems:setTopBottom( true, false, 0, 22 )
	rowItems:linkToElementModel( self, nil, false, function ( model )
		rowItems:setModel( model, controller )
	end )
	self:addElement( rowItems )
	self.rowItems = rowItems
	
	local FocusBarB = CoD.FE_FocusBarContainer.new( menu, controller )
	FocusBarB:setLeftRight( true, true, -2, 2 )
	FocusBarB:setTopBottom( false, true, -1, 3 )
	FocusBarB:setAlpha( 0 )
	FocusBarB:setZoom( 1 )
	self:addElement( FocusBarB )
	self.FocusBarB = FocusBarB
	
	local FocusBarT = CoD.FE_FocusBarContainer.new( menu, controller )
	FocusBarT:setLeftRight( true, true, -2, 2 )
	FocusBarT:setTopBottom( true, false, -2, 2 )
	FocusBarT:setAlpha( 0 )
	FocusBarT:setZoom( 1 )
	self:addElement( FocusBarT )
	self.FocusBarT = FocusBarT
	
	self.clipsPerState = {
		DefaultState = {
			DefaultClip = function ()
				self:setupElementClipCounter( 3 )
				background:completeAnimation()
				self.background:setRGB( 0.2, 0.2, 0.2 )
				self.clipFinished( background, {} )
				FocusBarB:completeAnimation()
				self.FocusBarB:setAlpha( 0 )
				self.clipFinished( FocusBarB, {} )
				FocusBarT:completeAnimation()
				self.FocusBarT:setAlpha( 0 )
				self.clipFinished( FocusBarT, {} )
			end,
			Focus = function ()
				self:setupElementClipCounter( 3 )
				background:completeAnimation()
				self.background:setRGB( 0.2, 0.2, 0.2 )
				self.clipFinished( background, {} )
				FocusBarB:completeAnimation()
				self.FocusBarB:setAlpha( 1 )
				self.clipFinished( FocusBarB, {} )
				FocusBarT:completeAnimation()
				self.FocusBarT:setAlpha( 1 )
				self.clipFinished( FocusBarT, {} )
			end,
			Over = function ()
				self:setupElementClipCounter( 3 )
				background:completeAnimation()
				self.background:setRGB( 0.39, 0.39, 0.39 )
				self.clipFinished( background, {} )
				FocusBarB:completeAnimation()
				self.FocusBarB:setAlpha( 0 )
				self.clipFinished( FocusBarB, {} )
				FocusBarT:completeAnimation()
				self.FocusBarT:setAlpha( 0 )
				self.clipFinished( FocusBarT, {} )
			end
		}
	}
	LUI.OverrideFunction_CallOriginalSecond( self, "close", function ( element )
		element.rowItems:close()
		element.FocusBarB:close()
		element.FocusBarT:close()
		element.m_leftMouseDown = nil
	end )
	
	if PostLoadFunc then
		PostLoadFunc( self, controller, menu )
	end
	
	return self
end