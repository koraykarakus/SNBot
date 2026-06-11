// Implementation of CBotManager
#include "CBotManager.h"
#include "CLogger.h"
#include "CDatabase.h"

void CBotManager::HandleResourceUpdate()
{
    time_t timeNow = std::time(nullptr);
    std::tm* pLocalTime = std::localtime(&timeNow);
    int hour = 0;
    if (pLocalTime != nullptr)
    {
        hour = pLocalTime->tm_hour;
    }

    for (auto& bot : m_vecBots)
    {
        if (!IsPlayingNow(bot.playTime, hour))
        {
            continue;
        }

        if (IsAway(bot, timeNow))
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
                BuildingQueue(planet, timeNow);
            }

            if (bot.b_tech != 0
                && bot.b_tech < timeNow)
            {
                ResearchQueue(bot, timeNow);
                bot.need_update = true;
            }

            UpdateResource(planet, bot, timeNow);

            planet.need_update = true;
        }
    }
}

bool CBotManager::BuildingQueue(table_planets& planet, const time_t currentTime)
{

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
    else if (build_mode == "destruct")
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

bool CBotManager::ResearchQueue(table_users& user, const time_t timeNow)
{
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

void CBotManager::UpdateResource(table_planets& planet, table_users& user, const time_t timeNow)
{
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
    const auto& reslist = m_pDatabase->GetReslist();
    for (const auto prod_id : reslist.storage)
    {
        // id = 901,902,903
        for (const auto id : reslist.resstype.at(1))
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
    ress_ids.reserve(reslist.resstype.at(1).size() + reslist.resstype.at(2).size());
    ress_ids.insert(ress_ids.end(), reslist.resstype.at(1).begin(), reslist.resstype.at(1).end());
    ress_ids.insert(ress_ids.end(), reslist.resstype.at(2).begin(), reslist.resstype.at(2).end());

    // 1,2,3,4,12,212
    for (const auto prod_id : reslist.prod)
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
            production = (((build_temp + 160) / 6) * (0.1 * build_level_factor) * build_level);
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
    else if (planet.metal <= max_metal_storage)
    {
        planet.metal = GetMin(planet.metal + metal_theoretical, max_metal_storage);
    }

    double crystal_theoretical = production_time * ((pConfig->crystal_basic_income * pConfig->resource_multiplier) + planet.crystal_perhour) / 3600;
    if (crystal_theoretical < 0)
    {
        planet.crystal = GetMax(planet.crystal + crystal_theoretical, static_cast<double>(0));
    }
    else if (planet.crystal <= max_crystal_storage)
    {
        planet.crystal = GetMin(planet.crystal + crystal_theoretical, max_crystal_storage);
    }

    double deu_theoretical = production_time * ((pConfig->deuterium_basic_income * pConfig->resource_multiplier) + planet.deuterium_perhour) / 3600;
    if (deu_theoretical < 0)
    {
        planet.deuterium = GetMax(planet.deuterium + deu_theoretical, static_cast<double>(0));
    }
    else if (planet.deuterium <= max_deu_storage)
    {
        planet.deuterium = GetMin(planet.deuterium + deu_theoretical, max_deu_storage);
    }

    planet.metal = GetMax(planet.metal, static_cast<double>(0));
    planet.crystal = GetMax(planet.crystal, static_cast<double>(0));
    planet.deuterium = GetMax(planet.deuterium, static_cast<double>(0));
}



