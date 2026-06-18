#pragma once
#include <cstdint>
#include <string>
// struct to send fleet..same types with database..
struct table_fleets
{
	int64_t fleet_id = 0;
	int fleet_owner = 0;
	int8_t fleet_mission = 0;
	int64_t fleet_amount = 0;
	std::string fleet_array = "";
	int8_t fleet_universe = 0;
	int fleet_start_time = 0;
	int fleet_start_id = 0;
	int8_t fleet_start_galaxy = 0;
	int16_t fleet_start_system = 0;
	int8_t fleet_start_planet = 0;
	int8_t fleet_start_type = 0;
	int fleet_end_time = 0;
	int fleet_end_stay = 0;
	int fleet_end_id = 0;
	int8_t fleet_end_galaxy = 1;
	int16_t fleet_end_system = 1;
	int8_t fleet_end_planet = 1;
	int8_t fleet_end_type = 1;
	int16_t fleet_target_obj = 0;
	double fleet_resource_metal = 0;
	double fleet_resource_crystal = 0;
	double fleet_resource_deuterium = 0;
	double fleet_resource_darkmatter = 0;
	int8_t fleet_wanted_resource = 0;
	double fleet_wanted_resource_amount = 0;
	int8_t fleet_no_m_return = 0;
	int fleet_target_owner = 0;
	int fleet_group = 0;
	int8_t fleet_mess = 0;
	int start_time = 0;
	int8_t fleet_busy = 0;
	int8_t hasCanceled = 0;

	void Reset()
	{
		*this = table_fleets();
	}
};