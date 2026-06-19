#include "CBotManager.h"
#include "CLogger.h"
#include "CDatabase.h"
#include "CApplication.h"

CBotManager::CBotManager(CLanguage* language, CDatabase* database)
	: bots_ptr_(nullptr)
	, database_(nullptr)
	, lang_(nullptr)
	, first_run_(true)
	, time_last_run_(std::chrono::steady_clock::time_point {})
	, system_time_(0)
	, system_hour_(0)
	, loop_time_(30)
	, settlement_data_ptr_(nullptr)
	, vars_ptr_(nullptr)
	, vars_requirements_ptr_(nullptr)
	, bot_max_email_num_(0)
{
	if (language)
	{
		lang_ = language->GetLangStrings();
	}

	if (database)
	{
		database_ = database;
	}

	// seed random
	std::srand(std::time(nullptr));
}

CBotManager::~CBotManager()
{
	bots_ptr_ = nullptr;
}

bool CBotManager::IsInTimeRange(int start_time, int end_time) const
{
	if (start_time == -1
		|| end_time == -1)
		return false;

	// normal (exp: 13-23)
	if (start_time < end_time)
	{
		return (system_hour_ >= start_time && system_hour_ < end_time);
	}

	// 2: exceed midnight (exp: 23-1)
	return (system_hour_ >= start_time || system_hour_ < end_time);
}

bool CBotManager::IsPlayingNow(const table_users& bot) const
{
	play_time bot_info = bot.playTime;
	if (IsInTimeRange(bot_info.play_start_time_1, bot_info.play_end_time_1))
		return true;
	if (IsInTimeRange(bot_info.play_start_time_2, bot_info.play_end_time_2))
		return true;
	if (IsInTimeRange(bot_info.play_start_time_3, bot_info.play_end_time_3))
		return true;
	if (IsInTimeRange(bot_info.play_start_time_4, bot_info.play_end_time_4))
		return true;

	return false;
}

bool CBotManager::IsAway(const table_users& bot) const
{
	return (bot.playTime.check_time * 60) > (static_cast<int>(system_time_) - bot.onlinetime);
}

void CBotManager::Run(const CApplication& app)
{
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

	bots_ptr_ = database_->GetLoadedBots();
	loop_time_ = database_->GetLoopTime();
	settlement_data_ptr_ = database_->GetSettlementData();

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
		if (!first_run_
			&& (timeNow < time_last_run_ + std::chrono::seconds(loop_time_)))
		{
			// sleep shortly to avoid overuse of CPU
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}

		// handlers.
		auto start = GetTimeNow();
		HandleMain();

		// save to db
		database_->UpdateBots();
		// reload from db
		if (database_->RefreshData())
		{
			CLogger::Info(lang_->at("ids_bot_data_refreshed"));
		}
		auto end = GetTimeNow();

		ProcessPendingRequests();
		// update time and firstrun flag
		first_run_ = false;
		time_last_run_ = timeNow;
		auto duration_micros = GetElapsedMicroseconds(start, end);
		double duration_millis = GetElapsedMilliseconds(start, end);
		CLogger::Info(lang_->at("ids_process_handled"),
			duration_micros, duration_millis, loop_time_);
		// sleep
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	CLogger::Info(lang_->at("ids_run_thread_finished"));
}

void CBotManager::HandleMain()
{
	SetSystemTime();
	SetHour();

	for (auto& bot : *bots_ptr_)
	{
		// reset logs at the start..
		log_.Reset();
		log_.bot_id = bot.id;

		if (!IsPlayingNow(bot))
		{
			log_.type = 1;
			logs_.push_back(log_);
			continue;
		}

		if (IsAway(bot))
		{
			log_.type = 2;
			log_.away_time = GetRemainingAwayTimeInSeconds(bot);
			logs_.push_back(log_);
			continue;
		}

		if (IsInVacation(bot))
		{
			log_.type = 3;
			logs_.push_back(log_);
			continue;
		}

		const table_config* config = GetConfigByUniID(bot.universe);
		if (config == nullptr)
		{
			log_.universe = bot.universe;
			log_.type = 4;
			logs_.push_back(log_);
			continue;
		}

		const uint64_t game_speed = std::floor(config->game_speed / 2500);

		bot.onlinetime = static_cast<int>(system_time_);

		// loop planets of the bot..
		for (auto& planet : bot.all_planets)
		{
			log_.id_planet = planet.id;

			// 1- HandleResourceUpdate
			HandleResourceUpdate(bot, planet);
			// 2- HandleBuildings
			HandleBuildings(bot, planet, game_speed);
			// 3- HandleResearches
			HandleResearches(bot, planet, game_speed);
			// 4- HandleColonization
			HandleColonization(bot, config);
		}
	}

	LogResult();
}

