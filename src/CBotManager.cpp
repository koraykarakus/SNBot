#include "CBotManager.h"
#include "CLogger.h"
#include "CDatabase.h"
#include "CApplication.h"

CBotManager::CBotManager(CLanguage* language, 
	CDatabase* database, CPhpHelper* phphelper)
	: bots_ptr_(nullptr)
	, database_(nullptr)
	, phphelper_(nullptr)
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

	if (phphelper)
	{
		phphelper_ = phphelper;
	}

	// seed random
	std::srand(std::time(nullptr));
}

CBotManager::~CBotManager()
{
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
			log_.type = log_type::not_playing_now;
			logs_.push_back(log_);
			continue;
		}

		if (IsAway(bot))
		{
			log_.type = log_type::is_away;
			log_.away_time = GetRemainingAwayTimeInSeconds(bot);
			logs_.push_back(log_);
			continue;
		}

		if (IsInVacation(bot))
		{
			log_.type = log_type::in_vacation;
			logs_.push_back(log_);
			continue;
		}

		const config_data* config = GetConfigByUniID(bot.universe);
		if (config == nullptr)
		{
			log_.universe = bot.universe;
			log_.type = log_type::config_wrong;
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

	for (const auto& log : logs_)
	{
		switch (log.type)
		{
			case log_type::not_playing_now:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_bot_is_not_online")),
					log.bot_id, log.id_planet);
				break;
			case log_type::is_away:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_bot_is_away")),
					log.away_time, log.bot_id, log.id_planet);
				break;
			case log_type::in_vacation:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_bot_in_vacation")),
					log.bot_id, log.id_planet);
				break;
			case log_type::config_wrong:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_config_map_missing")),
					log.universe, log.bot_id, log.id_planet);
				break;
			case log_type::building_already:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_already_building")),
					log.bot_id, log.id_planet);
				break;
			case log_type::build_list_finished:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_build_list_completed")),
					log.bot_id, log.id_planet);
				break;
			case log_type::build_elem_not_found:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_wrong_elem_id")),
					log.building_id, log.bot_id, log.id_planet);
				break;
			case log_type::tech_not_accessible:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_tech_not_accessible")),
					log.research_id, log.bot_id, log.id_planet);
				break;
			case log_type::not_have_enough_resources:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_not_enough_res")),
					log.galaxy, log.system, log.planet,
					log.email, log.building_id, log.cost901, log.cost902, log.cost903,
					log.planet_metal, log.planet_crystal, log.planet_deu,
					log.bot_id, log.id_planet);
				break;
			case log_type::building_success:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_started_building")),
					log.building_name, log.building_level, log.bot_id, log.id_planet);
				break;
			case log_type::researching_already:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_already_researching")),
					log.bot_id, log.id_planet);
				break;
			case log_type::dont_have_lab:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_planet_dont_have_lab")),
					log.bot_id, log.id_planet);
				break;
			case log_type::research_list_finished:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_research_list_complete")),
					log.bot_id, log.id_planet);
				break;
			case log_type::research_elem_not_found:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_wrong_elem_id")),
					log.building_id, log.bot_id, log.id_planet);
				break;
			case log_type::tech_not_accessible_research:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_tech_not_accessible")),
					log.research_id, log.bot_id, log.id_planet);
				break;
			case log_type::not_have_enough_resources_research:
				fmt::format_to(std::back_inserter(buf),
					fmt::runtime(lang_->at("ids_not_enough_res")),
					log.bot_id, log.id_planet, log.galaxy, log.system, log.planet,
					log.email, log.building_id, log.cost901, log.cost902, log.cost903,
					log.planet_metal, log.planet_crystal, log.planet_deu);
				break;
			case log_type::research_success:
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

const config_data* CBotManager::GetConfigByUniID(int uni) const
{
	const auto& config = database_->GetConfig();
	auto it = config.find(uni);
	if (it == config.end())
	{
		return nullptr;
	}
	return &it->second;
}

const vars_data* CBotManager::GetVarsByID(int id) const
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