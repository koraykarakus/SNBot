#pragma once
#include <string>
#include <vector>
#include <ctime>
#include <array>
#include <map>
#include "table_planets.h"

inline std::array<std::string, 18> bonus_list = {
    "Attack",
    "Defensive",
    "Shield",
    "BuildTime",
    "ResearchTime",
    "ShipTime",
    "DefensiveTime",
    "Resource",
    "Energy",
    "ResourceStorage",
    "ShipStorage",
    "FlyTime",
    "FleetSlots",
    "Planets",
    "SpyPower",
    "Expedition",
    "GateCoolTime",
    "MoreFound",
};

struct table_users
{
    int id;
    std::string strUserName;
    std::string email;

    int id_planet;
    int universe;
    int galaxy;
    int system;
    int planet;

    int vacation_mode;
    int vacation_until;

    int b_tech_planet;
    int b_tech;
    int b_tech_id;
    std::string b_tech_queue;

    // technologies
    uint8_t resource[200];

    bool is_bot;
    std::vector<table_planets> vecPlanets;
    std::map<std::string,int> factor;

    // Constructor (Tüm alanlar güvenli değerlerle başlatılıyor)
    table_users()
        : id(0)
        , strUserName("")
        , email("")
        , id_planet(0)
        , universe(0)
        , galaxy(0)
        , system(0)
        , planet(0)
        , vacation_mode(0)
        , vacation_until(0)
        , b_tech_planet(0)
        , b_tech(0)
        , b_tech_id(0)
        , b_tech_queue("")
        , is_bot(false)
        , resource{0}
        , factor{}
    {
        // init factor..
        for (const auto& bonus : bonus_list) 
        {
            factor[bonus] = 0;
        }
    }

    bool isActiveDMExtra(time_t extra, time_t time) 
    {
        return time - extra <= 0;
    }
    
    bool DMExtra(time_t extra, time_t time, bool is_true, bool is_false)
    {
        return isActiveDMExtra(extra, time) ? is_true : is_false;
    }

    void SetFactor() {

        for (const auto element_id : G_RESLIST.bonus)
        {
            std::map<std::string, BonusData> bonus = G_PRICELIST[element_id].bonus;

            
            int $element_level = resource[element_id];
            

            bool found = false;
            for (const auto& id : G_RESLIST.dmfunc) 
            {
                if (id == element_id) 
                {
                    found = true;
                    break;
                }
            }

            if (found)
            {
                time_t time = std::time(nullptr);
                if (DMExtra($element_level, time, false, true))
                {
                    continue;
                }

                for (const auto& bonus_key : bonus_list)
                {
                    factor[bonus_key] += bonus[bonus_key].value;
                }
            }
            else
            {
                for (const auto& bonus_key : bonus_list)
                {
                    factor[bonus_key] += $element_level * bonus[bonus_key].value;
                }
            }
        }

        
    }
};