void CBotManager::LogResult()
{
	fmt::memory_buffer buf;
	// no I/O
	for (const auto& log : logs_)
	{
		switch (log.type)
		{
			case 1:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_bot_is_not_online")),
					log.bot_id, log.id_planet);
				break;
			case 2:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_bot_is_away")),
					log.away_time, log.bot_id, log.id_planet);
				break;
			case 3:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_bot_in_vacation")),
					log.bot_id, log.id_planet);
				break;
			case 4:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_config_map_missing")),
					log.universe, log.bot_id, log.id_planet);
				break;
			case 5:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_already_building")),
					log.bot_id, log.id_planet);
				break;
			case 6:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_build_list_completed")),
					log.bot_id, log.id_planet);
				break;
			case 7:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_wrong_elem_id")),
					log.building_id, log.bot_id, log.id_planet);
				break;
			case 8:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_tech_not_accessible")),
					log.research_id, log.bot_id, log.id_planet);
				break;
			case 9:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_not_enough_res")),
					log.galaxy, log.system, log.planet,
					log.email, log.building_id, log.cost901, log.cost902, log.cost903,
					log.planet_metal, log.planet_crystal, log.planet_deu,
					log.bot_id, log.id_planet);
				break;
			case 10:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_started_building")),
					log.building_name, log.building_level, log.bot_id, log.id_planet);
				break;
			case 11:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_already_researching")),
					log.bot_id, log.id_planet);
				break;
			case 12:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_planet_dont_have_lab")),
					log.bot_id, log.id_planet);
				break;
			case 13:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_research_list_complete")),
					log.bot_id, log.id_planet);
				break;
			case 14:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_wrong_elem_id")),
					log.building_id, log.bot_id, log.id_planet);
				break;
			case 15:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_tech_not_accessible")),
					log.research_id, log.bot_id, log.id_planet);
				break;
			case 16:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_not_enough_res")),
					log.bot_id, log.id_planet, log.galaxy, log.system, log.planet,
					log.email, log.building_id, log.cost901, log.cost902, log.cost903,
					log.planet_metal, log.planet_crystal, log.planet_deu);
				break;
			case 17:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_started_research")),
					log.research_name, log.research_level, log.id_planet, log.bot_id);
				break;
			default:
				fmt::format_to(std::back_inserter(buf), fmt::runtime(lang_->at("ids_undef_log")));
				break;
		}
	}

	logs_.clear();

	// write all log at once
	CLogger::Info(lang_->at("ids_build_logs_all"), fmt::to_string(buf));
}

bool CBotManager::HaveEnoughResources(const table_planets& planet, double* cost)
{
	return planet.metal >= cost[0]
		   && planet.crystal >= cost[1]
		   && planet.deuterium >= cost[2];
}

void CBotManager::RemoveCostFromPlanet(table_planets& planet, double* cost)
{
	planet.metal -= cost[0];
	planet.crystal -= cost[1];
	planet.deuterium -= cost[2];
}

bool CBotManager::IsTechAccessible(int element_id,
	const table_planets& planet,
	const table_users& user)
{
	auto* vars_requirements = database_->GetVarsRequirements();

	if (vars_requirements == nullptr)
	{
		return false;
	}

	auto it = vars_requirements->find(element_id);
	if (it == vars_requirements->end())
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
std::string CBotManager::php_serialize(const PhpArray& arr)
{
	std::stringstream ss;

	// PHP array start: a:size:{
	ss << "a:" << arr.size() << ":{";

	for (size_t i = 0; i < arr.size(); ++i)
	{
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
		else
		{
			// String: s:length:"build";
			ss << "s:" << val.length() << ":\"" << val << "\";";
		}
	}

	ss << "}";
	return ss.str();
}

PhpArray CBotManager::php_unserialize(const std::string& serialized_data)
{
	PhpArray result_array;
	size_t pos = 0;

	struct BuildQueueItem
	{
		int index;
		int element_id;
		int level;
		time_t end_time;
		std::string task_type;
	};

	while (pos < serialized_data.length())
	{
		size_t sub_array_pos = serialized_data.find("a:5:{", pos);
		if (sub_array_pos == std::string::npos)
		{
			break;
		}

		pos = sub_array_pos + 5;
		BuildQueueItem item {};
		int fields_parsed = 0;

		for (int i = 0; i < 5; ++i)
		{
			if (pos >= serialized_data.length()) break;

			if (serialized_data[pos] == 'i')
			{
				pos = serialized_data.find(';', pos) + 1;
			}

			if (pos >= serialized_data.length()) break;

			if (serialized_data[pos] == 'i')
			{
				size_t val_start = pos + 2;
				size_t val_end = serialized_data.find(';', val_start);
				long long val = std::stoll(serialized_data.substr(val_start, val_end - val_start));

				if (fields_parsed == 0)
					item.index = static_cast<int>(val);
				else if (fields_parsed == 1)
					item.element_id = static_cast<int>(val);
				else if (fields_parsed == 2)
					item.level = static_cast<int>(val);
				else if (fields_parsed == 3)
					item.end_time = static_cast<time_t>(val);

				fields_parsed++;
				pos = val_end + 1;
			}
			else if (serialized_data[pos] == 's')
			{
				size_t len_start = pos + 2;
				size_t len_end = serialized_data.find(':', len_start);
				int str_len = std::stoi(serialized_data.substr(len_start, len_end - len_start));

				size_t str_start = len_end + 2;
				item.task_type = serialized_data.substr(str_start, str_len);

				fields_parsed++;
				pos = str_start + str_len + 2;
			}
		}

		if (fields_parsed == 5)
		{
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
	const auto& config = database_->GetConfig();
	auto it = config.find(uni);
	if (it == config.end())
	{
		return nullptr;
	}
	return &it->second;
}

const table_vars* CBotManager::GetVarsByID(int id) const
{
	auto* vars = database_->GetVars();

	if (vars == nullptr)
	{
		return nullptr;
	}

	auto it = vars->find(id);
	if (it == vars->end())
	{
		return nullptr;
	}

	return &it->second;
}