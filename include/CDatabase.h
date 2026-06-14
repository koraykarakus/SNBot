#pragma once

#include <mysql/mysql.h>
#include <string>
#include "globals.h"
#include "table_users.h"

class CApplication;
class CBotManager;

class CDatabase
{
private:
    static const int BATCH_SIZE = 300;
    MYSQL* m_pConn;
    CBotManager* m_pBotManager;

    std::string m_strDBUser;
    std::string m_strDBPass;
    std::string m_strDBHost;
    std::string m_strDBName;
    std::string m_strDBPrefix;

    std::unordered_map<int, table_vars> m_vars;
    std::unordered_map<int, std::vector<table_vars_requirements>> m_vars_requirements;
    std::unordered_map<int, std::string> m_resource;
    std::unordered_map<int, combat_caps> m_combatcaps;
    std::unordered_map<int, pricelist_data> m_pricelist;
    std::unordered_map<int, prodgrid_data> m_prodgrid;
    std::unordered_map<int, table_config> m_config;
    reslist_data m_reslist;

    std::vector<table_users> m_vecTempBots;

public:
    CDatabase();
    ~CDatabase();
    void Init();

    bool Connect();
    void Disconnect();
    bool LoadBots();
    bool LoadVars();
    bool LoadVarsRequirements();
    bool LoadConfig();
    bool UpdateBots(std::vector<table_users>& vecBots);
    bool AddBots(int count);
    bool RemoveBots();

    inline const std::unordered_map<int, table_vars>& GetVars() const
    {
        return m_vars;
    }

    inline const std::unordered_map<int, std::vector<table_vars_requirements>>& 
        GetVarsRequirements() const
    {
        return m_vars_requirements;
    }

    inline const std::unordered_map<int, std::string>& GetResource() const
    {
        return m_resource;
    }

    inline const std::unordered_map<int, combat_caps>& GetCombatCaps() const
    {
        return m_combatcaps;
    }

    inline const std::unordered_map<int, pricelist_data>& GetPriceList() const
    {
        return m_pricelist;
    }

    inline const std::unordered_map<int, prodgrid_data>& GetProdGrid() const
    {
        return m_prodgrid;
    }

    inline const std::unordered_map<int, table_config>& GetConfig() const 
    {
        return m_config;
    }

    inline const reslist_data& GetReslist() const
    {
        return m_reslist;
    }


    inline table_users* GetBotRef(int botId)
    {
        for (auto& bot : m_vecTempBots)
        {
            if (bot.id == botId)
                return &bot;
        }
        return nullptr;
    }

    const std::vector<table_users>& GetLoadedBots() const { return m_vecTempBots; }


    MYSQL* GetConnection() const { return m_pConn; }
};