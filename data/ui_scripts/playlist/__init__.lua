if Engine.GetCurrentMap() ~= "core_frontend" then
	return
end

Engine.SetDvar( "lobbyMigrate_Enabled", 0 )
Engine.SetDvar( "lobbyTimerStatusVotingInterval", 11000 )
Engine.SetDvar( "lobbyTimerStatusBeginInterval", 10 )
Engine.SetDvar( "lobbyTimerStatusStartInterval", 10 )
Engine.SetDvar( "lobbyTimerStatusPostGameInterval", 10 )
Engine.SetDvar( "lobbyTimerStatusVotingInterval_Arena", 11000 )
