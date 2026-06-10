#pragma once
#include <vector>
#include <unordered_map>
#include <map>
#include "globals.h"
#include "table_users.h"

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

// struct used to log.
struct stlog
{
	int type;
	int bot_id;
	int id_planet;
	int universe;
	int research_id;
	int research_level;
	int building_id;
	int building_level;
	int galaxy;
	int system;
	int planet;
	int cost901;
	int cost902;
	int cost903;
	int planet_metal;
	int planet_crystal;
	int planet_deu;
	std::string research_name;
	std::string building_name;
	std::string email;
	int away_time;
	stlog() 
		: type(0)
		, bot_id(0)
		, id_planet(0)
		, universe(0)
		, research_id(0)
		, research_level(0)
		, building_id(0)
		, building_level(0)
		, galaxy(0)
		, system(0)
		, planet(0)
		, cost901(0)
		, cost902(0)
		, cost903(0)
		, planet_metal(0)
		, planet_crystal(0)
		, planet_deu(0)
		, away_time(0)
	{}
};

using PhpArray = std::vector<std::string>;

class CBotManager 
{
private:
	static const int wait_time = 1 * 30;
	time_t m_timeLastRun;
	bool m_bFirstRun;
	std::vector<table_users> m_vecBots;
public:
	CBotManager();
	~CBotManager();

	void Run();

	// check if time is in player's daily play duration
	bool IsPlayingNow(const play_time& bot_info, int hour) const;
	// compare last click time to this time, some players check 5min,20min etc.
	bool IsAway(const table_users& bot, time_t timeNow) const;
	inline int GetRemainingAwayTimeInSeconds(const table_users& bot, time_t timeNow) const {
		return (bot.playTime.check_time * 60) - (static_cast<int>(timeNow) - bot.onlinetime);
	}
	bool IsInTimeRange(int current_hour, int start_time, int end_time) const;

	inline void AddBot(const table_users& bot)
	{
		m_vecBots.push_back(bot);
	}

	inline void ClearBots()
	{
		m_vecBots.clear();
	}

	std::vector<table_users>& GetBots()
	{
		return m_vecBots;
	}

	inline table_users* GetBotRef(int botId)
	{
		for (auto& bot : m_vecBots) 
		{
			if (bot.id == botId) 
				return &bot;
		}
		return nullptr;
	}

	// build research, building, fleet or defence
	void HandleBuildings();
	// simulates..
	int GetTargetBuildID(const std::vector<int>& vecList, const int* arrLevels) const;
	bool HaveEnoughResources(const table_planets& planet , double* arrCost);
	void RemoveCostFromPlanet(table_planets& planet, double* arrCost);
	inline bool IsResearching(const table_users& user) const
	{
		return user.b_tech > 0;
	}
	void LogResult(const std::vector<stlog>& logs) const;
	// resource update and its helpers
	void HandleResourceUpdate();
	bool BuildingQueue(table_planets& planet, const time_t currentTime);
	bool ResearchQueue(table_users& user, const time_t timeNow);
	void UpdateResource(table_planets& planet, table_users& user, const time_t timeNow);
	void UpdateCache(table_planets& planet, table_users& user);
	void ExecCalc(table_planets& planet, time_t production_time);


	// php helpers
	PhpArray php_unserialize(const std::string& serialized_data);
	std::string php_serialize(const PhpArray& arr);

};

extern CBotManager* g_pBotManager;