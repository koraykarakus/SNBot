#include "CBotManager.h"
#include "CLogger.h"
#include "CDatabase.h"
#include "CLoader.h"

extern bool g_bRunning;

CBotManager::CBotManager()
    : m_vecBots{}
    , m_bFirstRun(true)
    , m_timeLastRun(0)
{
    
}

CBotManager::~CBotManager()
{
	m_vecBots.clear();
}

bool CBotManager::IsPlayingNow(const play_time& bot_info)
{
    time_t timeNow = std::time(nullptr);
    std::tm* pLocalTime = std::localtime(&timeNow);
    if (!pLocalTime) return false;
    int hour = pLocalTime->tm_hour;

    if (bot_info.play_start_time_1 != -1
        && bot_info.play_end_time_1 != -1)
    {
        if (hour >= bot_info.play_start_time_1
            && hour < bot_info.play_end_time_1)
        {
            return true;
        }
    }

    if (bot_info.play_start_time_2 != -1
        && bot_info.play_end_time_2 != -1)
    {
        if (hour >= bot_info.play_start_time_2
            && hour < bot_info.play_end_time_2)
        {
            return true;
        }
    }

    if (bot_info.play_start_time_3 != -1
        && bot_info.play_end_time_3 != -1)
    {
        if (hour >= bot_info.play_start_time_3
            && hour < bot_info.play_end_time_3)
        {
            return true;
        }
    }

    if (bot_info.play_start_time_4 != -1
        && bot_info.play_end_time_4 != -1)
    {
        if (hour >= bot_info.play_start_time_4
            && hour < bot_info.play_end_time_4)
        {
            return true;
        }
    }

    return false;
}

