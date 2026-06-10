#include "CBotManager.h"
#include "CLogger.h"
#include "CDatabase.h"
#include "CLoader.h"

extern bool g_bRunning;

CBotManager::CBotManager()
    : m_vecBots{}
    , m_bFirstRun(true)
    , m_timeLastRun(0)
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

    CLogger::Info("[CBotManager] - [Run] Bot Main thread starting. vars size : {}\n", G_VARS.size());

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
        auto start = GetTimeNow();
        HandleResourceUpdate();
        HandleBuildings();

        // save to db
        g_pDatabase->UpdateBots();
        // reload from db
        // g_pDatabase->LoadBots();
        auto end = GetTimeNow();

        // update time and firstrun flag
        m_bFirstRun = false;
        m_timeLastRun = timeNow;
        auto duration_micros = GetElapsedMicroseconds(start, end);
        double duration_millis = GetElapsedMilliseconds(start, end);
        CLogger::Info("Process handled in [{} microsec / {} milisec]\n", duration_micros, duration_millis);
        // sleep
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    CLogger::Info("Bot Run thread finished.\n");
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