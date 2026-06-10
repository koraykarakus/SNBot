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

// trivial.
struct play_time
{
    int play_start_time_1;    // when it starts to play [0 - 23] hour
    int play_end_time_1;      // when it ends playing [0 - 23] hour
    int play_start_time_2;
    int play_end_time_2;
    int play_start_time_3;
    int play_end_time_3;
    int play_start_time_4;
    int play_end_time_4;
    int check_time;         // checks every x minutes
    play_time()
        : play_start_time_1(-1)
        , play_end_time_1(-1)
        , play_start_time_2(-1)
        , play_end_time_2(-1)
        , play_start_time_3(-1)
        , play_end_time_3(-1)
        , play_start_time_4(-1)
        , play_end_time_4(-1)
        , check_time(0)
    {
    }

    void reset() {
        play_start_time_1 = -1;
        play_end_time_1 = -1;
        play_start_time_2 = -1;
        play_end_time_2 = -1;
        play_start_time_3 = -1;
        play_end_time_3 = -1;
        play_start_time_4 = -1;
        play_end_time_4 = -1;
        check_time = 0;
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

    bool is_bot;
    std::vector<table_planets> vecPlanets;
    std::map<std::string,int> factor;
    play_time playTime;
    int onlinetime;

    // update bot only if it needs update
    bool need_update;
    // Constructor (Tüm alanlar güvenli değerlerle başlatılıyor)
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

    void SetFactor(time_t time) {

        for (const auto& element_id : G_RESLIST.bonus)
        {
            std::map<std::string, BonusData>& bonus = G_PRICELIST[element_id].bonus;

            
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

    void SetPlayStyle()
    {
        playTime.reset();
		switch (type)
		{
		case 0:
            playTime.check_time = 5;
            playTime.play_start_time_1 = 8;
            playTime.play_end_time_1 = 15;
            playTime.play_start_time_2 = 21;
            playTime.play_end_time_2 = 23;
			break;
        case 1:
            playTime.check_time = 20;
            playTime.play_start_time_1 = 8;
            playTime.play_end_time_1 = 15;
            playTime.play_start_time_2 = 21;
            playTime.play_end_time_2 = 23;
            break;
        case 2:
            // type 3 : id % 10 = 2
            playTime.check_time = 60;
            playTime.play_start_time_1 = 8;
            playTime.play_end_time_1 = 15;
            playTime.play_start_time_2 = 21;
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
            playTime.play_start_time_2 = 23;
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
};