myid = 9999;
my_name = "";
function set_info( x )
	myid = x;
end

function event_player_move (player)
	player_x = API_get_player_x(player)
	player_y = API_get_player_y(player)
	my_x = API_get_npc_x(myid)
	my_y = API_get_npc_y(myid)

	if(my_x == player_x) then
		if(my_y == player_y) then
			API_NPCDetectPlayer(player, myid)
			API_SendNPCMessage(player, myid, "HELLO")
		end
	end
end

myruntime = 0

function event_npc_run (player)
	local temptime = myruntime + 1
	myruntime = temptime
	if(myruntime > 2) then
		myruntime = 0
		API_NPCRunFinish(myid)
		API_SendNPCMessage(player, myid, "BYE")
	else
		API_AddTimerNPCRun(1000, player, myid)
	end
end