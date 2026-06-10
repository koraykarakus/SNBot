#pragma once

#include <map>
#include <vector>
#include <string>
#include <chrono>
#include <unordered_map>

struct table_vars {
	int element_id;
	std::string name;
	unsigned long long cost901;
	unsigned long long cost902;
	unsigned long long cost903;
	float factor;
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
};

struct CombatCaps 
{ 
	double attack; 
	double shield; 
};

struct BonusData 
{ 
	double value; 
	int unit; 
};

struct PriceListData 
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
	std::map<std::string, BonusData> bonus;
};

struct ProdGridData 
{
	std::map<int, std::string> production;
	std::map<int, std::string> storage;
};

struct ResListData 
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

extern std::map<int, std::string> G_RESOURCE;
extern std::map<int, CombatCaps> G_COMBATCAPS;
extern std::map<int, PriceListData> G_PRICELIST;
extern std::map<int, ProdGridData> G_PRODGRID;
extern std::unordered_map<int, table_vars> G_VARS;
extern std::unordered_map<int, table_config> G_CONFIG; // for multiverse
extern ResListData G_RESLIST;


static const table_config* GetConfigByUniID(int uni)
{
	auto it = G_CONFIG.find(uni);
	if (it == G_CONFIG.end())
	{
		return nullptr;
	}
	return &it->second;
}

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