// implementation of CBotManager.h

#include "CBotManager.h"

void CBotManager::HandleResearches(table_users& bot,
	table_planets& planet,
	const uint64_t game_speed)
{
	if (IsResearching(bot))
	{
		log_.type = log_type::researching_already;
		logs_.push_back(log_);
		return;
	}

	// has lab ?
	if (!HasLaboratory(planet))
	{
		log_.type = log_type::dont_have_lab;
		logs_.push_back(log_);
		return;
	}

	// tech
	int current_levels_tech[200] = {0};
	std::copy(bot.resource + 100, bot.resource + 200, current_levels_tech + 100);

	// try research
	int tar_research_id = GetTargetResearchID(current_levels_tech);
	if (tar_research_id == -1)
	{
		log_.type = log_type::research_list_finished;
		logs_.push_back(log_);
		return;
	}

	// search id in map
	const vars_data* element = GetVarsByID(tar_research_id);
	if (element == nullptr)
	{
		log_.type = log_type::research_elem_not_found;
		log_.research_id = tar_research_id;
		logs_.push_back(log_);
		return;
	}

	// if tech accessible ? check
	if (!IsTechAccessible(tar_research_id, planet, bot))
	{
		log_.type = log_type::tech_not_accessible_research;
		log_.research_id = tar_research_id;
		logs_.push_back(log_);
		return;
	}

	int current_level = current_levels_tech[tar_research_id];
	int level_up = current_level + 1;

	double array_cost[3] = {0, 0, 0};
	array_cost[0] = std::round(element->cost901 * std::pow(element->factor, current_level));
	array_cost[1] = std::round(element->cost902 * std::pow(element->factor, current_level));
	array_cost[2] = std::round(element->cost903 * std::pow(element->factor, current_level));

	if (!HaveEnoughResources(planet, array_cost))
	{
		log_.type = log_type::not_have_enough_resources_research;
		log_.galaxy = planet.galaxy;
		log_.system = planet.system;
		log_.planet = planet.planet;
		log_.email = bot.email;
		log_.research_id = tar_research_id;
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

	double build_time = ((array_cost[0] + array_cost[1] + 3.0) / (1000.0 * level_up)) / game_speed * (1.0 + planet.resource[31]);
	time_t end_time = system_time_ + static_cast<time_t>(build_time);

	bot.b_tech_planet = planet.id;
	bot.b_tech = end_time;
	bot.b_tech_id = element->element_id;

	// todo : check this order
	bot.b_tech_queue = "a:1:{i:0;a:5:{i:0;i:" + std::to_string(element->element_id) + ";i:1;i:" + std::to_string(level_up) + ";i:2;i:" + std::to_string(static_cast<int>(build_time)) + ";i:3;i:" + std::to_string(end_time) + ";i:4;i:" + std::to_string(planet.id) + ";}}";

	log_.type = log_type::research_success;
	log_.research_name = element->name;
	log_.research_level = level_up;
	logs_.push_back(log_);

	// update it
	bot.need_update = true;    // research q
	planet.need_update = true; // removecost
}