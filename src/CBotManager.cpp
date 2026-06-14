#include "CBotManager.h"
#include "CLogger.h"
#include "CDatabase.h"
#include "CApplication.h"

CBotManager::CBotManager()
    : m_vecBots{}
    , m_bFirstRun(true)
    , m_timeLastRun(std::chrono::steady_clock::time_point{})
    , m_pDatabase(nullptr)
    , m_sysTime(0)
    , m_sysHour(0)
    , m_loopTime(30)
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

bool CBotManager::IsPlayingNow(const table_users& bot) const
{
    play_time bot_info = bot.playTime;
    if (IsInTimeRange(m_sysHour, bot_info.play_start_time_1, bot_info.play_end_time_1)) 
        return true;
    if (IsInTimeRange(m_sysHour, bot_info.play_start_time_2, bot_info.play_end_time_2))
        return true;
    if (IsInTimeRange(m_sysHour, bot_info.play_start_time_3, bot_info.play_end_time_3))
        return true;
    if (IsInTimeRange(m_sysHour, bot_info.play_start_time_4, bot_info.play_end_time_4))
        return true;

    return false;
}

bool CBotManager::IsAway(const table_users& bot) const
{
    return (bot.playTime.check_time * 60) > (static_cast<int>(m_sysTime) - bot.onlinetime);
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
    m_loopTime = m_pDatabase->GetLoopTime();
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
			&& (timeNow < m_timeLastRun + std::chrono::seconds(m_loopTime)))
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
        CLogger::Info(
            "Process handled in [{} microsec / {} millisec]\n"
            "Next run is in {} seconds.",
             duration_micros, duration_millis, m_loopTime);
        // sleep
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    CLogger::Info("Bot Run thread finished.\n");
}

void CBotManager::HandleMain() 
{
    SetSystemTime();
    SetHour(); 

    const std::unordered_map<int, table_vars>& vars = m_pDatabase->GetVars();
    const std::unordered_map<int, std::vector<table_vars_requirements>>& 
        vars_requirements = m_pDatabase->GetVarsRequirements();

    for (auto& bot : m_vecBots)
    {
        // reset logs at the start..
        m_log.Reset();
        m_log.bot_id = bot.id;


        if (!IsPlayingNow(bot))
        {
            m_log.type = 1;
            m_vecLog.push_back(m_log);
            continue;
        }

        if (IsAway(bot))
        {
            m_log.type = 2;
            m_log.away_time = GetRemainingAwayTimeInSeconds(bot);
            m_vecLog.push_back(m_log);
            continue;
        }

        if (IsInVacation(bot))
        {
            m_log.type = 3;
            m_vecLog.push_back(m_log);
            continue;
        }

        const table_config* pConfig = GetConfigByUniID(bot.universe);
        if (pConfig == nullptr)
        {
            m_log.universe = bot.universe;
            m_log.type = 4;
            m_vecLog.push_back(m_log);
            continue;
        }
        
        const uint64_t game_speed = std::floor(pConfig->game_speed / 2500);

        bot.onlinetime = static_cast<int>(m_sysTime);

        // loop planets of the bot..
        for (auto& planet : bot.vecPlanets)
        {
            m_log.id_planet = planet.id;

            // 1- HandleResourceUpdate
            HandleResourceUpdate(bot, planet);
            // 2- HandleBuildings
            HandleBuildings(bot, planet, vars, vars_requirements, game_speed);
            // 3- HandleResearches
            HandleResearches(bot, planet, vars, vars_requirements, game_speed);
           
        }
    }

    LogResult();
}

