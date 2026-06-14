#include "CBotManager.h"
#include "CLogger.h"
#include "CDatabase.h"
#include "CApplication.h"
//#include "CLoader.h"

CBotManager::CBotManager()
    : m_vecBots{}
    , m_bFirstRun(true)
    , m_timeLastRun(std::chrono::steady_clock::time_point{})
    , m_pDatabase(nullptr)
{
    
}

CBotManager::~CBotManager()
{
	m_vecBots.clear();
}

bool CBotManager::IsInTimeRange(int current_hour, int start_time, int end_time) const
{
    if (start_time == -1 
        || end_time == -1) 
        return false;

    // normal (exp: 13-23)
    if (start_time < end_time)
    {
        return (current_hour >= start_time && current_hour < end_time);
    }

    // 2: exceed midnight (exp: 23-1 arası)
    return (current_hour >= start_time || current_hour < end_time);
}

bool CBotManager::IsPlayingNow(const play_time& bot_info, int hour) const
{
    if (IsInTimeRange(hour, bot_info.play_start_time_1, bot_info.play_end_time_1)) 
        return true;
    if (IsInTimeRange(hour, bot_info.play_start_time_2, bot_info.play_end_time_2)) 
        return true;
    if (IsInTimeRange(hour, bot_info.play_start_time_3, bot_info.play_end_time_3)) 
        return true;
    if (IsInTimeRange(hour, bot_info.play_start_time_4, bot_info.play_end_time_4)) 
        return true;

    return false;
}

bool CBotManager::IsAway(const table_users& bot, time_t timeNow) const
{
    return (bot.playTime.check_time * 60) > (static_cast<int>(timeNow) - bot.onlinetime);
}

void CBotManager::Run(CDatabase* pDatabase, const CApplication& app)
{
    // will use it to get vars, reslist etc.
    m_pDatabase = pDatabase;

    // sleep if not loaded yet.
    while (app.IsRunning() 
        && !app.IsLoaded())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // end thread if not running
	if (!app.IsRunning())
	{
		return;
	}

    m_vecBots = m_pDatabase->GetLoadedBots();
    // main loop as long as it is running
    while (app.IsRunning())
    {
        if (!app.IsStarted())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }

        auto timeNow = std::chrono::steady_clock::now();

        // time check
		if (!m_bFirstRun
			&& (timeNow < m_timeLastRun + std::chrono::seconds(wait_time)))
		{
			// sleep shortly to avoid overuse of CPU
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}

        // handlers.
        auto start = GetTimeNow();
        HandleMain();
        // HandleResourceUpdate();
        // HandleBuildings();
        // HandleColonization();

        // save to db
        pDatabase->UpdateBots(m_vecBots);
        // reload from db
        // pDatabase->LoadBots();
        auto end = GetTimeNow();

        ProcessPendingRequests();
        // update time and firstrun flag
        m_bFirstRun = false;
        m_timeLastRun = timeNow;
        auto duration_micros = GetElapsedMicroseconds(start, end);
        double duration_millis = GetElapsedMilliseconds(start, end);
        CLogger::Info("Process handled in [{} microsec / {} millisec]\n", duration_micros, duration_millis);
        // sleep
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    CLogger::Info("Bot Run thread finished.\n");
}

