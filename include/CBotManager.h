#pragma once
#include <vector>
#include <unordered_map>
#include "table_users.h"
#include "table_vars.h"
#include "table_config.h"

class CBotManager 
{
private:
	static const int wait_time = 1 * 60;
	time_t m_timeLastRun;
	bool m_bFirstRun;

	std::vector<table_users> m_vecBots;
	std::unordered_map<int, table_vars> m_mapGameVars;
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
		m_mapGameVars.clear();
	}

	inline void ClearConfig()
	{
		m_mapConfig.clear();
	}

	inline void AddGameVar(int elementID, const table_vars& item)
	{
		m_mapGameVars[elementID] = item;
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

	// handlers
	std::string GetElementNameByIndex(int index);
	void HandleBuildings();

};

extern CBotManager* g_pBotManager;