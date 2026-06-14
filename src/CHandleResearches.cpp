// implementation of CBotManager.h

#include "CBotManager.h"

void CBotManager::HandleResearches(table_users& bot,
    table_planets& planet,
    const std::unordered_map<int, table_vars>& vars,
    const std::unordered_map<int, std::vector<table_vars_requirements>>& vars_requirements,
    const uint64_t game_speed)
{
    if (IsResearching(bot))
    {
        m_log.type = 11;
        m_vecLog.push_back(m_log);
        return;
    }

    // have lab ?
    if (planet.resource[31] == 0)
    {
        m_log.type = 12;
        m_vecLog.push_back(m_log);
        return;
    }

    // tech
    int current_levels_tech[200] = { 0 };
    std::copy(bot.resource + 100, bot.resource + 200, current_levels_tech + 100);

    // try research
    int tar_research_id = GetTargetResearchID(current_levels_tech);
    if (tar_research_id == -1)
    {
        m_log.type = 13;
        m_vecLog.push_back(m_log);
        return;
    }

    // search id in map
    auto it = vars.find(tar_research_id);
    if (it == vars.end())
    {
        m_log.type = 14;
        m_log.research_id = tar_research_id;
        m_vecLog.push_back(m_log);
        return;
    }

    // if tech accessible ? check
    if (!IsTechAccessible(tar_research_id, vars_requirements, planet, bot))
    {
        m_log.type = 15;
        m_log.research_id = tar_research_id;
        m_vecLog.push_back(m_log);
        return;
    }

    const table_vars& element = it->second;
    int current_level = current_levels_tech[tar_research_id];
    int level_up = current_level + 1;

    double array_cost[3] = { 0,0,0 };
    array_cost[0] = std::round(element.cost901 * std::pow(element.factor, current_level));
    array_cost[1] = std::round(element.cost902 * std::pow(element.factor, current_level));
    array_cost[2] = std::round(element.cost903 * std::pow(element.factor, current_level));

    if (!HaveEnoughResources(planet, array_cost))
    {

        m_log.type = 16;
        m_log.galaxy = planet.galaxy;
        m_log.system = planet.system;
        m_log.planet = planet.planet;
        m_log.email = bot.email;
        m_log.research_id = tar_research_id;
        m_log.cost901 = array_cost[0];
        m_log.cost902 = array_cost[1];
        m_log.cost903 = array_cost[2];
        m_log.planet_metal = planet.metal;
        m_log.planet_crystal = planet.crystal;
        m_log.planet_deu = planet.deuterium;

        m_vecLog.push_back(m_log);
        return;
    }

    RemoveCostFromPlanet(planet, array_cost);

    double buildTime = ((array_cost[0] + array_cost[1] + 3.0) / (1000.0 * level_up)) / game_speed * (1.0 + planet.resource[31]);
    time_t endTime = m_sysTime + static_cast<time_t>(buildTime);

    bot.b_tech_planet = planet.id;
    bot.b_tech = endTime;
    bot.b_tech_id = element.element_id;

    // todo : check this order
    bot.b_tech_queue = "a:1:{i:0;a:5:{i:0;i:" + std::to_string(element.element_id) +
        ";i:1;i:" + std::to_string(level_up) +
        ";i:2;i:" + std::to_string(static_cast<int>(buildTime)) +
        ";i:3;i:" + std::to_string(endTime) +
        ";i:4;i:" + std::to_string(planet.id) + ";}}";


    m_log.type = 17;
    m_log.research_name = element.name;
    m_log.research_level = level_up;
    m_vecLog.push_back(m_log);


    // update it
    bot.need_update = true; // research q
    planet.need_update = true; // removecost 
}