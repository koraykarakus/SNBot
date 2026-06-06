#pragma once
#include <vector>
#include <unordered_map>
#include "bot_info.h" 
#include "vars.h"

class CBotManager 
{
private:
	static const int wait_time = 1 * 60;
	time_t m_timeLastRun;
	bool m_bFirstRun;

	std::vector<bot_info> m_vecBots;
	std::unordered_map<int, GameVarItem> m_mapGameVars;
public:
	CBotManager();
	~CBotManager();

	void Run();

	inline void AddBot(const bot_info& bot)
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

	inline void AddGameVar(int elementID, const GameVarItem& item)
	{
		m_mapGameVars[elementID] = item;
	}

	const std::vector<bot_info>& GetBots() const
	{
		return m_vecBots;
	}

	inline bot_info* GetBotRef(int botId) 
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

extern CBotManager g_BotManager;