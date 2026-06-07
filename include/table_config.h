#pragma once

struct table_config
{
	int uni;
	unsigned long long game_speed;
	int resource_multiplier;
	int metal_basic_income;
	int crystal_basic_income;
	int deuterium_basic_income;
	float max_overflow;
	table_config() 
		: uni(0)
		, game_speed(2500)
		, resource_multiplier(1)
		, metal_basic_income(20)
		, crystal_basic_income(10)
		, deuterium_basic_income(0)
		, max_overflow(1.0)
	{}
};