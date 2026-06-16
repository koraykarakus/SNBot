#pragma once

#include <map>
#include <vector>
#include <string>
#include <chrono>
#include <unordered_map>

struct table_vars {
	int element_id = 0;
	std::string name = "";
	unsigned long long cost901 = 0;
	unsigned long long cost902 = 0;
	unsigned long long cost903 = 0;
	float factor = 1.0;

	void Reset()
	{
		*this = table_vars();
	}

};

struct table_vars_requirements 
{
	int require_id = 0;
	int require_level = 0;

	void Reset() 
	{
		*this = table_vars_requirements();
	}

};

struct table_config
{
	int uni;
	unsigned long long game_speed;
	int resource_multiplier;
	int storage_multiplier;
	int metal_basic_income;
	int crystal_basic_income;
	int deuterium_basic_income;
	float max_overflow;
	int energySpeed;
	int max_galaxy;
	int max_system;
	int max_planet;
	table_config()
		: uni(0)
		, game_speed(2500)
		, resource_multiplier(1)
		, storage_multiplier(1)
		, metal_basic_income(20)
		, crystal_basic_income(10)
		, deuterium_basic_income(0)
		, max_overflow(1.0)
		, energySpeed(1)
		, max_galaxy(9)
		, max_system(499)
		, max_planet(15)
	{
	}

	void Reset()
	{
		*this = table_config();
	}
};

struct combat_caps 
{ 
	double attack; 
	double shield; 
};

struct bonus_data 
{ 
	double value; 
	int unit; 
};

struct pricelist_data
{
	std::map<int, double> cost;
	double factor;
	int max;
	double consumption;
	double consumption2;
	double speed;
	double speed2;
	double capacity;
	int tech;
	double time;
	std::map<std::string_view, bonus_data> bonus;
};

struct prodgrid_data 
{
	std::map<int, std::string> production;
	std::map<int, std::string> storage;
};

struct reslist_data 
{
	std::vector<int> prod;
	std::vector<int> storage;
	std::vector<int> bonus;
	std::vector<int> one;
	std::vector<int> build;
	std::vector<int> tech;
	std::vector<int> fleet;
	std::vector<int> defense;
	std::vector<int> missile;
	std::vector<int> officers;
	std::vector<int> dmfunc;
	std::map<int, std::vector<int>> allow;
	std::vector<int> ressources;
	std::map<int, std::vector<int>> resstype;
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
		end - start).count();
}

static int64_t GetElapsedMicroseconds(
	const std::chrono::steady_clock::time_point& start,
	const std::chrono::steady_clock::time_point& end)
{
	return std::chrono::duration_cast<std::chrono::microseconds>(
		end - start).count();
}