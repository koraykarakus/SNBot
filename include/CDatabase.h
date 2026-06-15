#pragma once

#include <mysql/mysql.h>
#include <string>
#include "globals.h"
#include "table_users.h"

class CApplication;
class CBotManager;
using time_var = std::chrono::steady_clock::time_point;

class CDatabase
{
private:
    static const int BATCH_SIZE = 300;
    MYSQL* conn_;

    std::string db_user_;
    std::string db_pass_;
    std::string db_host_;
    std::string db_name_;
    std::string db_uni_prefix_;
    bool db_ssl_;
    // loop time in seconds
    int loop_time_;
    // last time bots loaded from db
    time_var last_load_time_;
    // reload time of bots
    int reload_time_;

    std::unordered_map<int, table_vars> vars_;
    std::unordered_map<int, std::vector<table_vars_requirements>> vars_requirements_;
    std::unordered_map<int, std::string> resource_;
    std::unordered_map<int, combat_caps> combatcaps_;
    std::unordered_map<int, pricelist_data> pricelist_;
    std::unordered_map<int, prodgrid_data> prodgrid_;
    std::unordered_map<int, table_config> config_;
    reslist_data reslist_;

    std::vector<table_users> temp_bots_;

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
        return vars_;
    }

    inline const std::unordered_map<int, std::vector<table_vars_requirements>>& 
        GetVarsRequirements() const
    {
        return vars_requirements_;
    }

    inline const std::unordered_map<int, std::string>& GetResource() const
    {
        return resource_;
    }

    inline const std::unordered_map<int, combat_caps>& GetCombatCaps() const
    {
        return combatcaps_;
    }

    inline const std::unordered_map<int, pricelist_data>& GetPriceList() const
    {
        return pricelist_;
    }

    inline const std::unordered_map<int, prodgrid_data>& GetProdGrid() const
    {
        return prodgrid_;
    }

    inline const std::unordered_map<int, table_config>& GetConfig() const 
    {
        return config_;
    }

    inline const reslist_data& GetReslist() const
    {
        return reslist_;
    }

    inline const int GetLoopTime() const {
        return loop_time_;
    }

    
    inline table_users* GetBotRef(int botId)
    {
        for (auto& bot : temp_bots_)
        {
            if (bot.id == botId)
                return &bot;
        }
        return nullptr;
    }

    const std::vector<table_users>& GetLoadedBots() const { return temp_bots_; }


    MYSQL* GetConnection() const { return conn_; }
};