// Implementation of CBotManager.h
#include "CBotManager.h"

void CBotManager::HandleColonization() 
{
    // get colony ship's database info..
    const table_vars* pColonyShip = GetVarsByID(208);
    if (pColonyShip == nullptr)
    {
        return;
    }

    for (auto& bot : m_vecBots)
    {
        const table_config* pConfig = GetConfigByUniID(bot.universe);
        if (pConfig == nullptr) continue;

        if (!HaveSpotForNewPlanet(bot))
        {
            continue;
        }

        if (!HaveColonyShip(bot))
        {
            // build colony ship and continue..

            int iIndex = FindFirstPlanetCanColonize(bot, pColonyShip);

            if (iIndex == -1)
            {
                continue;
            }

            bot.vecPlanets[iIndex].metal -= pColonyShip->cost901;
            bot.vecPlanets[iIndex].crystal -= pColonyShip->cost902;
            bot.vecPlanets[iIndex].deuterium -= pColonyShip->cost903;

            // todo: add build list instead of just adding one.
            bot.vecPlanets[iIndex].resource[208] += 1;
            bot.vecPlanets[iIndex].need_update = true;

            continue;
        }

        int iIndex = GetFirstPlanetWithColonyShip(bot);

        if (iIndex == -1)
        {
            continue;
        }

        // update planet
        bot.vecPlanets[iIndex].resource[208] -= 1;
        bot.vecPlanets[iIndex].need_update = true;

        // send fleet
        bot.vecPlanets[iIndex].need_fleet_colony = true;
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

int CBotManager::FindFirstPlanetCanColonize(const table_users& user, const table_vars* pColonyShip) const
{
    int index = 0;
    for (const auto& p : user.vecPlanets)
    {
        if (p.metal >= pColonyShip->cost901
            && p.crystal >= pColonyShip->cost902
            && p.deuterium >= pColonyShip->cost903)
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