void CBotManager::HandleMain() 
{
    int hour = GetHour();
    const time_t time_now = std::time(nullptr);
    const std::unordered_map<int, table_vars>& vars = m_pDatabase->GetVars();
    const std::unordered_map<int, std::vector<table_vars_requirements>>& 
        vars_requirements = m_pDatabase->GetVarsRequirements();

    for (auto& bot : m_vecBots)
    {
        // reset logs at the start..
        m_log.Reset();
        m_log.bot_id = bot.id;


        if (!IsPlayingNow(bot.playTime, hour))
        {
            m_log.type = 12;
            m_vecLog.push_back(m_log);
            continue;
        }

        if (IsAway(bot, time_now))
        {
            m_log.type = 13;
            m_log.away_time = GetRemainingAwayTimeInSeconds(bot, time_now);
            m_vecLog.push_back(m_log);
            continue;
        }

        if (bot.vacation_mode == 1)
        {
            // todo: add 14
            m_log.type = 14;
            m_vecLog.push_back(m_log);
            continue;
        }

        const table_config* pConfig = GetConfigByUniID(bot.universe);
        if (pConfig == nullptr)
        {
            m_log.universe = bot.universe;
            m_log.type = 1;
            m_vecLog.push_back(m_log);
            continue;
        }
        
        const uint64_t game_speed = std::floor(pConfig->game_speed / 2500);

        bot.onlinetime = static_cast<int>(time_now);

        // loop planets of the bot..
        for (auto& planet : bot.vecPlanets)
        {
            m_log.id_planet = planet.id;

            // 1- HandleResourceUpdate
            HandleResourceUpdate(bot, planet, time_now);
            // 2- HandleBuildings
            HandleBuildings(bot, planet, time_now, vars, vars_requirements, hour, game_speed);
            // 3- HandleResearches
            HandleResearches(bot, planet, time_now, vars, vars_requirements, game_speed);
           
        }
    }

    LogResult();
}

void CBotManager::LogResult()
{
    fmt::memory_buffer buf;
    std::string strMsg;
    // Bu döngü sadece RAM içinde string birleştirir, I/O yapmaz (Çok hızlıdır)
    for (const auto& log : m_vecLog)
    {
        switch (log.type)
        {
        case 1:
            // CLogger::Error("[CBotManager] - Config map missing : uni_id '{}' not found !\n", bot.universe);
            fmt::format_to(std::back_inserter(buf),
                "[CBotManager] - Config map missing : uni_id '{}' not found !\n", log.universe);
            break;
        case 2:
            // CLogger::Info("[CBotManager] - corrected a bot with buggy data !\n", bot.id, planet.id);
            fmt::format_to(std::back_inserter(buf),
                "[CBotManager] - corrected a bot with buggy data id:{}planet_id:{}!\n",
                log.bot_id, log.id_planet);
            break;
        case 3:
            fmt::format_to(std::back_inserter(buf),
                "[CBotManager] - SKIP [botID: {} - planetID: {}], building already !\n",
                log.bot_id, log.id_planet);
            break;
        case 4:
            fmt::format_to(std::back_inserter(buf),
                "[CBotManager] - Research list has been completed for bot: {}\n", log.bot_id);
            break;
        case 5:
            fmt::format_to(std::back_inserter(buf), "[CBotManager] - wrong element id. bot_id : {}!\n", log.bot_id);
            break;
        case 6:
            fmt::format_to(std::back_inserter(buf),
                "[CBotManager] - SKIP [botID:{} - planetID:{}]\n"
                "coordinates:[{}:{}:{}] - email:[{}] \n"
                "not enough resources for element id:{} \n"
                "required: [metal:{}|crystal:{}|deu:{}]\n"
                "have: [metal:{}|crystal:{}|deu:{}]\n",
                log.bot_id,
                log.id_planet,
                log.galaxy, log.system, log.planet,
                log.email,
                log.research_id,
                log.cost901,
                log.cost902,
                log.cost903,
                log.planet_metal,
                log.planet_crystal,
                log.planet_deu);
            break;
        case 7:
            fmt::format_to(std::back_inserter(buf),
                "[CBotManager] - Bot ID {} [Planet: {}] -> Started Research: {} Level {}\n", log.bot_id,
                log.id_planet, log.research_name, log.research_level);
            break;
            break;
        case 8:
            fmt::format_to(std::back_inserter(buf),
                "[CBotManager] - building list has been completed for planet: {}\n",
                log.id_planet);
            break;
        case 9:
            fmt::format_to(std::back_inserter(buf), "[CBotManager] - WRONG ELEMENT ID:{} , NOT FOUND !\n", log.building_id);
            break;
        case 10:
            fmt::format_to(std::back_inserter(buf),
                "[CBotManager] - SKIP [botID:{} - planetID:{}]\n"
                "coordinates:[{}:{}:{}] - email:[{}] \n"
                "not enough resources for element id:{} \n"
                "required: [metal:{}|crystal:{}|deu:{}]\n"
                "have: [metal:{}|crystal:{}|deu:{}]\n",
                log.bot_id, log.id_planet, log.galaxy, log.system, log.planet,
                log.email, log.building_id, log.cost901, log.cost902, log.cost903,
                log.planet_metal, log.planet_crystal, log.planet_deu);
            break;
        case 11:
            fmt::format_to(std::back_inserter(buf),
                "[CBotManager] - Bot ID {} [Planet: {}] -> Started Building: {} Level {}\n",
                log.bot_id, log.id_planet, log.building_name, log.building_level);
            break;
        case 12:
            fmt::format_to(std::back_inserter(buf),
                "[CBotManager] - Bot ID {} Bot is not online now !\n",
                log.bot_id);
            break;
        case 13:
            fmt::format_to(std::back_inserter(buf),
                "[CBotManager] - Bot ID {} Bot is away for {} seconds...\n",
                log.bot_id, log.away_time);
            break;
        case 15:
            fmt::format_to(std::back_inserter(buf),
                "[CBotManager] - Bot ID {} Tech is not accessible.[research_id:{}] \n",
                log.bot_id, log.research_id);
            break;
        case 16:
            fmt::format_to(std::back_inserter(buf),
                "[CBotManager] - Bot ID {} Don't have research lab yet. \n",
                log.bot_id);
            break;
        default:
            fmt::format_to(std::back_inserter(buf), "Bot:{}, Planet:{}\n", log.bot_id, log.id_planet);
            break;
        }
    }

    m_vecLog.clear();

    // 5000 botun tüm bilgisini TEK BİR SEFERDE diske/konsola yazar. 
    // I/O işlemi 5000 kez değil, sadece 1 kez çağrılır!
    CLogger::Info("### BUILD LOGS ###\n{}", fmt::to_string(buf));
}


