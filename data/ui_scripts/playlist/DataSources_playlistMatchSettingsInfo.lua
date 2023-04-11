CoD.playlistMatchSettingsInfo = InheritFrom( LUI.UIElement )
CoD.playlistMatchSettingsInfo.new = function ( menu, controller )
	local self = LUI.UIElement.new()

	if PreLoadFunc then
		PreLoadFunc( self, controller )
	end

	self:setUseStencil( false )
	self:setClass( CoD.playlistMatchSettingsInfo )
	self.id = "playlistMatchSettingsInfo"
	self.soundSet = "default"
	self:setLeftRight( true, false, 0, 450 )
	self:setTopBottom( true, false, 0, 550 )
	self.anyChildUsesUpdateState = true
	
	local gameModeInfo = CoD.matchSettingsInfo.new( menu, controller )
	gameModeInfo:setLeftRight( true, false, 0, 446 )
	gameModeInfo:setTopBottom( true, false, 0, 550 )
	gameModeInfo.FRBestTime.BestTimeValueText:setText( Engine.Localize( "--:--:--" ) )
	gameModeInfo:linkToElementModel( self, nil, false, function ( model )
		gameModeInfo:setModel( model, controller )
	end )
	gameModeInfo:mergeStateConditions( {
		{
			stateName = "AspectRatio_1x1",
			condition = function ( menu, element, event )
				return AlwaysTrue()
			end
		}
	} )
	self:addElement( gameModeInfo )
	self.gameModeInfo = gameModeInfo
	
	-- local playlistCount = LUI.UIText.new()
	-- playlistCount:setLeftRight( true, false, 1, 446 )
	-- playlistCount:setTopBottom( true, false, -22, -2 )
	-- playlistCount:setTTF( "fonts/RefrigeratorDeluxe-Regular.ttf" )
	-- playlistCount:setAlignment( Enum.LUIAlignment.LUI_ALIGNMENT_LEFT )
	-- playlistCount:setAlignment( Enum.LUIAlignment.LUI_ALIGNMENT_TOP )
	-- playlistCount:linkToElementModel( self, "playlistCount", true, function ( model )
		-- local _playlistCount = Engine.GetModelValue( model )
		-- if _playlistCount then
			-- playlistCount:setText( Engine.Localize( _playlistCount ) )
		-- end
	-- end )
	-- self:addElement( playlistCount )
	-- self.playlistCount = playlistCount
	
	local PlaylistSpecialEventWidget = CoD.Playlist_SpecialEventWidget.new( menu, controller )
	PlaylistSpecialEventWidget:setLeftRight( true, false, 8.55, 408.55 )
	PlaylistSpecialEventWidget:setTopBottom( true, false, 277, 307 )
	PlaylistSpecialEventWidget:linkToElementModel( self, nil, false, function ( model )
		PlaylistSpecialEventWidget:setModel( model, controller )
	end )
	PlaylistSpecialEventWidget:mergeStateConditions( {
		{
			stateName = "DoubleXPWeaponXP",
			condition = function ( menu, element, event )
				return IsSelfModelValueEqualTo( element, controller, "hasDoubleXP", true ) and IsSelfModelValueEqualTo( element, controller, "hasDoubleWeaponXP", true )
			end
		},
		{
			stateName = "DoubleXPDoubleCryptokeys",
			condition = function ( menu, element, event )
				return IsSelfModelValueEqualTo( element, controller, "hasDoubleXP", true ) and IsSelfModelValueEqualTo( element, controller, "hasDoubleCryptokeys", true )
			end
		},
		{
			stateName = "WeaponXPDoubleCryptokeysCopy",
			condition = function ( menu, element, event )
				return IsSelfModelValueEqualTo( element, controller, "hasDoubleWeaponXP", true ) and IsSelfModelValueEqualTo( element, controller, "hasDoubleCryptokeys", true )
			end
		},
		{
			stateName = "DoubleXP",
			condition = function ( menu, element, event )
				return IsSelfModelValueEqualTo( element, controller, "hasDoubleXP", true )
			end
		},
		{
			stateName = "DoubleWeaponXP",
			condition = function ( menu, element, event )
				return IsSelfModelValueEqualTo( element, controller, "hasDoubleWeaponXP", true )
			end
		},
		{
			stateName = "DoubleCryptokeys",
			condition = function ( menu, element, event )
				return IsSelfModelValueEqualTo( element, controller, "hasDoubleCryptokeys", true )
			end
		}
	} )
	PlaylistSpecialEventWidget:linkToElementModel( PlaylistSpecialEventWidget, "hasDoubleXP", true, function ( model )
		menu:updateElementState( PlaylistSpecialEventWidget, {
			name = "model_validation",
			menu = menu,
			modelValue = Engine.GetModelValue( model ),
			modelName = "hasDoubleXP"
		} )
	end )
	PlaylistSpecialEventWidget:linkToElementModel( PlaylistSpecialEventWidget, "hasDoubleWeaponXP", true, function ( model )
		menu:updateElementState( PlaylistSpecialEventWidget, {
			name = "model_validation",
			menu = menu,
			modelValue = Engine.GetModelValue( model ),
			modelName = "hasDoubleWeaponXP"
		} )
	end )
	PlaylistSpecialEventWidget:linkToElementModel( PlaylistSpecialEventWidget, "hasDoubleCryptokeys", true, function ( model )
		menu:updateElementState( PlaylistSpecialEventWidget, {
			name = "model_validation",
			menu = menu,
			modelValue = Engine.GetModelValue( model ),
			modelName = "hasDoubleCryptokeys"
		} )
	end )
	self:addElement( PlaylistSpecialEventWidget )
	self.PlaylistSpecialEventWidget = PlaylistSpecialEventWidget
	
	self.clipsPerState = {
		DefaultState = {
			DefaultClip = function ()
				self:setupElementClipCounter( 0 )
			end
		}
	}

	LUI.OverrideFunction_CallOriginalSecond( self, "close", function ( element )
		element.gameModeInfo:close()
		element.PlaylistSpecialEventWidget:close()
		-- element.playlistCount:close()
	end )
	
	if PostLoadFunc then
		PostLoadFunc( self, controller, menu )
	end
	
	return self
end