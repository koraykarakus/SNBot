#pragma once
#include "globals.h"
#include "table_users.h"
#include "table_fleets.h"
#include "bot_names.h"

#include <vector>
#include <unordered_map>
#include <map>
#include <queue>
#include <mutex>
#include <string>
#include <set>
#include <tuple>

template <typename T>
const T& GetMax(const T& a, const T& b)
{
	return (a < b) ? b : a;
}

template <typename T>
const T& GetMin(const T& a, const T& b)
{
	return (b < a) ? b : a;
}

// console commands processing
struct cmd_queue
{
	int type = 0;
	int count = 0;
	int universe = 0;
};

// struct used to log.
struct stlog
{
	int type = 0;
	int bot_id = 0;
	int id_planet = 0;
	int universe = 0;
	int research_id = 0;
	int research_level = 0;
	int building_id = 0;
	int building_level = 0;
	int galaxy = 0;
	int system = 0;
	int planet = 0;
	int cost901 = 0;
	int cost902 = 0;
	int cost903 = 0;
	int planet_metal = 0;
	int planet_crystal = 0;
	int planet_deu = 0;
	std::string research_name = "";
	std::string building_name = "";
	std::string email = "";
	int away_time = 0;

	void Reset()
	{
		*this = stlog();
	}
};

class CApplication;
class CDatabase;
class CLanguage;

using PhpArray = std::vector<std::string>;
using time_var = std::chrono::steady_clock::time_point;
class CBotManager
{
private:
	// wait time in seconds between each bot handle overall loops.
	int loop_time_;
	// timestamp
	time_t system_time_;
	// current system hour
	int system_hour_;
	time_var time_last_run_;
	bool first_run_;
	std::vector<table_users>* bots_ptr_;
	std::vector<settlement_data>* settlement_data_ptr_;

	std::unordered_map<std::string, std::string>* lang_;

	CDatabase* database_;
	std::unordered_map<int, table_vars>* vars_ptr_;
	std::unordered_map<int, std::vector<table_vars_requirements>>* vars_requirements_ptr_;
	// logging
	std::vector<stlog> logs_;
	stlog log_;

	std::queue<cmd_queue> commands_;
	std::mutex mutex_command_;

public:
	CBotManager(CLanguage* language, CDatabase* database);
	~CBotManager();
	void Run(const CApplication& app);

	// time related
	inline void SetHour()
	{
		std::tm* local_time = std::localtime(&system_time_);

		if (local_time != nullptr)
		{
			system_hour_ = local_time->tm_hour;
			return;
		}

		system_hour_ = 0;
	}

	inline void SetSystemTime()
	{
		system_time_ = std::time(nullptr);
	}

	// console commands processing, such as add bot remove bot etc.
	inline void PushCmdRequest(cmd_queue& st)
	{
		std::lock_guard<std::mutex> lock(mutex_command_);
		commands_.push(st);
	}

	// command handlers.
	bool ProcessPendingRequests();
	void CreateBots(const cmd_queue& cmd);
	void RemoveBots();

	const table_config* GetConfigByUniID(int uni) const;

	const table_vars* GetVarsByID(int id) const;

	// check if time is in player's daily play duration
	bool IsPlayingNow(const table_users& bot) const;
	// compare last click time to this time, some players check 5min,20min etc.
	bool IsAway(const table_users& bot) const;
	inline bool IsInVacation(const table_users& bot)
	{
		return bot.vacation_mode == 1;
	}
	inline int GetRemainingAwayTimeInSeconds(const table_users& bot) const
	{
		return (bot.playTime.check_time * 60) - (static_cast<int>(system_time_) - bot.onlinetime);
	}
	bool IsInTimeRange(int start_time, int end_time) const;
	// main bot loot, run once. loop once.
	void HandleMain();

	void HandleBuildings(table_users& bot, table_planets& planet,
		const uint64_t game_speed);
	void HandleResearches(table_users& bot,
		table_planets& planet,
		const uint64_t game_speed);
	// simulates..
	int GetTargetBuildID(const int* levels) const;
	int GetTargetResearchID(const int* levels) const;
	bool HaveEnoughResources(const table_planets& planet, double* cost);
	void RemoveCostFromPlanet(table_planets& planet, double* cost);
	bool IsTechAccessible(int element_id,
		const table_planets& planet,
		const table_users& user);
	inline bool IsResearching(const table_users& user) const
	{
		return user.b_tech > 0;
	}
	inline bool HasLaboratory(const table_planets& planet) const
	{
		return planet.resource[31] > 0;
	}
	inline bool IsBuilding(const table_planets& planet) const
	{
		return planet.b_building > 0;
	}
	void LogResult();
	// resource update and its helpers
	void HandleResourceUpdate(table_users& bot, table_planets& planet);
	bool BuildingQueue(table_planets& planet);
	bool ResearchQueue(table_users& user);
	void UpdateResource(table_planets& planet, table_users& user);
	void UpdateCache(table_planets& planet, table_users& user);
	void ExecCalc(table_planets& planet, time_t production_time);
	// colonization handler and its helpers
	void HandleColonization(table_users& bot, const table_config* config);
	bool HaveColonyShip(const table_users& user) const;
	int GetPlanetCountMax(const table_users& user) const;
	inline size_t GetPlanetCount(const table_users& user) const
	{
		return user.all_planets.size();
	}
	bool HaveSpotForNewPlanet(const table_users& user) const;
	int FindFirstPlanetCanColonize(const table_users& user, const table_vars* data_colonyship) const;
	int GetFirstPlanetWithColonyShip(const table_users& user) const;

	// HandleCommands
	void SetName(create_info& st);
	void SetLocation(create_info& st,
		const table_config* config,
		std::set<std::tuple<int, int, int>>& occupied_locations);
	void CryptPassword(std::string& pass);
	void SetEmailStartNum();
	int bot_max_email_num_;
	void SetEmail(create_info& st);
	void SetImage(create_info& st);
	void SetTemp(create_info& st);
	// php helpers
	PhpArray php_unserialize(const std::string& serialized_data);
	std::string php_serialize(const PhpArray& arr);
};