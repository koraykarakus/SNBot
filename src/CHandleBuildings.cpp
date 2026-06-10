// implementation of CBotManager.h
#include "CBotManager.h"
#include "CLogger.h"

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

    std::tm* pLocalTime = std::localtime(&currentTime); 
    int hour = 0;
    if (pLocalTime != nullptr) 
    {
        hour = pLocalTime->tm_hour;
    }

    for (auto& bot : m_vecBots)
    {
        // do not log inside for loop.. just form a vector and log after loop
        stlog log;
        log.bot_id = bot.id;

        // is it online now ?
        if (!IsPlayingNow(bot.playTime, hour))
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
                    auto it = G_VARS.find(tar_research_id);
                    if (it == G_VARS.end())
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
                            // update it
                            bot.need_update = true; // research q
                            planet.need_update = true; // removecost
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
            auto it = G_VARS.find(tar_building_id);
            if (it == G_VARS.end())
            {
                log.type = 9;
                log.building_id = tar_building_id;
                vecLog.push_back(log);
                continue;
            }

            const table_vars& varItem = it->second;
            int current_level = current_levels_buildings[tar_building_id];
            int level_up = current_level + 1;

            double array_cost[3] = { 0,0,0 };

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
            planet.need_update;
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

                bot.need_update = true;
            }

        }
    }
    
    LogResult(vecLog);
}

int CBotManager::GetTargetBuildID(const std::vector<int>& vecList, const int* arrLevels) const
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

void CBotManager::LogResult(const std::vector<stlog>& logs) const
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
