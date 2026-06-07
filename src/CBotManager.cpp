#include "CBotManager.h"
#include "CLogger.h"
#include "CDatabase.h"
#include "CLoader.h"

extern bool g_bRunning;

CBotManager::CBotManager()
{
	// todo: reserve if constant number of boss as user input
	m_vecBots = {};
    m_bFirstRun = true;
    m_timeLastRun = 0;
}

CBotManager::~CBotManager()
{
	m_vecBots.clear();
}

void CBotManager::Run()
{
    // sleep if not loaded yet.
    while (g_bRunning 
        && !g_bLoaded)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // end thread if not running
	if (!g_bRunning)
	{
		return;
	}

    CLogger::Info("[CBotManager] - [Run] Bot Main thread starting. vars size : {}\n", m_mapVars.size());

    // main loop as long as it is running
    while (g_bRunning)
    {
        time_t timeNow = std::time(nullptr);

        // time check
		if (!m_bFirstRun
			&& (timeNow < m_timeLastRun + static_cast<time_t>(wait_time)))
		{
			// sleep shortly to avoid overuse of CPU
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}

        // handlers.
        HandleResourceUpdate();
        HandleBuildings();
        // save to db
        g_pDatabase->UpdateBots();
        // reload from db
        g_pDatabase->LoadBots();
        CLogger::Info("Bot buildings has been handled.\n");

        // update time and firstrun flag
        m_bFirstRun = false;
        m_timeLastRun = timeNow;

        // sleep
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    CLogger::Info("Bot Run thread finished.\n");
}

void CBotManager::HandleResourceUpdate() 
{
    for (auto& bot: m_vecBots)
    {
        if (bot.vacation_mode == 1)
        {
            CLogger::Info("Bot is in vacation mode id: {}\n", bot.id);
            continue;
        }

        // loop planets of the bot..
        for (auto& planet : bot.vecPlanets)
        {
            if (planet.b_building != 0)
            {
                BuildingQueue(planet);
            }

            if (bot.b_tech != 0 
                && bot.b_tech < std::time(nullptr))
            {
                ResearchQueue(bot);
            }

            UpdateResource(planet);
            
        }
    }

}

bool CBotManager::BuildingQueue(table_planets& planet)
{
    time_t currentTime = std::time(nullptr);

    if (planet.b_building_id.empty()
        || planet.b_building > currentTime)
    {
        CLogger::Info("CheckPlanetBuildingQueue : not time\n");
        return false;
    }

    PhpArray current_queue = php_unserialize(planet.b_building_id);

    int element = std::stoi(current_queue[0]);
    CLogger::Info("CheckPlanetBuildingQueue : element {}\n", element);

    int build_end_time = std::stoi(current_queue[3]);
    std::string build_mode = current_queue[4];
    
    if (build_mode == "build")
    {
        if (element > 0 
            && element < 1000)
        {
            planet.field_current += 1;
            planet.resource[element] += 1;
            CLogger::Info("Built element id:{}\n", element);
        }
        //planet.builded[$element] += 1;
    }
    else if(build_mode == "destruct")
    {
        if (element > 0
            && element < 1000)
        {
            planet.field_current -= 1;
            planet.resource[element] -= 1;
            CLogger::Info("CheckPlanetBuildingQueue : destruct\n");
        }
        //planet.builded[$element] -= 1;
    }

    planet.b_building = 0;
    planet.b_building_id = "";

    return true;
    
    /* multi queue not really needed
    array_shift($current_queue);
    $on_hash = in_array($element, $RESLIST['prod']);
    $this->UpdateResource($build_end_time, !$on_hash);

    if (current_queue.size() == 0)
    {
        planet.b_building = 0;
        planet.b_building_id = "";
        return false;
    }
    else
    {
        planet.b_building_id = php_serialize(current_queue);
        return true;
    }
    */
}

bool CBotManager::ResearchQueue(table_users& user) 
{
    // todo : store time in member
    time_t timeNow = std::time(nullptr);
    
    if (user.b_tech_id == 0 
        || (user.b_tech > timeNow 
        && !user.b_tech_queue.empty()))
    {
        return false;
    }

    user.resource[user.b_tech_id] += 1;
    user.b_tech = 0;
    user.b_tech_id = 0;
    user.b_tech_planet = 0;
    user.b_tech_queue = "";
    return true;
}

void CBotManager::UpdateResource(table_planets& planet)
{
    time_t timeNow = std::time(nullptr);
    time_t production_time = timeNow - planet.last_update;

    if (production_time > 0)
    {
        planet.last_update = timeNow;
        /*
        if ($hash == false)
        {
            $this->ReBuildCache();
        }
        else
        {
            $this->hash = $this->CreateHash();

            if ($this->planet['eco_hash'] != $this->hash)
            {
                $this->planet['eco_hash'] = $this->hash;
                $this->ReBuildCache();
            }
        }
        */
        ExecCalc(planet, production_time);
    }
}

void CBotManager::UpdateCache(table_planets& planet) 
{
    // todo
}

void CBotManager::ExecCalc(table_planets& planet, time_t production_time)
{
    if (planet.planet_type == 3)
    {
        return;
    }

    const table_config* pConfig = GetConfigByUniID(planet.universe);

    if (pConfig == nullptr)
    {
        CLogger::Info("ExecCalc - Config for bot not found !");
        return;
    }

    // todo: double lose precision but this comes through steemnova database
    double max_metal_storage = planet.metal_max * pConfig->max_overflow;
    double max_crystal_storage = planet.crystal_max * pConfig->max_overflow;
    double max_deu_storage = planet.deuterium_max * pConfig->max_overflow;

    double metal_theoretical = production_time * ((pConfig->metal_basic_income * pConfig->resource_multiplier) + planet.metal_perhour) / 3600;

    if (metal_theoretical < 0)
    {
        planet.metal = GetMax(planet.metal + metal_theoretical, static_cast<double>(0));
    }
    else if(planet.metal <= max_metal_storage)
    {
        planet.metal = GetMin(planet.metal + metal_theoretical, max_metal_storage);
    }

    double crystal_theoretical = production_time * ((pConfig->crystal_basic_income * pConfig->resource_multiplier) + planet.crystal_perhour) / 3600;
    if (crystal_theoretical < 0)
    {
        planet.crystal = GetMax(planet.crystal + crystal_theoretical, static_cast<double>(0));
    }
    else if(planet.crystal <= max_crystal_storage)
    {
        planet.crystal = GetMin(planet.crystal + crystal_theoretical, max_crystal_storage);
    }

    double deu_theoretical = production_time * ((pConfig->deuterium_basic_income * pConfig->resource_multiplier) + planet.deuterium_perhour) / 3600;
    if (deu_theoretical < 0)
    {
        planet.deuterium = GetMax(planet.deuterium + deu_theoretical, static_cast<double>(0));
    }
    else if(planet.deuterium <= max_deu_storage)
    {
        planet.deuterium = GetMin(planet.deuterium + deu_theoretical, max_deu_storage);
    }

    planet.metal = GetMax(planet.metal, static_cast<double>(0));
    planet.crystal = GetMax(planet.crystal, static_cast<double>(0));
    planet.deuterium = GetMax(planet.deuterium, static_cast<double>(0));
}

void CBotManager::HandleBuildings()
{
    // same as vars table's ids, increased by one to make it more understandable.
    const std::vector<int> building_list_destroyer = {
    4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 1, 2, 4, 1, 2, 3,
    22, 23, 24,
    4, 1, 1, 2, 4, 1, 2, 3, 4, 1, 2, 3,
    22, 23, 24,
    4, 1, 1, 2, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 1, 2, 4, 1, 2, 3,
    4, 1, 2, 3, 4, 1, 1, 2,
    22, 23, 24, 22, 23, 24,
    4, 1, 2, 3, 4, 1, 2, 3,
    6, 6, 31, 31, 31, 1, 1, 2, 3, 8, 8, 9, 10, 14, 14, 15,
    21, 21, 21, 1, 2, 3, 12, 12, 12, 17, 17, 18, 18, 18, 25, 25,
    25, 20, 20, 1, 1, 2, 2, 3, 6, 6, 6, 26, 26, 26, 26, 11,
    11, 19, 19, 19, 32, 33, 34, 32, 33, 32, 33, 34, 32, 33, 32, 33,
    34, 34, 1, 2, 3, 27, 27, 27, 27, 27, 28, 28, 7, 31, 31
    };

    const std::vector<int> building_list_deathstar = {
    4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 1, 2, 4, 1, 2, 3,
    22, 23, 24,
    4, 1, 1, 2, 4, 1, 2, 3, 4, 1, 2, 3,
    22, 23, 24,
    4, 1, 1, 2, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 1, 2, 4, 1, 2, 3,
    4, 1, 2, 3, 4, 1, 1, 2,
    22, 23, 24, 22, 23, 24,
    4, 1, 2, 3, 4, 1, 2, 3,
    1, 1, 2, 2, 3, 3, 23, 23, 23, 12, 12, 12, 12, 12, 8, 8,
    8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11, 14, 14, 14, 17,
    17, 17, 18, 18, 18, 18, 19, 19, 19, 19, 20, 20, 20, 20, 25, 25,
    25, 25, 26, 26, 26, 26, 26, 6, 6, 6, 6, 6, 6, 6, 6, 31,
    31, 31, 31, 31, 31, 31, 31, 31, 15, 15, 15, 15, 21, 21, 21, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 32, 33, 34, 32, 33, 32, 33, 34,
    32, 33, 32, 33, 34, 34, 32, 32, 33, 34, 32, 33, 34, 27, 27, 27,
    27, 27, 28, 28, 7, 35
    };

    time_t currentTime = std::time(nullptr);

    for (auto& bot : m_vecBots)
    {
        const table_config* pConfig = GetConfigByUniID(bot.universe);
        if (pConfig == nullptr)
        {
            CLogger::Error("[CBotManager] - Config map missing : uni_id '{}' not found !\n", bot.universe);
            continue;
        }
        unsigned long long game_speed = std::floor(pConfig->game_speed / 2500);
        // game speed for uni

        for (auto& planet : bot.vecPlanets)
        {
            if (planet.b_building > 0
                || bot.b_tech > 0)
            {
                // todo : this ideally should never happen, same for tech
                // consider removing
                if ((planet.b_building > 0
                    && planet.b_building_id.empty())
                    || (planet.b_building == 0
                    && !planet.b_building_id.empty()))
                {
                    planet.b_building = 0;
                    planet.b_building_id = "";
                    CLogger::Info("[CBotManager] - corrected a bot with buggy data !\n", bot.id, planet.id);
                }
                CLogger::Info("[CBotManager] - SKIP [botID: {} - planetID: {}], building already !\n",bot.id, planet.id);
                continue;
            }

            // Simülasyon için kademeleri eşleştirirken yine elementID'leri doğrudan indeks gibi kullanabiliriz.
            // un1_vars tablosunda element_id'ler maksimum 199 (graviton) olduğu için 200 elemanlık bir dizi açıyoruz.
            int current_levels[200] = { 0 };

            current_levels[1] = planet.resource[1];
            current_levels[2] = planet.resource[2];
            current_levels[3] = planet.resource[3];
            current_levels[4] = planet.resource[4];
            current_levels[12] = planet.resource[12];
            current_levels[14] = planet.resource[14];
            current_levels[15] = planet.resource[15];
            current_levels[21] = planet.resource[21];
            current_levels[22] = planet.resource[22];
            current_levels[23] = planet.resource[23];
            current_levels[24] = planet.resource[24];
            current_levels[31] = planet.resource[31];
            current_levels[33] = planet.resource[33];
            current_levels[34] = planet.resource[6];
            current_levels[36] = planet.resource[34];
            current_levels[44] = planet.resource[44];

            // Teknolojiler (Gerçek ID'leri ile)
            current_levels[106] = bot.resource[106];
            current_levels[108] = bot.resource[108];
            current_levels[109] = bot.resource[109];
            current_levels[110] = bot.resource[110];
            current_levels[111] = bot.resource[111];
            current_levels[113] = bot.resource[113];
            current_levels[114] = bot.resource[114];
            current_levels[115] = bot.resource[115];
            current_levels[116] = bot.resource[116];
            current_levels[117] = bot.resource[117];
            current_levels[120] = bot.resource[120];
            current_levels[121] = bot.resource[121];
            current_levels[122] = bot.resource[122];
            current_levels[123] = bot.resource[123];
            current_levels[124] = bot.resource[124];
            current_levels[131] = bot.resource[131];
            current_levels[132] = bot.resource[132];
            current_levels[133] = bot.resource[133];
            current_levels[199] = bot.resource[199];

            int simulated_levels[200] = { 0 };
            int target_element_id = -1;

            const std::vector<int>* pTargetList = (bot.id % 2 != 0) ? &building_list_destroyer : &building_list_deathstar;
            const std::vector<int>& target_building_list = *pTargetList;

            // Listeyi tarıyoruz (Artık listeden çıkan değer direkt gerçek ID)
            for (size_t m = 0; m < target_building_list.size(); ++m)
            {
                int element_id = target_building_list[m];
                simulated_levels[element_id]++;

                if (simulated_levels[element_id] > current_levels[element_id])
                {
                    target_element_id = element_id;
                    break;
                }
            }

            if (target_element_id == -1)
            {
                CLogger::Info("[CBotManager] - The list to build has been completed for planet: {}\n", planet.name);
                continue;
            }

            // search id in map
            auto it = m_mapVars.find(target_element_id);
            if (it == m_mapVars.end())
            {
                CLogger::Error("[CBotManager] - WRONG ELEMENT ID:{} , NOT FOUND !\n", target_element_id);
                continue;
            }

            const table_vars& varItem = it->second;
            int current_level = current_levels[target_element_id];
            int level_up = current_level + 1;

            double required_metal = std::round(varItem.costMetal * std::pow(varItem.factor, current_level));
            double required_crystal = std::round(varItem.costCrystal * std::pow(varItem.factor, current_level));
            double required_deuterium = std::round(varItem.costDeuterium * std::pow(varItem.factor, current_level));

            if (planet.metal < required_metal 
                || planet.crystal < required_crystal 
                || planet.deuterium < required_deuterium)
            {
                CLogger::Info("[CBotManager] - SKIP [botID:{} - planetID:{}], not enough resources for element id :{}\n", bot.id, planet.id, varItem.elementID);
                continue;
            }

            planet.metal -= required_metal;
            planet.crystal -= required_crystal;
            planet.deuterium -= required_deuterium;

            // it is building if id is less than 100
            if (target_element_id < 100)
            {
                double baseTime = (required_metal + required_crystal + 3.0) / (game_speed * (1.0 + planet.resource[14]));
                double buildTime = baseTime * std::pow(0.5, planet.resource[15]) * varItem.factor;
                time_t endTime = currentTime + static_cast<time_t>(buildTime);
                planet.b_building = endTime;

                planet.b_building_id = "a:1:{i:0;a:5:{i:0;i:" + std::to_string(varItem.elementID) +
                    ";i:1;i:" + std::to_string(level_up) +
                    ";i:2;i:" + std::to_string(static_cast<int>(buildTime)) +
                    ";i:3;i:" + std::to_string(endTime) + ";i:4;s:5:\"build\";}}";

                CLogger::Info("[CBotManager] - Bot ID {} [Planet: {}] -> Started Building: {} Level {}\n", 
                    bot.id, planet.name, varItem.name, level_up);
			}
			// techno
			else
			{
                double buildTime = ((required_metal + required_crystal + 3.0) / (1000.0 * level_up)) / game_speed * (1.0 + planet.resource[31]);
                time_t endTime = currentTime + static_cast<time_t>(buildTime);

                bot.b_tech_planet = planet.id;
                bot.b_tech = endTime;
                bot.b_tech_id = varItem.elementID;

                // todo : check this order
                bot.b_tech_queue = "a:1:{i:0;a:5:{i:0;i:" + std::to_string(varItem.elementID) +
                    ";i:1;i:" + std::to_string(level_up) +
                    ";i:2;i:" + std::to_string(static_cast<int>(buildTime)) +
                    ";i:3;i:" + std::to_string(endTime) +
                    ";i:4;i:" + std::to_string(planet.id) + ";}}";

                CLogger::Info("[CBotManager] - Bot ID {} [Planet: {}] -> Started Research: {} Level {}\n", bot.id, planet.name, varItem.name, level_up);
            }
        }
    }
}


// php helpers
std::string CBotManager::php_serialize(const PhpArray& arr) {
    std::stringstream ss;

    // PHP array başlatma: a:Dizi_Boyutu:{
    ss << "a:" << arr.size() << ":{";

    for (size_t i = 0; i < arr.size(); ++i) {
        // Önce PHP array indeksini yaz (i:0;, i:1; vb.)
        ss << "i:" << i << ";";

        // Değerin sayı olup olmadığını kontrol et
        const std::string& val = arr[i];
        bool is_number = !val.empty() && val.find_first_not_of("0123456789-") == std::string::npos;

        if (is_number) {
            // Sayı ise: i:1718000000;
            ss << "i:" << val << ";";
        }
        else {
            // String ise: s:5:"build";
            ss << "s:" << val.length() << ":\"" << val << "\";";
        }
    }

    ss << "}"; // Kapatma parantezi
    return ss.str();
}

PhpArray CBotManager::php_unserialize(const std::string& serialized_data) {
    PhpArray result_array;
    size_t pos = 0;

    struct BuildQueueItem {
        int index;
        int element_id;
        int level;
        time_t end_time;
        std::string task_type;
    };

    while (pos < serialized_data.length()) {
        size_t sub_array_pos = serialized_data.find("a:5:{", pos);
        if (sub_array_pos == std::string::npos) {
            break;
        }

        pos = sub_array_pos + 5;
        BuildQueueItem item{};
        int fields_parsed = 0;

        for (int i = 0; i < 5; ++i) {
            if (pos >= serialized_data.length()) break;

            if (serialized_data[pos] == 'i') {
                pos = serialized_data.find(';', pos) + 1;
            }

            if (pos >= serialized_data.length()) break;

            if (serialized_data[pos] == 'i') {
                size_t val_start = pos + 2;
                size_t val_end = serialized_data.find(';', val_start);
                long long val = std::stoll(serialized_data.substr(val_start, val_end - val_start));

                if (fields_parsed == 0) item.index = static_cast<int>(val);
                else if (fields_parsed == 1) item.element_id = static_cast<int>(val);
                else if (fields_parsed == 2) item.level = static_cast<int>(val);
                else if (fields_parsed == 3) item.end_time = static_cast<time_t>(val);

                fields_parsed++;
                pos = val_end + 1;
            }
            else if (serialized_data[pos] == 's') {
                size_t len_start = pos + 2;
                size_t len_end = serialized_data.find(':', len_start);
                int str_len = std::stoi(serialized_data.substr(len_start, len_end - len_start));

                size_t str_start = len_end + 2;
                item.task_type = serialized_data.substr(str_start, str_len);

                fields_parsed++;
                pos = str_start + str_len + 2;
            }
        }

        if (fields_parsed == 5) {
            result_array.push_back(std::to_string(item.index));
            result_array.push_back(std::to_string(item.element_id));
            result_array.push_back(std::to_string(item.level));
            result_array.push_back(std::to_string(item.end_time));
            result_array.push_back(item.task_type);
        }
    }

    return result_array;
}