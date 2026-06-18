// Implementation of CBotManager.h
#include "CBotManager.h"

void CBotManager::HandleColonization()
{
	// get colony ship's database info..
	const table_vars* colonyship = GetVarsByID(208);
	if (colonyship == nullptr)
	{
		return;
	}

	for (auto& bot : *bots_ptr_)
	{
		const table_config* config = GetConfigByUniID(bot.universe);
		if (config == nullptr) continue;

		if (!HaveSpotForNewPlanet(bot))
		{
			continue;
		}

		if (!HaveColonyShip(bot))
		{
			// build colony ship and continue..

			int iIndex = FindFirstPlanetCanColonize(bot, colonyship);

			if (iIndex == -1)
			{
				continue;
			}

			bot.vecPlanets[iIndex].metal -= colonyship->cost901;
			bot.vecPlanets[iIndex].crystal -= colonyship->cost902;
			bot.vecPlanets[iIndex].deuterium -= colonyship->cost903;

			// todo: add build list instead of just adding one.
			bot.vecPlanets[iIndex].resource[208] += 1;
			bot.vecPlanets[iIndex].need_update = true;

			continue;
		}

		int index = GetFirstPlanetWithColonyShip(bot);

		if (index == -1)
		{
			continue;
		}

		// update planet
		bot.vecPlanets[index].resource[208] -= 1;
		bot.vecPlanets[index].need_update = true;

		// send fleet
		bot.vecPlanets[index].need_fleet_colony = true;
	}
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
	for (const auto& p : user.vecPlanets)
	{
		if (p.resource[208] > 0)
		{
			return true;
		}
	}
	return false;
}

int CBotManager::FindFirstPlanetCanColonize(const table_users& user, const table_vars* data_colonyship) const
{
	int index = 0;
	for (const auto& p : user.vecPlanets)
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
	for (auto& p : user.vecPlanets)
	{
		if (p.resource[208] > 0)
		{
			return index;
		}
		index++;
	}
	return -1;
}
