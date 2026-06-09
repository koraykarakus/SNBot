#pragma once
#include <vector>
#include <unordered_map>
#include <map>
#include "globals.h"
#include "table_users.h"
#include "table_vars.h"
#include "table_config.h"

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

struct stlog
{
	int type = 0;
	int bot_id = 0;
	int planet_id = 0;
	int universe = 0;
};

using PhpArray = std::vector<std::string>;

class CBotManager 
{
private:
	// thinking about it
	enum class bot_type 
	{
		buildings_prio,
		fleets_prio,
		defence_prio,
		research_prio
	};

	static const int wait_time = 1 * 30;
	time_t m_timeLastRun;
	bool m_bFirstRun;

	std::vector<table_users> m_vecBots;
	std::unordered_map<int, table_vars> m_mapVars;
	std::unordered_map<int, table_config> m_mapConfig; // for multiverse
public:
	CBotManager();
	~CBotManager();

	void Run();

	inline void AddBot(const table_users& bot)
	{
		m_vecBots.push_back(bot);
	}

	inline void ClearBots()
	{
		m_vecBots.clear();
	}

	inline void ClearVars()
	{
		m_mapVars.clear();
	}

	inline void ClearConfig()
	{
		m_mapConfig.clear();
	}

	inline void AddVar(int elementID, const table_vars& item)
	{
		m_mapVars[elementID] = item;
	}

	inline void AddConfig(int uni, const table_config& item)
	{
		m_mapConfig[uni] = item;
	}

	const std::vector<table_users>& GetBots() const
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

	const table_config* GetConfigByUniID(int uni) const
	{
		auto it = m_mapConfig.find(uni);
		if (it == m_mapConfig.end())
		{
			return nullptr;
		}
		return &it->second;
	}

	// build research, building, fleet or defence
	void HandleBuildings();
	// simulates..
	int GetTargetBuildID(const std::vector<int> vecList, const int* arrLevels);
	bool HaveEnoughResources(const table_planets& planet , double* arrCost);
	void RemoveCostFromPlanet(table_planets& planet, double* arrCost);
	inline bool IsResearching(const table_users& user) 
	{
		return user.b_tech > 0;
	}
	void LogResult(const std::vector<stlog>& logs);
	// resource update and its helpers
	void HandleResourceUpdate();
	bool BuildingQueue(table_planets& planet);
	bool ResearchQueue(table_users& user);
	void UpdateResource(table_planets& planet, table_users& user);
	void UpdateCache(table_planets& planet, table_users& user);
	void ExecCalc(table_planets& planet, time_t production_time);


	// php helpers
	PhpArray php_unserialize(const std::string& serialized_data);
	std::string php_serialize(const PhpArray& arr);

};

extern CBotManager* g_pBotManager;