void CBotManager::LogResult()
{
    fmt::memory_buffer buf;
    // no I/O
    for (const auto& log : m_vecLog)
    {
        switch (log.type)
        {
		case 1:
			fmt::format_to(std::back_inserter(buf),
				"skip - bot is not online now. uid:{} - pid:{}\n",
				log.bot_id, log.id_planet);
			break;
		case 2:
			fmt::format_to(std::back_inserter(buf),
				"skip - bot is away for {} seconds. uid:{} - pid:{}\n",
				log.away_time, log.bot_id, log.id_planet);
			break;
		case 3:
			fmt::format_to(std::back_inserter(buf),
				"skip - bot is in vacation mode. uid:{} - pid:{}\n",
				log.bot_id, log.id_planet);
			break;
        case 4:
            fmt::format_to(std::back_inserter(buf),
                "skip - config map missing : uni_id:{} not found. uid:{} - pid:{}\n", 
                log.universe, log.bot_id, log.id_planet);
            break;
        case 5:
            fmt::format_to(std::back_inserter(buf),
                "skip - already building. uid:{} - pid:{}\n",
                log.bot_id, log.id_planet);
            break;
        case 6:
            fmt::format_to(std::back_inserter(buf),
                "skip - building list has been completed. uid:{} - pid:{}\n",
                log.bot_id, log.id_planet);
            break;
        case 7:
            fmt::format_to(std::back_inserter(buf), 
                "skip - wrong element id:[{}]. uid:{} - pid:{}\n", 
                log.building_id, log.bot_id, log.id_planet);
            break;
        case 8:
            fmt::format_to(std::back_inserter(buf),
                "skip - tech is not accessible for research id:[{}]. uid:{} - pid:{}\n",
                log.research_id, log.bot_id, log.id_planet);
            break;
        case 9:
            fmt::format_to(std::back_inserter(buf),
                "skip - [g{}:s{}:p{}] email:[{}]\n"
                "not enough resources for build id:{}\n"
                "required:[metal:{}|crystal:{}|deu:{}] have:[metal:{}|crystal:{}|deu:{}]\n"
                "bid:{} - pid:{}\n",
                 log.galaxy, log.system, log.planet,
                 log.email, log.building_id, log.cost901, log.cost902, log.cost903,
                 log.planet_metal, log.planet_crystal, log.planet_deu,
                 log.bot_id, log.id_planet);
            break;
        case 10:
            fmt::format_to(std::back_inserter(buf),
                "started building - {}, level:{}. uid:{} - pid:{}\n",
                log.building_name, log.building_level, log.bot_id, log.id_planet);
            break;
        case 11:
            fmt::format_to(std::back_inserter(buf),
                "skip - already researching.. uid:{} - pid:{}\n",
                log.bot_id, log.id_planet);
            break;
        case 12:
            fmt::format_to(std::back_inserter(buf),
                "skip - planet don't have laboratory. uid:{} - pid:{}\n",
                log.bot_id, log.id_planet);
            break;
        case 13:
            fmt::format_to(std::back_inserter(buf),
                "skip - research list has been completed for bot. uid:{} - pid:{}\n", 
                log.bot_id, log.id_planet);
            break;
        case 14:
            fmt::format_to(std::back_inserter(buf), 
                "skip - wrong element id:{}. uid:{} - pid:{}\n", 
                log.building_id,log.bot_id, log.id_planet);
            break;
        case 15:
            fmt::format_to(std::back_inserter(buf),
                "skip - tech is not accessible for [research_id:{}]. uid:{} - pid:{}\n",
                log.research_id, log.bot_id, log.id_planet);
            break;
        case 16:
            fmt::format_to(std::back_inserter(buf),
                "skip - uid:{} - pid:{} [g{}:s{}:p{}] - email:[{}] \n"
                "not enough res for element id:{} \n"
                "required: [metal:{}|crystal:{}|deu:{}] have: [metal:{}|crystal:{}|deu:{}]\n",
                log.bot_id, log.id_planet, log.galaxy, log.system, log.planet,
                log.email, log.building_id, log.cost901, log.cost902, log.cost903,
                log.planet_metal, log.planet_crystal, log.planet_deu);
            break;
        case 17:
            fmt::format_to(std::back_inserter(buf),
                "started research - {}, level:{}. uid:{} - pid:{}\n",
                log.research_name, log.research_level, log.id_planet, log.bot_id);
            break;
        default:
            fmt::format_to(std::back_inserter(buf), "Undefined log type.\n");
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