#pragma once
#include <string>
#include <vector>
#include <ctime>
#include <array>
#include <map>
#include <string_view>
#include "table_planets.h"

inline constexpr std::array<std::string_view, 18> bonus_list = {
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

// trivial.
struct play_time
{
	// when it starts to play [0 - 23] hour
	int play_start_time_1 = -1;
	// when it ends playing [0 - 23] hour
	int play_end_time_1 = -1;
	int play_start_time_2 = -1;
	int play_end_time_2 = -1;
	int play_start_time_3 = -1;
	int play_end_time_3 = -1;
	int play_start_time_4 = -1;
	int play_end_time_4 = 0;
	// checks every x minutes
	int check_time;

    void reset() 
    {
        *this = play_time();
    }
};

struct table_users
{
    int id;
    int type;
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

    // commanders.
	int rpg_geologist;
	int rpg_admiral;
	int rpg_engineer;
	int rpg_technocrat;
	int rpg_espion;
	int rpg_constructor;
	int rpg_scientist;
	int rpg_commander;
	int rpg_stocker;
	int rpg_defender;
	int rpg_destructor;
	int rpg_general;
	int rpg_bunker;
	int rpg_raider;
	int rpg_emperor;

    bool is_bot;
    std::vector<table_planets> vecPlanets;
    std::map<std::string_view,int> factor;
    play_time playTime;
    int onlinetime;

    // update bot only if it needs update
    bool need_update;
    table_users()
        : id(0)
        , type(0)
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
        , onlinetime(0)
        , need_update(false)
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

    void SetFactor(const time_t time, 
        const reslist_data& reslist,
        const std::map<int, pricelist_data>& pricelist)
    {
        for (const auto& element_id : reslist.bonus)
        {
            auto itPrice = pricelist.find(element_id);
            if (itPrice == pricelist.end())
            {
                continue; // Bir sonraki element_id'ye atla
            }
            const std::map<std::string_view, bonus_data>& bonus = itPrice->second.bonus;

            if (bonus.empty()) // veya bonus.size() == 0
            {
                continue;
            }
            
            int $element_level = resource[element_id];
            

            bool found = false;
            for (const auto& id : reslist.dmfunc)
            {
                if (id == element_id) 
                {
                    found = true;
                    break;
                }
            }

            if (found)
            {
                if (DMExtra($element_level, time, false, true))
                {
                    continue;
                }

                for (const auto& bonus_key : bonus_list)
                {
                    factor[bonus_key] += bonus.at(bonus_key).value;
                }
            }
            else
            {
                for (const auto& bonus_key : bonus_list)
                {
                    // bonus_key'i haritada arıyoruz (Örn: "metal_production")
                    auto itBonus = bonus.find(bonus_key);

                    if (itBonus != bonus.end())
                    {
                        // Eleman bulundu! Güvenle değerini alıp factor haritasına ekliyoruz.
                        // itBonus->second doğrudan bonus_data struct'ına karşılık gelir.
                        factor[bonus_key] += $element_level * itBonus->second.value;
                    }
                    else
                    {
                        // Eğer veritabanında veya kodda bu bonus_key yoksa oyunun çökmesini engelledik.
                        // İstersen buraya bir log atıp hangi key'in eksik olduğunu görebilirsin:
                        // CLogger::Warn("[CBotManager] - Bonus key '{}' bulunamadi!", bonus_key);
                    }
                }
            }
        }

        
    }

    void SetPlayStyle()
    {
        playTime.reset();
		switch (type)
		{
		case 0:
            playTime.check_time = 5;
            playTime.play_start_time_1 = 8;
            playTime.play_end_time_1 = 15;
            playTime.play_start_time_2 = 20;
            playTime.play_end_time_2 = 23;
			break;
        case 1:
            playTime.check_time = 20;
            playTime.play_start_time_1 = 12;
            playTime.play_end_time_1 = 16;
            playTime.play_start_time_2 = 20;
            playTime.play_end_time_2 = 23;
            break;
        case 2:
            // type 3 : id % 10 = 2
            playTime.check_time = 60;
            playTime.play_start_time_1 = 16;
            playTime.play_end_time_1 = 19;
            playTime.play_start_time_2 = 20;
            playTime.play_end_time_2 = 23;
            break;
        case 3:
            playTime.check_time = 5;
            playTime.play_start_time_1 = 12;
            playTime.play_end_time_1 = 18;
            playTime.play_start_time_2 = 23;
            playTime.play_end_time_2 = 1;
            break;
        case 4:
            // type 5 : id % 10 = 4
            playTime.check_time = 20;
            playTime.play_start_time_1 = 12;
            playTime.play_end_time_1 = 18;
            playTime.play_start_time_2 = 23;
            playTime.play_end_time_2 = 1;
            break;
        case 5:
            // type 6 : id % 10 = 5
            playTime.check_time = 60;
            playTime.play_start_time_1 = 12;
            playTime.play_end_time_1 = 18;
            playTime.play_start_time_2 = 20;
            playTime.play_end_time_2 = 1;
            break;
        case 6:
            playTime.check_time = 5;
            playTime.play_start_time_1 = 13;
            playTime.play_end_time_1 = 19;
            playTime.play_start_time_2 = 0;
            playTime.play_end_time_2 = 4;
            break;
        case 7:
            playTime.check_time = 30;
            playTime.play_start_time_1 = 12;
            playTime.play_end_time_1 = 21;
            break;
        case 8:
            playTime.check_time = 60;
            playTime.play_start_time_1 = 8;
            playTime.play_end_time_1 = 23;
            break;
        case 9:
            playTime.check_time = 120;
            playTime.play_start_time_1 = 8;
            playTime.play_end_time_1 = 4;
            break;
		default:
            playTime.check_time = 5;
            playTime.play_start_time_1 = 8;
            playTime.play_end_time_1 = 15;
            playTime.play_start_time_2 = 21;
            playTime.play_end_time_2 = 23;
			break;
		}
    }

    void Reset() {
        *this = table_users();
    }
};