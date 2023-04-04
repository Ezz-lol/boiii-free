Dvar.purchasedClassSetCount:set( 10 )

Engine.GetCustomClassCount = function( arg0 )
	return 10
end

Engine.GetNumberOfClassSetsOwned = function( arg0 )
	return 10
end

Engine.IsClassSetsAvailableForCurrentGameMode = function()
	return true
end

function IsClassSetsAvailableForCurrentGameMode()
	return  Engine.IsClassSetsAvailableForCurrentGameMode()
end

function DoesPlayerHaveExtraSlotsItem( arg0 )
	return true
end
