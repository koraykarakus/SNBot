// implementation of CLanguage.h

#include "CLanguage.h"
#include "CLogger.h"
#include <filesystem>
#include <fstream>
#include "toml++/toml.hpp"

CLanguage::CLanguage()
	: lang_(lang::en)
	, strings_ {}
{
	Init();
}

CLanguage::~CLanguage()
{
}

void CLanguage::Init()
{
	std::string fn = "language/lang_en.toml"; // default
	std::filesystem::path lang_path =
		std::filesystem::current_path() / fn;

	// create language file if it does not exist
	if (!std::filesystem::exists(lang_path))
	{
		CreateDefaultLangFile();
	}
	// open language file
	else
	{
		if (!LoadLangFile())
		{
			CreateDefaultLangFile();
			LoadLangFile();
		}
	}
}

bool CLanguage::LoadLangFile(std::string lang_key /*= "en"*/)
{
	std::string fn = "language/lang_" + lang_key + ".toml"; // default
	std::filesystem::path lang_path =
		std::filesystem::current_path() / fn;

	toml::table lang_table;

	try
	{
		lang_table = toml::parse_file(lang_path.string());
		auto* strings = lang_table["strings"].as_table();
		if (strings == nullptr)
		{
			return false;
		}
		strings_.clear();
		for (const auto& [key, value] : *strings)
		{
			if (auto str = value.value<std::string>())
			{
				strings_.emplace(std::string(key.str()), *str);
			}
		}
	}
	catch (const toml::parse_error& err)
	{
		std::string str = "Error parsing lang_" + lang_key + ".toml: {}\n";
		CLogger::Error(str, err.description());
		return false;
	}
}

