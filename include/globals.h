#pragma once

#include <map>
#include <vector>
#include <string>
#include <chrono>
#include <unordered_map>

struct vars_data
{
	int element_id = 0;
	std::string name = "";
	unsigned long long cost901 = 0;
	unsigned long long cost902 = 0;
	unsigned long long cost903 = 0;
	float factor = 1.0;

	void Reset()
	{
		*this = vars_data();
	}
};

struct vars_requirements_data
{
	int require_id = 0;
	int require_level = 0;

	void Reset()
	{
		*this = vars_requirements_data();
	}
};

struct config_data
{
	int uni = 0;
	unsigned long long game_speed = 2500;
	int resource_multiplier = 1;
	int storage_multiplier = 1;
	int metal_basic_income = 20;
	int crystal_basic_income = 10;
	int deuterium_basic_income = 0;
	float max_overflow = 1.0;
	int energySpeed = 1;
	int max_galaxy = 9;
	int max_system = 499;
	int max_planet = 15;
	uint8_t max_elements_ships = 10;
	uint64_t max_fleet_per_build = 1000000;
	uint8_t factor_university = 8;
	uint8_t min_build_time = 1;

	config_data()
	{
	}

	void Reset()
	{
		*this = config_data();
	}
};

struct combat_caps_data
{
	double attack = 0.0;
	double shield = 0.0;
};

struct bonus_data
{
	double value = 0.0;
	int unit = 0;
};

struct pricelist_data
{
	std::map<int, double> cost = {};
	double factor = 1.0;
	int max = 0;
	double consumption = 0.0;
	double consumption2 = 0.0;
	double speed = 0.0;
	double speed2 = 0.0;
	double capacity = 0.0;
	int tech = 0;
	double time = 0.0;
	std::map<std::string_view, bonus_data> bonus = {};
};

struct prodgrid_data
{
	std::map<int, std::string> production = {};
	std::map<int, std::string> storage = {};
};

struct reslist_data
{
	std::vector<int> prod = {};
	std::vector<int> storage = {};
	std::vector<int> bonus = {};
	std::vector<int> one = {};
	std::vector<int> build = {};
	std::vector<int> tech = {};
	std::vector<int> fleet = {};
	std::vector<int> defense = {};
	std::vector<int> missile = {};
	std::vector<int> officers = {};
	std::vector<int> dmfunc = {};
	std::map<int, std::vector<int>> allow = {};
	std::vector<int> ressources = {};
	std::map<int, std::vector<int>> resstype = {};
};

struct settlement_data
{
	uint8_t galaxy = 0;
	uint16_t system = 0;
	uint8_t planet = 0;
	uint8_t universe = 0;
	void Reset()
	{
		*this = settlement_data();
	}
};

static std::chrono::steady_clock::time_point GetTimeNow()
{
	return std::chrono::steady_clock::now();
}

static int64_t GetElapsedMilliseconds(
	const std::chrono::steady_clock::time_point& start,
	const std::chrono::steady_clock::time_point& end)
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		end - start)
		.count();
}

static int64_t GetElapsedMicroseconds(
	const std::chrono::steady_clock::time_point& start,
	const std::chrono::steady_clock::time_point& end)
{
	return std::chrono::duration_cast<std::chrono::microseconds>(
		end - start)
		.count();
}