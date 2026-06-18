#pragma once
#include <string>
#include <array>
#include <cstdint>
#include "enums.h"

struct table_planets
{
	int id = 0;
	std::string name = "";
	int id_owner = 0;
	uint8_t universe = 0;
	uint8_t galaxy = 0;
	uint16_t system = 0;
	uint8_t planet = 0;
	int last_update = 0;
	uint8_t planet_type = 1;
	uint8_t destroyed = 0;

	int b_building = 0;
	std::string b_building_id = "";
	int b_shipyard = 0;
	std::string b_shipyard_id = "";
	int b_shipyard_plus = 0;
	std::string image = "";

	uint16_t diameter = 0;
	uint16_t field_current = 0;
	uint16_t field_max = 0;
	int temp_min = 0;
	int temp_max = 0;

	std::string eco_hash = "";
	double metal = 0.0;
	double metal_perhour = 0.0;
	double metal_max = 0.0;
	double crystal = 0.0;
	double crystal_perhour = 0.0;
	double crystal_max = 0.0;
	double deuterium = 0.0;
	double deuterium_perhour = 0.0;
	double deuterium_max = 0.0;
	double energy_used = 0.0;
	double energy = 0.0;

	// =========================================================================
	// all data related to planet, ships, buildings, defences, missiles
	// =========================================================================
	uint8_t resource[600] = {0};

	// production
	std::string metal_mine_percent = "10";
	std::string crystal_mine_percent = "10";
	std::string deuterium_synthesizer_percent = "10";
	std::string solar_plant_percent = "10";
	std::string fusion_plant_percent = "10";
	std::string solar_satellite_percent = "10";

	int last_jump_time = 0;
	double debris_metal = 0.0;
	double debris_crystal = 0.0;
	int id_moon = 0;
	uint8_t is_bot = 1;
	int last_relocate = 0;

	// flag used to decide if planet needs an update.
	bool need_update = false;
	// flag used to state fleet action
	bool need_fleet_colony = false;

	// Constructor
	table_planets()
	{
	}

	void Reset()
	{
		*this = table_planets();
	}
};