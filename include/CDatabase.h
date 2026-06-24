#pragma once
#include "types.h"
#include "table_users.h"
#include "bot_names.h"

#include <mysql/mysql.h>
#include <string>

class CLanguage;

class CDatabase
{
private:
	std::unordered_map<std::string, std::string>* lang_;
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

	vars_umap vars_;
	vars_requirements_umap vars_requirements_;
	resource_umap resource_;
	combatcaps_umap combatcaps_;
	pricelist_umap pricelist_;
	prodgrid_umap prodgrid_;
	config_umap config_;
	reslist_data reslist_;

	std::vector<table_users> temp_bots_;
	std::vector<settlement_data> settlement_data_;

public:
	CDatabase(CLanguage* language);
	~CDatabase();
	void Init();

	bool Connect();
	void Disconnect();
	bool LoadBots();
	bool LoadVars();
	bool LoadVarsRequirements();
	bool LoadConfig();
	bool LoadSettlementData();
	bool RefreshData();

	bool UpdateBots();
	bool RemoveBots();

	inline vars_umap* GetVars()
	{
		return &vars_;
	}

	inline const std::unordered_map<int,
		std::vector<vars_requirements_data>>*
	GetVarsRequirements() const
	{
		return &vars_requirements_;
	}

	inline const resource_umap& GetResource() const
	{
		return resource_;
	}

	inline const combatcaps_umap& GetCombatCaps() const
	{
		return combatcaps_;
	}

	inline const pricelist_umap& GetPriceList() const
	{
		return pricelist_;
	}

	inline const prodgrid_umap& GetProdGrid() const
	{
		return prodgrid_;
	}

	inline const config_umap& GetConfig() const
	{
		return config_;
	}

	inline const reslist_data& GetReslist() const
	{
		return reslist_;
	}

	inline std::vector<settlement_data>* GetSettlementData()
	{
		return &settlement_data_;
	}

	inline const int GetLoopTime() const
	{
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

	std::vector<table_users>* GetLoadedBots()
	{
		return &temp_bots_;
	}

	// create bots related
	bool AddBots(std::vector<create_info>& bots);

	MYSQL* GetConnection() const { return conn_; }
};