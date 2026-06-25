// Implementation of CBotManager.h
#include "CBotManager.h"

void CBotManager::HandleColonization(table_users& bot,
	const config_data* config,
	const vars_data* colonyship_ptr)
{
	if (colonyship_ptr == nullptr) return;
	if (config == nullptr) return;

	if (!HaveSpotForNewPlanet(bot)) return;

	if (!HaveColonyShip(bot))
	{
		// build colony ship and return..

		int iIndex = FindFirstPlanetCanColonize(bot, colonyship_ptr);

		if (iIndex == -1) return;

		bot.all_planets[iIndex].metal -= colonyship_ptr->cost901;
		bot.all_planets[iIndex].crystal -= colonyship_ptr->cost902;
		bot.all_planets[iIndex].deuterium -= colonyship_ptr->cost903;

		// todo: add build list instead of just adding one.
		AddShipsToQueue(bot.all_planets[iIndex], config, colonyship_ptr, 1);
		bot.all_planets[iIndex].need_update = true;

		return;
	}

	int index = GetFirstPlanetWithColonyShip(bot);

	if (index == -1) return;

	// update planet
	bot.all_planets[index].resource[208] -= 1;
	bot.all_planets[index].need_update = true;

	// send fleet
	bot.all_planets[index].need_fleet_colony = true;
}

int CBotManager::GetPlanetCountMax(const table_users& user) const
{
	return 1 + std::ceil(user.resource[124] / 2) + (user.rpg_emperor * 2);
}

bool CBotManager::HaveSpotForNewPlanet(const table_users& user) const
{
	return (GetPlanetCountMax(user) > GetPlanetCount(user));
}

bool CBotManager::HaveColonyShip(const table_users& user) const
{
	for (const auto& p : user.all_planets)
	{
		if (p.resource[208] > 0)
		{
			return true;
		}
	}
	return false;
}

int CBotManager::FindFirstPlanetCanColonize(const table_users& user, const vars_data* data_colonyship) const
{
	int index = 0;
	for (const auto& p : user.all_planets)
	{
		if (p.metal >= data_colonyship->cost901
			&& p.crystal >= data_colonyship->cost902
			&& p.deuterium >= data_colonyship->cost903)
		{
			return index;
		}
		index++;
	}
	return -1;
}

int CBotManager::GetFirstPlanetWithColonyShip(const table_users& user) const
{
	int index = 0;
	for (auto& p : user.all_planets)
	{
		if (p.resource[208] > 0)
		{
			return index;
		}
		index++;
	}
	return -1;
}

void CBotManager::AddShipsToQueue(table_planets& planet, 
	const config_data* config_ptr,
	const vars_data* ship_ptr, 
	const int count)
{
	//if (ship_ptr == nullptr) return; // checked
	//if (config_ptr == nullptr) return; // checked
	if (count <= 0) return;
	if (count > config_ptr->max_fleet_per_build) return;

	// build queue is empty
	if (planet.b_shipyard_id == ""
		&& planet.b_shipyard == 0)
	{
		planet.b_shipyard_id = "a:1:{i:0;a:2:{i:0;i:"
							   + std::to_string(ship_ptr->element_id)
							   + ";i:1;d:" + std::to_string(count)
							   + ";}}";
	}
	// building already, 
	// check queue count and add it to the end of queue
	else
	{
		std::string& str = planet.b_shipyard_id;
		size_t start = str.find("a:");
		size_t end = str.find(":{", start);

		// wrong string
		if (start == std::string::npos
			|| end == std::string::npos)
		{
			return;
		}

		std::string number = str.substr(start + 2, end - (start + 2));
		int queue_count = std::stoi(number);
		if (queue_count >= config_ptr->max_elements_ships) return;
		

		str.replace(start + 2, end - (start + 2), 
			std::to_string(queue_count + 1));
		
		// remove last '}'
		str.pop_back();
		
		str += ("i:"
				+ std::to_string(queue_count)
				+ ";a:2:{i:0;i:"
				+ std::to_string(ship_ptr->element_id)
				+ ";i:1;d:"
				+ std::to_string(count)
				+ ";}}");
		
	}

}