bool CBotManager::HaveEnoughResources(const table_planets& planet, double* arrCost)
{
	return planet.metal >= arrCost[0]
		&& planet.crystal >= arrCost[1]
		&& planet.deuterium >= arrCost[2];
}

void CBotManager::RemoveCostFromPlanet(table_planets& planet, double* arrCost) 
{
    planet.metal -= arrCost[0];
    planet.crystal -= arrCost[1];
    planet.deuterium -= arrCost[2];
}

bool CBotManager::IsTechAccessible(int element_id, 
    const std::unordered_map<int, std::vector<table_vars_requirements>>& vars_requirements,
    const table_planets& planet,
    const table_users& user)
{
    auto it = vars_requirements.find(element_id);
    if (it == vars_requirements.end())
    {
        return true;
    }

    for (const auto& req : it->second)
    {
        // buildings
        if (req.require_id < 100)
        {
            if (planet.resource[req.require_id] < req.require_level)
                return false;
        }
        // tech
        else
        {
            if (user.resource[req.require_id] < req.require_level)
                return false;
        }
    }

    return true;
}


// php helpers
std::string CBotManager::php_serialize(const PhpArray& arr) {
    std::stringstream ss;

    // PHP array start: a:size:{
    ss << "a:" << arr.size() << ":{";

    for (size_t i = 0; i < arr.size(); ++i) {
        // PHP array index (i:0;, i:1; vb.)
        ss << "i:" << i << ";";

        // check if numeric
        const std::string& val = arr[i];
        bool is_number = !val.empty() && val.find_first_not_of("0123456789-") == std::string::npos;

        if (is_number) 
        {
            // number: i:num
            ss << "i:" << val << ";";
        }
        else {
            // String: s:length:"build";
            ss << "s:" << val.length() << ":\"" << val << "\";";
        }
    }

    ss << "}"; 
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

const table_config* CBotManager::GetConfigByUniID(int uni) const
{
    auto& config = m_pDatabase->GetConfig();
    auto it = config.find(uni);
    if (it == config.end())
    {
        return nullptr;
    }
    return &it->second;
}

const table_vars* CBotManager::GetVarsByID(int id) const
{
    const auto& vars = m_pDatabase->GetVars();
    auto it = vars.find(id);
    if (it == vars.end())
    {
        return nullptr;
    }
    return &it->second;
}