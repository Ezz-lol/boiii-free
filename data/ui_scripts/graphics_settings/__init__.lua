DataSources.OptionGraphicsVideo = DataSourceHelpers.ListSetup( "PC.OptionGraphicsVideo", function ( controller )
	local videoSettings = {}
	table.insert( videoSettings, {
		models = {
			label = "PLATFORM_DISPLAY_MODE",
			description = "PLATFORM_DISPLAY_MODE_DESC",
			profileVarName = "r_fullscreen",
			datasource = "OptionPCGraphicsDisplayMode",
			widgetType = "dropdown"
		},
		properties = CoD.PCUtil.DependantDropdownProperties
	} )
	table.insert( videoSettings, {
		models = {
			label = "PLATFORM_MONITOR",
			description = "PLATFORM_MONITOR_DESC",
			profileVarName = "r_monitor",
			datasource = "OptionPCGraphicsMonitor",
			widgetType = "dropdown",
			disabledFunction = function ()
				return Engine.GetHardwareProfileValueAsString( "r_fullscreen" ) == "0"
			end
		},
		properties = CoD.PCUtil.DependantDropdownProperties
	} )
	table.insert( videoSettings, {
		models = {
			label = "PLATFORM_SCREEN_RESOLUTION",
			description = "PLATFORM_SCREEN_RESOLUTION_DESC",
			profileVarName = "r_mode",
			datasource = "OptionPCGraphicsScreenResolution",
			widgetType = "dropdown",
			disabledFunction = function ()
				return Engine.GetHardwareProfileValueAsString( "r_fullscreen" ) == "2"
			end
		},
		properties = CoD.PCUtil.DependantDropdownProperties
	} )
	table.insert( videoSettings, {
		models = {
			label = "PLATFORM_REFRESH_RATE",
			description = "PLATFORM_REFRESH_RATE_DESC",
			profileVarName = "r_refreshRate",
			datasource = "OptionPCGraphicsRefreshRate",
			widgetType = "dropdown",
			disabledFunction = function ()
				return Engine.GetHardwareProfileValueAsString( "r_fullscreen" ) ~= "1"
			end
		},
		properties = CoD.PCUtil.OptionsGenericDropdownProperties
	} )
	table.insert( videoSettings, {
		models = {
			label = "PLATFORM_SCENE_RESOLUTION_RATIO",
			description = "PLATFORM_SCENE_RESOLUTION_RATIO_DESC",
			profileVarName = "r_sceneResolutionMultiplier",
			datasource = "OptionPCGraphicsSceneResolution",
			widgetType = "dropdown"
		},
		properties = CoD.PCUtil.DependantDropdownProperties
	} )
	table.insert( videoSettings, {
		models = {
			label = "PLATFORM_DISPLAY_GAMMA",
			description = "PLATFORM_DISPLAY_GAMMA_DESC",
			profileVarName = "r_videoMode",
			datasource = "OptionPCGraphicsDisplayGamma",
			widgetType = "dropdown"
		},
		properties = CoD.PCUtil.OptionsGenericDropdownProperties
	} )
	table.insert( videoSettings, {
		models = {
			label = "PLATFORM_BRIGHTNESS",
			description = "PLATFORM_BRIGHTNESS_DESC",
			profileVarName = "r_sceneBrightness",
			profileType = "user",
			lowValue = -1,
			highValue = 1,
			widgetType = "slider"
		},
		properties = CoD.PCUtil.OptionsGenericSliderProperties
	} )
	table.insert( videoSettings, {
		models = {
			label = "PLATFORM_FOV",
			description = "PLATFORM_FOV_DESC",
			profileVarName = "cg_fov_default",
			lowValue = 65,
			highValue = 120,
			useIntegerDisplay = 1,
			widgetType = "slider"
		},
		properties = CoD.PCUtil.OptionsGenericSliderProperties
	} )
	table.insert( videoSettings, {
		models = {
			widgetType = "spacer",
			height = 32
		}
	} )
	table.insert( videoSettings, {
		models = {
			label = "PLATFORM_MAX_FPS",
			description = "PLATFORM_MAX_FPS_DESC",
			profileVarName = "com_maxfps",
			lowValue = 24,
			highValue = 500,
			useIntegerDisplay = 1,
			widgetType = "slider"
		},
		properties = CoD.PCUtil.OptionsGenericSliderProperties
	} )
	table.insert( videoSettings, {
		models = {
			label = "PLATFORM_SYNC_EVERY_FRAME",
			description = "PLATFORM_VSYNC_DESC",
			profileVarName = "r_vsync",
			widgetType = "checkbox"
		},
		properties = CoD.PCUtil.OptionsGenericCheckboxProperties
	} )
	table.insert( videoSettings, {
		models = {
			label = "PLATFORM_DRAW_FPS",
			description = "PLATFORM_DRAW_FPS_DESC",
			profileVarName = "com_drawFPS_PC",
			widgetType = "checkbox"
		},
		properties = CoD.PCUtil.OptionsGenericCheckboxProperties
	} )
	return videoSettings
end, true )
DataSources.OptionGraphicsVideo.getWidgetTypeForItem = function ( list, dataItemModel, row )
	if dataItemModel then
		local widgetType = Engine.GetModelValue( Engine.GetModel( dataItemModel, "widgetType" ) )
		if widgetType == "dropdown" then
			return CoD.OptionDropdown
		elseif widgetType == "checkbox" then
			return CoD.StartMenu_Options_CheckBoxOption
		elseif widgetType == "slider" then
			return CoD.StartMenu_Options_SliderBar
		elseif widgetType == "spacer" then
			return CoD.VerticalListSpacer
		end
	end
	return nil
end