void CBotManager::Run()
{
    // sleep if not loaded yet.
    while (g_bRunning 
        && !g_bLoaded)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // end thread if not running
	if (!g_bRunning)
	{
		return;
	}

    CLogger::Info("[CBotManager] - [Run] Bot Main thread starting. vars size : {}\n", m_mapVars.size());

    // main loop as long as it is running
    while (g_bRunning)
    {
        time_t timeNow = std::time(nullptr);

        // time check
		if (!m_bFirstRun
			&& (timeNow < m_timeLastRun + static_cast<time_t>(wait_time)))
		{
			// sleep shortly to avoid overuse of CPU
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}

        // handlers.
        HandleResourceUpdate();
        HandleBuildings();
        // save to db
        g_pDatabase->UpdateBots();
        // reload from db
        g_pDatabase->LoadBots();
        CLogger::Info("Bot buildings has been handled.\n");

        // update time and firstrun flag
        m_bFirstRun = false;
        m_timeLastRun = timeNow;

        // sleep
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    CLogger::Info("Bot Run thread finished.\n");
}

void CBotManager::HandleResourceUpdate() 
{
    time_t timeNow = std::time(nullptr);
    for (auto& bot: m_vecBots)
    {
        if (!IsPlayingNow(bot.playTime))
        {
            continue;
        }

        if (bot.vacation_mode == 1)
        {
            CLogger::Info("Bot is in vacation mode id: {}\n", bot.id);
            continue;
        }

        // loop planets of the bot..
        for (auto& planet : bot.vecPlanets)
        {
            if (planet.b_building != 0)
            {
                BuildingQueue(planet);
            }

            if (bot.b_tech != 0 
                && bot.b_tech < std::time(nullptr))
            {
                ResearchQueue(bot);
            }

            UpdateResource(planet, bot);
            
        }
    }

}

bool CBotManager::BuildingQueue(table_planets& planet)
{
    time_t currentTime = std::time(nullptr);

    if (planet.b_building_id.empty()
        || planet.b_building > currentTime)
    {
        CLogger::Info("CheckPlanetBuildingQueue : not time\n");
        return false;
    }

    PhpArray current_queue = php_unserialize(planet.b_building_id);

    int element = std::stoi(current_queue[0]);

    int build_end_time = std::stoi(current_queue[3]);
    std::string build_mode = current_queue[4];
    
    if (build_mode == "build")
    {
        if (element > 0 
            && element < 1000)
        {
            planet.field_current += 1;
            planet.resource[element] += 1;
        }
        //planet.builded[$element] += 1;
    }
    else if(build_mode == "destruct")
    {
        if (element > 0
            && element < 1000)
        {
            planet.field_current -= 1;
            planet.resource[element] -= 1;
            CLogger::Info("CheckPlanetBuildingQueue : destruct\n");
        }
        //planet.builded[$element] -= 1;
    }

    planet.b_building = 0;
    planet.b_building_id = "";

    return true;
    
    /* multi queue not really needed
    array_shift($current_queue);
    $on_hash = in_array($element, $RESLIST['prod']);
    $this->UpdateResource($build_end_time, !$on_hash);

    if (current_queue.size() == 0)
    {
        planet.b_building = 0;
        planet.b_building_id = "";
        return false;
    }
    else
    {
        planet.b_building_id = php_serialize(current_queue);
        return true;
    }
    */
}

bool CBotManager::ResearchQueue(table_users& user) 
{
    // todo : store time in member
    time_t timeNow = std::time(nullptr);
    
    if (user.b_tech_id == 0 
        || (user.b_tech > timeNow 
        && !user.b_tech_queue.empty()))
    {
        return false;
    }

    user.resource[user.b_tech_id] += 1;
    user.b_tech = 0;
    user.b_tech_id = 0;
    user.b_tech_planet = 0;
    user.b_tech_queue = "";
    return true;
}

void CBotManager::UpdateResource(table_planets& planet, table_users& user)
{
    time_t timeNow = std::time(nullptr);
    time_t production_time = timeNow - planet.last_update;

    if (production_time > 0)
    {
        planet.last_update = timeNow;
        /*
        if ($hash == false)
        {
            $this->ReBuildCache();
        }
        else
        {
            $this->hash = $this->CreateHash();

            if ($this->planet['eco_hash'] != $this->hash)
            {
                $this->planet['eco_hash'] = $this->hash;
                $this->ReBuildCache();
            }
        }
        */
        UpdateCache(planet, user);
        ExecCalc(planet, production_time);
    }
}

void CBotManager::UpdateCache(table_planets& planet, table_users& user)
{
    const table_config* pConfig = GetConfigByUniID(planet.universe);

    if (pConfig == nullptr)
    {
        CLogger::Info("ExecCalc - Config for bot not found !");
        return;
    }

    int metal_basic_income = pConfig->metal_basic_income;
    int crystal_basic_income = pConfig->crystal_basic_income;
    int deuterium_basic_income = pConfig->deuterium_basic_income;
    if (planet.planet_type == 3)
    {
        metal_basic_income = 0;
        crystal_basic_income = 0;
        deuterium_basic_income = 0;
    }

    std::map<int, std::map<std::string, int>> temp = {
        {901, {{"max", 0}, {"plus", 0}, {"minus", 0}}},
        {902, {{"max", 0}, {"plus", 0}, {"minus", 0}}},
        {903, {{"max", 0}, {"plus", 0}, {"minus", 0}}},
        {911, {{"plus", 0}, {"minus", 0}}} 
    };
    
    int build_temp = planet.temp_max;
    int build_energy = user.resource[113];
    int build_level = 0;

    // prod_id = 22, 23, 24
    for(const auto prod_id : G_RESLIST.storage)
    {
        // id = 901,902,903
        for(const auto id : G_RESLIST.resstype[1])
        {
            build_level = planet.resource[prod_id];

            if (id == 901 && prod_id == 22)
            {
                temp[901]["max"] += round(floor(2.5 * pow(1.8331954764, build_level)) * 5000);
            }
            else if (id == 902 && prod_id == 23)
            {
                temp[902]["max"] += round(floor(2.5 * pow(1.8331954764, build_level)) * 5000);
            }
            else if (id == 903 && prod_id == 24)
            {
                temp[903]["max"] += round(floor(2.5 * pow(1.8331954764, build_level)) * 5000);
            }

        }
    }

    std::vector<int> ress_ids;
    ress_ids.reserve(G_RESLIST.resstype[1].size() + G_RESLIST.resstype[2].size());
    ress_ids.insert(ress_ids.end(), G_RESLIST.resstype[1].begin(), G_RESLIST.resstype[1].end());
    ress_ids.insert(ress_ids.end(), G_RESLIST.resstype[2].begin(), G_RESLIST.resstype[2].end());

    // 1,2,3,4,12,212
    for(const auto prod_id : G_RESLIST.prod)
    {
        int build_level_factor = 0;
        build_level = planet.resource[prod_id];
        int production = 0, consumption = 0;
        if (prod_id == 1)
        {
            build_level_factor = std::stoi(planet.metal_mine_percent);
            production = (30 * build_level * pow((1.1), build_level)) * (0.1 * build_level_factor);
            temp[901]["plus"] += production;
            consumption = -(10 * build_level * pow((1.1), build_level)) * (0.1 * build_level_factor);
            temp[911]["minus"] += consumption;
        }
        else if (prod_id == 2)
        {
            build_level_factor = std::stoi(planet.crystal_mine_percent);
            production = (20 * build_level * pow((1.1), build_level)) * (0.1 * build_level_factor);
            temp[902]["plus"] += production;
            consumption = -(10 * build_level * pow((1.1), build_level)) * (0.1 * build_level_factor);
            temp[911]["minus"] += consumption;
        }
        else if (prod_id == 3)
        {
            build_level_factor = std::stoi(planet.deuterium_synthesizer_percent);
            production = (10 * build_level * pow((1.1), build_level) * (-0.002 * build_temp + 1.28) * (0.1 * build_level_factor));
            temp[903]["plus"] += production;
            consumption = -(30 * build_level * pow((1.1), build_level)) * (0.1 * build_level_factor);
            temp[911]["minus"] += consumption;
        }
        else if (prod_id == 4)
        {
            build_level_factor = std::stoi(planet.solar_plant_percent);
            production = (20 * build_level * pow((1.1), build_level)) * (0.1 * build_level_factor);
            temp[911]["plus"] += production;
        }
        else if (prod_id == 12)
        {
            build_level_factor = std::stoi(planet.fusion_plant_percent);
            production = 30 * build_level * pow(1.05 + (build_energy * 0.01), build_level) * (0.1 * build_level_factor);
            temp[911]["plus"] += production;
            consumption = -(10 * build_level * pow(1.1, build_level) * (0.1 * build_level_factor));
            temp[903]["minus"] += consumption;
        }
        else if (prod_id == 212)
        {
            build_level_factor = std::stoi(planet.solar_satellite_percent);
            production = (((build_temp + 160) / 6)* (0.1 * build_level_factor)* build_level);
            temp[911]["plus"] += production;
        }

        
    }

    planet.metal_max = temp[901]["max"] * pConfig->storage_multiplier * (1 + user.factor["ResourceStorage"]);
    planet.crystal_max = temp[902]["max"] * pConfig->storage_multiplier * (1 + user.factor["ResourceStorage"]);
    planet.deuterium_max = temp[903]["max"] * pConfig->storage_multiplier * (1 + user.factor["ResourceStorage"]);
    
    
    
    planet.energy = std::round(temp[911]["plus"] * pConfig->energySpeed * (1 + user.factor["Energy"]));
    planet.energy_used = temp[911]["minus"] * pConfig->energySpeed;
    if (planet.energy_used == 0)
    {
        planet.metal_perhour = 0;
        planet.crystal_perhour = 0;
        planet.deuterium_perhour = 0;
    }
    else
    {
        double prod_level = GetMin(static_cast<double>(1), planet.energy / std::abs(planet.energy_used));

        planet.metal_perhour = (temp[901]["plus"] * (1 + user.factor["Resource"] + 0.02 * user.resource[131]) * prod_level + temp[901]["minus"]) * pConfig->resource_multiplier;
        planet.crystal_perhour = (temp[902]["plus"] * (1 + user.factor["Resource"] + 0.02 * user.resource[132]) * prod_level + temp[902]["minus"]) * pConfig->resource_multiplier;
        planet.deuterium_perhour = (temp[903]["plus"] * (1 + user.factor["Resource"] + 0.02 * user.resource[133]) * prod_level + temp[903]["minus"]) * pConfig->resource_multiplier;
    }
}

void CBotManager::ExecCalc(table_planets& planet, time_t production_time)
{
    if (planet.planet_type == 3)
    {
        return;
    }

    const table_config* pConfig = GetConfigByUniID(planet.universe);

    if (pConfig == nullptr)
    {
        CLogger::Info("ExecCalc - Config for bot not found !");
        return;
    }

    // todo: double lose precision but this comes through steemnova database
    double max_metal_storage = planet.metal_max * pConfig->max_overflow;
    double max_crystal_storage = planet.crystal_max * pConfig->max_overflow;
    double max_deu_storage = planet.deuterium_max * pConfig->max_overflow;

    double metal_theoretical = production_time * ((pConfig->metal_basic_income * pConfig->resource_multiplier) + planet.metal_perhour) / 3600;

    if (metal_theoretical < 0)
    {
        planet.metal = GetMax(planet.metal + metal_theoretical, static_cast<double>(0));
    }
    else if(planet.metal <= max_metal_storage)
    {
        planet.metal = GetMin(planet.metal + metal_theoretical, max_metal_storage);
    }

    double crystal_theoretical = production_time * ((pConfig->crystal_basic_income * pConfig->resource_multiplier) + planet.crystal_perhour) / 3600;
    if (crystal_theoretical < 0)
    {
        planet.crystal = GetMax(planet.crystal + crystal_theoretical, static_cast<double>(0));
    }
    else if(planet.crystal <= max_crystal_storage)
    {
        planet.crystal = GetMin(planet.crystal + crystal_theoretical, max_crystal_storage);
    }

    double deu_theoretical = production_time * ((pConfig->deuterium_basic_income * pConfig->resource_multiplier) + planet.deuterium_perhour) / 3600;
    if (deu_theoretical < 0)
    {
        planet.deuterium = GetMax(planet.deuterium + deu_theoretical, static_cast<double>(0));
    }
    else if(planet.deuterium <= max_deu_storage)
    {
        planet.deuterium = GetMin(planet.deuterium + deu_theoretical, max_deu_storage);
    }

    planet.metal = GetMax(planet.metal, static_cast<double>(0));
    planet.crystal = GetMax(planet.crystal, static_cast<double>(0));
    planet.deuterium = GetMax(planet.deuterium, static_cast<double>(0));
}

void CBotManager::HandleBuildings()
{
    // same as vars table's ids, increased by one to make it more understandable.
    
    const std::vector<int> basic_buildings
    {
        4, 1, 2, 4, 1, 1, 4, 1, 1, 4, 2, 2, 3, 4, 1, 1, 4, 4, 2, 2,
        3, 4, 3, 4, 1, 1, 4, 2, 2, 3, 3, 4, 4, 1, 1, 22, 23, 24,
        22, 23, 4, 14, 14, 14, 14, 14, 14, 2, 2, 3, 3, 4, 1, 1,
        2, 2, 4, 3, 3, 22, 22, 23, 24, 22, 31, 31, 31, 31, 31, 21,
        21, 21, 21, 21, 1, 2, 3, 4, 1, 2, 3, 4, 22, 23, 24, 1, 2, 3,
        14, 21, 21, 14, 14, 22, 23, 24, 1, 2, 3, 1, 4, 4, 4, 24,
        31, 31, 31, 21, 21, 14, 22, 23, 24, 23, 24,
        22, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 1, 1, 2, 2, 2, 22,
        1, 2, 23, 24, 3, 3, 22, 4, 1, 3, 3, 21, 21, 31, 31, 1 
    };

    const std::vector<int> basic_research
    {
        113, 113, 113, 113, 115, 115, 115, 115,
        106, 106, 106, 106, 108, 108, 108, 108, 106, 109,
        109, 109, 109, 110, 110, 110, 110, 111, 111, 111, 111,
        109, 109, 109, 111, 111, 111, 110, 110, 110, 108, 108,
        108, 108, 106, 106, 117, 117, 117, 117, 124, 124, 124,
        124, 124, 124, 124, 120, 120, 120, 120, 120, 121, 121,
        121, 121, 121, 124, 124, 120, 120, 120, 113, 113, 113,
        113, 106, 120, 120, 122, 122, 122, 122, 122, 108, 108,
        114, 114, 114, 114, 114, 114, 114, 114, 115, 115, 115,
        117, 117, 118, 118, 118, 118, 118, 118, 110, 111, 109,
        131, 131, 131, 131, 131, 132, 132, 132, 132, 132, 133,
        133, 133, 133, 133, 109, 110, 111, 131, 131, 131, 132,
        132, 132, 133, 133, 133
    };

    time_t currentTime = std::time(nullptr);
    
    std::vector<stlog> vecLog;

    for (auto& bot : m_vecBots)
    {
        // do not log inside for loop.. just form a vector and log after loop
        stlog log;
        log.bot_id = bot.id;

        // is it online now ?
        if (!IsPlayingNow(bot.playTime))
        {
            log.type = 12;
            vecLog.push_back(log);
            continue;
        }

        const table_config* pConfig = GetConfigByUniID(bot.universe);
        if (pConfig == nullptr)
        {
            log.universe = bot.universe;
            log.type = 1;
            vecLog.push_back(log);
            continue;
        }
        
        // game speed for uni
        unsigned long long game_speed = std::floor(pConfig->game_speed / 2500);
        int current_levels_tech[200] = { 0 };
        // tech
        for (size_t i = 100; i < 200; i++)
        {
            current_levels_tech[i] = bot.resource[i];
        }

        for (auto& planet : bot.vecPlanets)
        {

            log.id_planet = bot.id_planet;

            if (planet.b_building > 0)
            {
                // todo : this ideally should never happen, same for tech
                // consider removing
                if ((planet.b_building > 0
                    && planet.b_building_id.empty())
                    || (planet.b_building == 0
                    && !planet.b_building_id.empty()))
                {
                    planet.b_building = 0;
                    planet.b_building_id = "";
                    
                    log.type = 2;
                    vecLog.push_back(log);
                }

                log.type = 3;
                vecLog.push_back(log);
                continue;
            }

            

            // buildings
            int current_levels_buildings[100] = { 0 };
            for (size_t i = 0; i < 100; i++)
            {
                current_levels_buildings[i] = planet.resource[i];
            }

            

            if (!IsResearching(bot))
            {
                // try research
                int tar_research_id = GetTargetBuildID(basic_research, current_levels_tech);
                if (tar_research_id == -1)
                {
                    log.type = 4;
                    vecLog.push_back(log);
                }
                else
                {
                    // search id in map
                    auto it = m_mapVars.find(tar_research_id);
                    if (it == m_mapVars.end())
                    {
                        log.type = 5;
                        log.research_id = tar_research_id;
                        vecLog.push_back(log);
                    }
                    else
                    {
                        const table_vars& varItem = it->second;
                        int current_level = current_levels_tech[tar_research_id];
                        int level_up = current_level + 1;

                        double array_cost[3] = { 0,0,0 };
                        array_cost[0] = std::round(varItem.cost901 * std::pow(varItem.factor, current_level));
                        array_cost[1] = std::round(varItem.cost902 * std::pow(varItem.factor, current_level));
                        array_cost[2] = std::round(varItem.cost903 * std::pow(varItem.factor, current_level));

                        if (!HaveEnoughResources(planet, array_cost))
                        {
                            log.type = 6;
                            log.galaxy = planet.galaxy;
                            log.system = planet.system;
                            log.planet = planet.planet;
                            log.email = bot.email;
                            log.research_id = tar_research_id;
                            log.cost901 = array_cost[0];
                            log.cost902 = array_cost[1];
                            log.cost903 = array_cost[2];
                            log.planet_metal = planet.metal;
                            log.planet_crystal = planet.crystal;
                            log.planet_deu = planet.deuterium;
                            
                            vecLog.push_back(log);
                        }
                        else
                        {
                            RemoveCostFromPlanet(planet, array_cost);
                            
                            double buildTime = ((array_cost[0] + array_cost[1] + 3.0) / (1000.0 * level_up)) / game_speed * (1.0 + planet.resource[31]);
                            time_t endTime = currentTime + static_cast<time_t>(buildTime);

                            bot.b_tech_planet = planet.id;
                            bot.b_tech = endTime;
                            bot.b_tech_id = varItem.element_id;

                            // todo : check this order
                            bot.b_tech_queue = "a:1:{i:0;a:5:{i:0;i:" + std::to_string(varItem.element_id) +
                                ";i:1;i:" + std::to_string(level_up) +
                                ";i:2;i:" + std::to_string(static_cast<int>(buildTime)) +
                                ";i:3;i:" + std::to_string(endTime) +
                                ";i:4;i:" + std::to_string(planet.id) + ";}}";

                            log.type = 7;
                            log.research_name = varItem.name;
                            log.research_level = level_up;
                            vecLog.push_back(log);
                        }

                        

                    }
                }
            }
                        

            int tar_building_id = GetTargetBuildID(basic_buildings, current_levels_buildings);

            if (tar_building_id == -1)
            {
                log.type = 8;
                vecLog.push_back(log);
                continue;
            }

            // search id in map
            auto it = m_mapVars.find(tar_building_id);
            if (it == m_mapVars.end())
            {
                log.type = 9;
                log.building_id = tar_building_id;
                vecLog.push_back(log);
                continue;
            }

            const table_vars& varItem = it->second;
            int current_level = current_levels_buildings[tar_building_id];
            int level_up = current_level + 1;

            double array_cost[3] = {0,0,0};

            array_cost[0] = std::round(varItem.cost901 * std::pow(varItem.factor, current_level));
            array_cost[1] = std::round(varItem.cost902 * std::pow(varItem.factor, current_level));
            array_cost[2] = std::round(varItem.cost903 * std::pow(varItem.factor, current_level));

            if (!HaveEnoughResources(planet, array_cost))
            {
                log.type = 10;
                log.galaxy = planet.galaxy;
                log.system = planet.system;
                log.planet = planet.planet;
                log.email = bot.email;
                log.building_id = tar_building_id;
                log.cost901 = array_cost[0];
                log.cost902 = array_cost[1];
                log.cost903 = array_cost[2];
                log.planet_metal = planet.metal;
                log.planet_crystal = planet.crystal;
                log.planet_deu = planet.deuterium;
                vecLog.push_back(log);
                
                continue;
            }

            RemoveCostFromPlanet(planet, array_cost);

            // it is building if id is less than 100
            if (tar_building_id < 100)
            {
                double baseTime = (array_cost[0] + array_cost[1] + 3.0) / (game_speed * (1.0 + planet.resource[14]));
                double buildTime = baseTime * std::pow(0.5, planet.resource[15]) * varItem.factor;
                time_t endTime = currentTime + static_cast<time_t>(buildTime);
                planet.b_building = endTime;

                planet.b_building_id = "a:1:{i:0;a:5:{i:0;i:" + std::to_string(varItem.element_id) +
                    ";i:1;i:" + std::to_string(level_up) +
                    ";i:2;i:" + std::to_string(static_cast<int>(buildTime)) +
                    ";i:3;i:" + std::to_string(endTime) + ";i:4;s:5:\"build\";}}";

                log.type = 11;
                log.building_name = varItem.name;
                log.building_level = level_up;
                vecLog.push_back(log);
			}
			
        }
    }

    LogResult(vecLog);
}

void CBotManager::LogResult(const std::vector<stlog>& logs) 
{
    fmt::memory_buffer buf;
    std::string strMsg;
    // Bu döngü sadece RAM içinde string birleştirir, I/O yapmaz (Çok hızlıdır)
    for (const auto& log : logs) 
    {
		switch (log.type)
		{
		case 1:
            // CLogger::Error("[CBotManager] - Config map missing : uni_id '{}' not found !\n", bot.universe);
            fmt::format_to(std::back_inserter(buf), 
                "[CBotManager] - Config map missing : uni_id '{}' not found !\n", log.universe);
			break;
        case 2:
            // CLogger::Info("[CBotManager] - corrected a bot with buggy data !\n", bot.id, planet.id);
            fmt::format_to(std::back_inserter(buf), 
                "[CBotManager] - corrected a bot with buggy data id:{}planet_id:{}!\n", 
                log.bot_id, log.id_planet);
            break;
        case 3:
            fmt::format_to(std::back_inserter(buf), 
                "[CBotManager] - SKIP [botID: {} - planetID: {}], building already !\n", 
                log.bot_id, log.id_planet);
            break;
        case 4:
            fmt::format_to(std::back_inserter(buf), 
                "[CBotManager] - Research list has been completed for bot: {}\n", log.bot_id);
            break;
        case 5:
            fmt::format_to(std::back_inserter(buf), "[CBotManager] - wrong element id. bot_id : {}!\n", log.bot_id);
            break;
        case 6:           
                fmt::format_to(std::back_inserter(buf), 
                    "[CBotManager] - SKIP [botID:{} - planetID:{}]\n"
                    "coordinates:[{}:{}:{}] - email:[{}] \n"
                    "not enough resources for element id:{} \n"
                    "required: [metal:{}|crystal:{}|deu:{}]\n"
                    "have: [metal:{}|crystal:{}|deu:{}]\n",
                    log.bot_id,
                    log.id_planet,
                    log.galaxy, log.system, log.planet,
                    log.email,
                    log.research_id,
                    log.cost901,
                    log.cost902,
                    log.cost903,
                    log.planet_metal,
                    log.planet_crystal,
                    log.planet_deu);
            break;
        case 7:
            fmt::format_to(std::back_inserter(buf), 
                "[CBotManager] - Bot ID {} [Planet: {}] -> Started Research: {} Level {}\n", log.bot_id,
                log.id_planet, log.research_name, log.research_level);
            break;
            break;
        case 8:
            fmt::format_to(std::back_inserter(buf), 
                "[CBotManager] - building list has been completed for planet: {}\n", 
                log.id_planet);
            break;
        case 9:
            fmt::format_to(std::back_inserter(buf), "[CBotManager] - WRONG ELEMENT ID:{} , NOT FOUND !\n", log.building_id);
            break;
        case 10:
            fmt::format_to(std::back_inserter(buf), 
                "[CBotManager] - SKIP [botID:{} - planetID:{}]\n"
                "coordinates:[{}:{}:{}] - email:[{}] \n"
                "not enough resources for element id:{} \n"
                "required: [metal:{}|crystal:{}|deu:{}]\n"
                "have: [metal:{}|crystal:{}|deu:{}]\n",
                log.bot_id, log.id_planet, log.galaxy, log.system, log.planet,
                log.email, log.building_id, log.cost901, log.cost902, log.cost903,
                log.planet_metal, log.planet_crystal, log.planet_deu);
            break;
        case 11:
            fmt::format_to(std::back_inserter(buf), 
                "[CBotManager] - Bot ID {} [Planet: {}] -> Started Building: {} Level {}\n",
                log.bot_id, log.id_planet, log.building_name, log.building_level);
            break;
        case 12:
            fmt::format_to(std::back_inserter(buf),
                "[CBotManager] - Bot ID {} Bot is not online now !\n",
                log.bot_id);
            break;
		default:
			fmt::format_to(std::back_inserter(buf), "Bot:{}, Planet:{}\n", log.bot_id, log.id_planet);
			break;
		}
    }

    // 5000 botun tüm bilgisini TEK BİR SEFERDE diske/konsola yazar. 
    // I/O işlemi 5000 kez değil, sadece 1 kez çağrılır!
    CLogger::Info("### BUILD LOGS ###\n{}", fmt::to_string(buf));
}

int CBotManager::GetTargetBuildID(const std::vector<int> vecList, const int* arrLevels)
{
    int simulated_levels[200] = { 0 };
    int tar_building_id = -1;
    // scan the list for buildings
    for (size_t m = 0; m < vecList.size(); ++m)
    {
        int element_id = vecList[m];
        simulated_levels[element_id]++;

        if (simulated_levels[element_id] > arrLevels[element_id])
        {
            tar_building_id = element_id;
            break;
        }
    }

    return tar_building_id;
}

bool CBotManager::HaveEnoughResources(const table_planets& planet, double* arrCost)
{
	return planet.metal >= arrCost[0]
		&& planet.crystal >= arrCost[1]
		&& planet.deuterium >= arrCost[2];
}

void CBotManager::RemoveCostFromPlanet(table_planets& planet, double* arrCost) 
{
    planet.metal -= arrCost[0];
    planet.crystal -= arrCost[1];
    planet.deuterium -= arrCost[2];
}

// php helpers
std::string CBotManager::php_serialize(const PhpArray& arr) {
    std::stringstream ss;

    // PHP array başlatma: a:Dizi_Boyutu:{
    ss << "a:" << arr.size() << ":{";

    for (size_t i = 0; i < arr.size(); ++i) {
        // Önce PHP array indeksini yaz (i:0;, i:1; vb.)
        ss << "i:" << i << ";";

        // Değerin sayı olup olmadığını kontrol et
        const std::string& val = arr[i];
        bool is_number = !val.empty() && val.find_first_not_of("0123456789-") == std::string::npos;

        if (is_number) {
            // Sayı ise: i:1718000000;
            ss << "i:" << val << ";";
        }
        else {
            // String ise: s:5:"build";
            ss << "s:" << val.length() << ":\"" << val << "\";";
        }
    }

    ss << "}"; // Kapatma parantezi
    return ss.str();
}

PhpArray CBotManager::php_unserialize(const std::string& serialized_data) {
    PhpArray result_array;
    size_t pos = 0;

    struct BuildQueueItem {
        int index;
        int element_id;
        int level;
        time_t end_time;
        std::string task_type;
    };

    while (pos < serialized_data.length()) {
        size_t sub_array_pos = serialized_data.find("a:5:{", pos);
        if (sub_array_pos == std::string::npos) {
            break;
        }

        pos = sub_array_pos + 5;
        BuildQueueItem item{};
        int fields_parsed = 0;

        for (int i = 0; i < 5; ++i) {
            if (pos >= serialized_data.length()) break;

            if (serialized_data[pos] == 'i') {
                pos = serialized_data.find(';', pos) + 1;
            }

            if (pos >= serialized_data.length()) break;

            if (serialized_data[pos] == 'i') {
                size_t val_start = pos + 2;
                size_t val_end = serialized_data.find(';', val_start);
                long long val = std::stoll(serialized_data.substr(val_start, val_end - val_start));

                if (fields_parsed == 0) item.index = static_cast<int>(val);
                else if (fields_parsed == 1) item.element_id = static_cast<int>(val);
                else if (fields_parsed == 2) item.level = static_cast<int>(val);
                else if (fields_parsed == 3) item.end_time = static_cast<time_t>(val);

                fields_parsed++;
                pos = val_end + 1;
            }
            else if (serialized_data[pos] == 's') {
                size_t len_start = pos + 2;
                size_t len_end = serialized_data.find(':', len_start);
                int str_len = std::stoi(serialized_data.substr(len_start, len_end - len_start));

                size_t str_start = len_end + 2;
                item.task_type = serialized_data.substr(str_start, str_len);

                fields_parsed++;
                pos = str_start + str_len + 2;
            }
        }

        if (fields_parsed == 5) {
            result_array.push_back(std::to_string(item.index));
            result_array.push_back(std::to_string(item.element_id));
            result_array.push_back(std::to_string(item.level));
            result_array.push_back(std::to_string(item.end_time));
            result_array.push_back(item.task_type);
        }
    }

    return result_array;
}