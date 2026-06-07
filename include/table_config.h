#pragma once

struct table_config
{
	int uni;
	unsigned long long game_speed;
	table_config() 
		: uni(0)
		, game_speed(2500)
	{}
};