void CLanguage::CreateDefaultLangFile()
{
	toml::table lang;
	lang = toml::table {
		{"strings", toml::table {
						// CDatabase.cpp
						{"ids_settings_not_found", "settings.toml not found! Creating default configuration file\n"},
						{"ids_settings_cannot_be_saved", "settings.toml cannot be saved!\n"},
						{"ids_settings_cannot_be_parsed", "cannot parse settings.toml: {}\n"},
						{"ids_settings_read", "[CDatabase] settings read from settings.toml Host: {}\n"},
						{"ids_mysql_init_failed", "[CDatabase] - mysql_init failed\n"},
						{"ids_ssl_is_active", "ssl is active\n"},
						{"ids_mysql_conn_failed", "[CDatabase] - MySQL Connection Failed. Error: {} ({})\n"},
						{"ids_mysql_query_error", "[CDatabase] - Query Error {}\n"},
						{"ids_mysql_retrieve_error", "[CDatabase] - Retrieve result error {}\n"},
						{"ids_found_num_bots", "[CDatabase] - Found {} bots in database.\n"},
						{"ids_no_match_planet_bot", "Planet ID {} has owner ID {} but no matching bot was found!\n"},
						{"ids_load_planet_bots_succ", "[CDatabase] ### {} bots and {} planets loaded successfully. ### [time:{} us / {} ms]\n"},
						{"ids_load_vars_succ", "[CDatabase] - Vars NUM : {} (vars) has been successfully loaded.\n"},
						{"ids_load_varsreq_succ", "[CDatabase] - Vars_Requirements NUM : {} (vars_req) has been successfully loaded.\n"},
						{"ids_load_config_succ", "[CDatabase] - {}x config row has been loaded.\n"},
						{"ids_update_planet_bots_succ", "[CDatabase] - {} bots and {} planets updated successfully in database.- time {}ms - {}us]\n"},
						{"ids_mysql_autocommit_fail", "[CDatabase] - Failed to disable autocommit: {}\n"},
						{"ids_mysql_commit_fail", "[CDatabase] - Commit Error: {}\n"},
						{"ids_load_settlement_succ", "[CDatabase] - {}x settlement data has been loaded.\n"},
						// end CDatabase.cpp

						// CBotManager
						{"ids_bot_data_refreshed", "bot data has been refreshed\n"},
						{"ids_process_handled", "Process handled in [{} microsec / {} millisec]\nNext run is in {} seconds.\n"},
						{"ids_run_thread_finished", "Bot Run thread finished.\n"},
						{"ids_bot_is_not_online", "skip - bot is not online now. uid:{} - pid:{}\n"},
						{"ids_bot_is_away", "skip - bot is away for {} seconds. uid:{} - pid:{}\n"},
						{"ids_bot_in_vacation", "skip - bot is in vacation mode. uid:{} - pid:{}\n"},
						{"ids_config_map_missing", "skip - config map missing : uni_id:{} not found. uid:{} - pid:{}\n"},
						{"ids_already_building", "skip - already building. uid:{} - pid:{}\n"},
						{"ids_build_list_completed", "skip - building list has been completed. uid:{} - pid:{}\n"},
						{"ids_wrong_elem_id", "skip - wrong element id:[{}]. uid:{} - pid:{}\n"},
						{"ids_tech_not_accessible", "skip - tech is not accessible for research id:[{}]. uid:{} - pid:{}\n"},
						{"ids_not_enough_res", "skip - [g{}:s{}:p{}] email:[{}]}\nnot enough resources for build id:{}\nrequired:[metal:{}|crystal:{}|deu:{}] have:[metal:{}|crystal:{}|deu:{}]\nbid:{} - pid:{}\n"},
						{"ids_started_building", "started building - {}, level:{}. uid:{} - pid:{}\n"},
						{"ids_already_researching", "skip - already researching.. uid:{} - pid:{}\n"},
						{"ids_planet_dont_have_lab", "skip - planet don't have laboratory. uid:{} - pid:{}\n"},
						{"ids_research_list_complete", "skip - research list has been completed for bot. uid:{} - pid:{}\n"},
						{"ids_started_research", "started research - {}, level:{}. uid:{} - pid:{}\n"},
						{"ids_undef_log", "Undefined log type.\n"},
						{"ids_build_logs_all", "### BUILD LOGS ###\n{}\n"},
						{"ids_short_logs", "### Short logs, only one from each type of bot ###\n{}\n"},

						// CHandleConsoleCommand

						{"ids_creating_bots", "Creating {} bots..\n"},
						{"ids_bot_remove_succ", "All bots and their planet removed from database by success.\n"},

						// CCommandHandler
						{"ids_help_start", "type /start to start bot processing\n"},
						{"ids_help_exit", "type /exit to close\n"},
						{"ids_help_addbots", "type /add_bot 100 1 -> to add 100 bots to universe 1\n"},
						{"ids_help_removebots", "type /remove_bots -> to delete all bots and their planet\n"},
						{"ids_help_set_lang", "type /set_lang en -> to set language as english, other keys [de,tr,es,pt,pl,fr,ru]"},
						{"ids_lang_change_success", "Language settings have been changed by success.\n"},
						{"ids_wrong_lang_key", "Wrong language key, try:[de,en,es,fr,pl,pt,tr,ru] \n"},
						{"ids_cmd_exit", "[Console] exitting...\n"},
						{"ids_cmd_wrong", "Wrong command, type /help to display commands.\n"},
						{"ids_addbot_received", "[Console]: {} bot add request accepted.\n"},
						{"ids_addbot_wrong_count", "Wrong input for count {}\n"},
						{"ids_addbot_null", "Cannot access botmanager pointer, it is null\n"},
						{"ids_addbot_wrong_uni", "Cannot found config with entered uni id : {}\n"},

					}}};

	std::string fn = "language/lang_en.toml"; // default
	std::filesystem::path lang_path =
		std::filesystem::current_path() / fn;
	std::ofstream out_file(lang_path);
	if (out_file.is_open())
	{
		out_file << lang;
		out_file.close();
	}
	else
	{
		CLogger::Error("Error:[CLanguage] - CreateDefaultLangFile()");
	}
}

bool CLanguage::ExistsLang(std::string& lang_key)
{
	for (auto& key : s_lang_arr)
	{
		if (key == lang_key)
		{
			return true;
		}
	}
	return false;
}