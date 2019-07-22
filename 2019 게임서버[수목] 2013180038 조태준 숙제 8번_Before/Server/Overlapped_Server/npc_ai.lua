myid = 9999;

function set_uid( x )
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

function event_npc_run (player)
	API_SendNPCMessage(player, myid, "BYE")
end