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

inline constexpr int bot_type_num = 10;
struct table_users
{
	int id = 0;
	int type = 0;
	std::string strUserName = "";
	std::string email = "";

	int id_planet = 0;
	uint8_t universe = 0;
	uint8_t galaxy = 0;
	uint16_t system = 0;
	uint8_t planet = 0;

	uint8_t vacation_mode = 0;
	int vacation_until = 0;

	int b_tech_planet = 0;
	int b_tech = 0;
	int b_tech_id = 0;
	std::string b_tech_queue = "";

	// technologies
	uint8_t resource[200] = {0};

	// commanders.
	uint8_t rpg_geologist = 0;
	uint8_t rpg_admiral = 0;
	uint8_t rpg_engineer = 0;
	uint8_t rpg_technocrat = 0;
	uint8_t rpg_espion = 0;
	uint8_t rpg_constructor = 0;
	uint8_t rpg_scientist = 0;
	uint8_t rpg_commander = 0;
	uint8_t rpg_stocker = 0;
	uint8_t rpg_defender = 0;
	uint8_t rpg_destructor = 0;
	uint8_t rpg_general = 0;
	uint8_t rpg_bunker = 0;
	uint8_t rpg_raider = 0;
	uint8_t rpg_emperor = 0;

	uint8_t is_bot = 1;
	std::vector<table_planets> all_planets = {};
	std::map<std::string_view, int> factor = {};
	play_time playTime = {};
	int onlinetime = 0;

	// update bot only if it needs update
	bool need_update = false;
	table_users()
	{
		all_planets.reserve(15);
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
		const pricelist_umap& pricelist)
	{
		for (const auto& element_id : reslist.bonus)
		{
			auto itPrice = pricelist.find(element_id);
			if (itPrice == pricelist.end())
			{
				continue;
			}
			const std::map<std::string_view, bonus_data>& bonus = itPrice->second.bonus;

			if (bonus.empty()) // or bonus.size() == 0
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
					auto itBonus = bonus.find(bonus_key);

					if (itBonus != bonus.end())
					{
						factor[bonus_key] += $element_level * itBonus->second.value;
					}
					else
					{
						// log if needed
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
				playTime.check_time = 0;
				playTime.play_start_time_1 = 8;
				playTime.play_end_time_1 = 23;
				playTime.play_start_time_2 = 23;
				playTime.play_end_time_2 = 5;
				break;
		}
	}

	void Reset()
	{
		*this = table_users();
	}
};