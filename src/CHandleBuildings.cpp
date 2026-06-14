// implementation of CBotManager.h
#include "CBotManager.h"
#include "CLogger.h"
#include "CDatabase.h"
#include <cmath>
#include <algorithm>

static constexpr std::array basic_buildings = {
    4, 1, 2, 4, 1, 1, 4, 1, 1, 4, 2, 2, 3, 4, 1, 1, 4, 4, 2, 2,
    3, 4, 3, 4, 1, 1, 4, 2, 2, 3, 3, 4, 4, 1, 1, 22, 23, 24,
    22, 23, 4, 14, 14, 14, 14, 14, 14, 2, 2, 3, 3, 4, 1, 1,
    2, 2, 4, 3, 3, 22, 22, 23, 24, 22, 31, 31, 31, 31, 31, 21,
    21, 21, 21, 21, 1, 2, 3, 4, 1, 2, 3, 4, 22, 23, 24, 1, 2, 3,
    14, 21, 21, 14, 14, 22, 23, 24, 1, 2, 3, 1, 4, 4, 4, 24,
    31, 31, 31, 21, 21, 14, 22, 23, 24, 23, 24,
    22, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 1, 1, 2, 2, 2, 22,
    1, 2, 23, 24, 3, 3, 22, 4, 1, 3, 3, 21, 21, 31, 31, 1, 3, 1
};

static constexpr std::array basic_research = {
    113, 113, 113, 113, 115, 115, 115, 115, 106, 106,
    106, 106, 108, 108, 108, 108, 106, 109, 109, 109,
    109, 110, 110, 110, 110, 111, 111, 111, 111, 109,
    109, 109, 111, 111, 111, 110, 110, 110, 108, 108,
    108, 108, 106, 106, 117, 117, 117, 117, 124, 124,
    124, 124, 124, 124, 124, 120, 120, 120, 120, 120,
    121, 121, 121, 121, 121, 124, 124, 120, 120, 120,
    113, 113, 113, 113, 106, 120, 120, 122, 122, 122,
    122, 122, 108, 108, 114, 114, 114, 114, 114, 114,
    114, 114, 115, 115, 115, 117, 117, 118, 118, 118,
    118, 118, 118, 110, 111, 109, 131, 131, 131, 131,
    131, 132, 132, 132, 132, 132, 133, 133, 133, 133,
    133, 109, 110, 111, 131, 131, 131, 132, 132, 132,
    133, 133, 133
};

void CBotManager::HandleBuildings(table_users& bot,
    table_planets& planet, 
    const std::unordered_map<int, table_vars>& vars,
    const std::unordered_map<int, std::vector<table_vars_requirements>>& vars_requirements,
    const uint64_t game_speed)
{
    if (planet.b_building > 0)
    {
        log_.type = 5;
        logs_.push_back(log_);
        return;
    }

    // buildings
    int current_levels_buildings[100] = { 0 };
    std::copy(planet.resource, planet.resource + 100, current_levels_buildings);
    int tar_building_id = GetTargetBuildID(current_levels_buildings);
    if (tar_building_id == -1)
    {
        // todo, build list has finished, 
        // decide what to do with resource wrt game style
        log_.type = 6;
        logs_.push_back(log_);
        return;
    }

    // search id in unordered map
    auto it = vars.find(tar_building_id);
    if (it == vars.end())
    {
        log_.type = 7;
        log_.building_id = tar_building_id;
        logs_.push_back(log_);
        return;
    }

    // todo , is tech accessible ?
    // if tech accessible ? check
    if (!IsTechAccessible(tar_building_id, vars_requirements, planet, bot))
    {
        log_.type = 8;
        log_.research_id = tar_building_id;
        logs_.push_back(log_);
        return;
    }

    const table_vars& element = it->second;
    int current_level = current_levels_buildings[tar_building_id];
    int level_up = current_level + 1;

    double array_cost[3] = { 0,0,0 };

    array_cost[0] = std::round(element.cost901 * std::pow(element.factor, current_level));
    array_cost[1] = std::round(element.cost902 * std::pow(element.factor, current_level));
    array_cost[2] = std::round(element.cost903 * std::pow(element.factor, current_level));

    if (!HaveEnoughResources(planet, array_cost))
    {
        log_.type = 9;
        log_.galaxy = planet.galaxy;
        log_.system = planet.system;
        log_.planet = planet.planet;
        log_.email = bot.email;
        log_.building_id = tar_building_id;
        log_.cost901 = array_cost[0];
        log_.cost902 = array_cost[1];
        log_.cost903 = array_cost[2];
        log_.planet_metal = planet.metal;
        log_.planet_crystal = planet.crystal;
        log_.planet_deu = planet.deuterium;
        logs_.push_back(log_);
        return;
    }

    RemoveCostFromPlanet(planet, array_cost);
    planet.need_update;
    // it is building if id is less than 100
    double baseTime = (array_cost[0] + array_cost[1] + 3.0) / (game_speed * (1.0 + planet.resource[14]));
    double buildTime = baseTime * std::pow(0.5, planet.resource[15]) * element.factor;
    time_t endTime = system_time_ + static_cast<time_t>(buildTime);
    planet.b_building = endTime;

    planet.b_building_id = "a:1:{i:0;a:5:{i:0;i:" + std::to_string(element.element_id) +
        ";i:1;i:" + std::to_string(level_up) +
        ";i:2;i:" + std::to_string(static_cast<int>(buildTime)) +
        ";i:3;i:" + std::to_string(endTime) + ";i:4;s:5:\"build\";}}";

    log_.type = 10;
    log_.building_name = element.name;
    log_.building_level = level_up;
    logs_.push_back(log_);

    bot.need_update = true;
}

int CBotManager::GetTargetBuildID(const int* arrLevels) const
{
    // todo: reserve last spot for terraformer.

    int simulated_levels[100] = { 0 };
    int tar_building_id = -1;
    // scan the list for buildings
    for (size_t m = 0; m < basic_buildings.size(); ++m)
    {
        int element_id = basic_buildings[m];
        simulated_levels[element_id]++;

        if (simulated_levels[element_id] > arrLevels[element_id])
        {
            tar_building_id = element_id;
            break;
        }
    }

    return tar_building_id;
}

int CBotManager::GetTargetResearchID(const int* arrLevels) const {
    int simulated_levels[200] = { 0 };
    int tar_research_id = -1;
    // scan the list for buildings
    for (size_t m = 0; m < basic_research.size(); ++m)
    {
        int element_id = basic_research[m];
        simulated_levels[element_id]++;

        if (simulated_levels[element_id] > arrLevels[element_id])
        {
            tar_research_id = element_id;
            break;
        }
    }

    return tar_research_id;
}