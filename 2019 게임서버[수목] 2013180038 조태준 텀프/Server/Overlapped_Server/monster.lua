my_id = -1;
my_x = -1;
my_y = -1;
my_type = "";
my_level = -1;
my_hp = - 1;
my_damage = -1;

function set_info( id, type, x, y, level, hp, damage )
	my_id = id;
	my_x = x;
	my_y = y;
	my_type = type;
	my_level = level;
	my_hp = hp;
	my_damage = damage;
end

function get_info(  )
	return my_x, my_y, my_hp;
end

function get_x()
	return my_x;
end

function get_y()
	return my_y;
end