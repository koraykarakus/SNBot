// implementation of CCommandHandler.h
#include "CCommandHandler.h"
#include "CApplication.h"

CCommandHandler::CCommandHandler(CLanguage* language,
	CBotManager* botManager)
	: bot_manager_(nullptr)
	, lang_(nullptr)
	, language_(nullptr)
{
	if (language)
	{
		lang_ = language->GetLangStrings();
		language_ = language;
	}

	if (botManager)
	{
		bot_manager_ = botManager;
	}
}

CCommandHandler::~CCommandHandler()
{
}

void CCommandHandler::Run(CApplication& app)
{
	std::cout << "\n==========================================================" << std::endl;
	std::cout << "[Console] App is ready! Write /help to display commands." << std::endl;
	std::cout << "==========================================================\n"
			  << std::endl;

	std::string line;
	while (app.IsRunning())
	{
		if (!std::getline(std::cin, line))
			continue;

		if (line.empty())
			continue;

		ProcessCommand(line, app);
	}
}

bool CCommandHandler::ProcessCommand(std::string& line, CApplication& app)
{
	std::stringstream ss(line);
	std::string cmd;
	ss >> cmd;
	cmd_queue info = {};

	if (cmd == "/add_bot")
	{
		int count = 0;
		int universe = 0;

		ss >> count;
		ss >> universe;

		if (count <= 0)
		{
			CLogger::Error(lang_->at("ids_addbot_wrong_count"), count);
			return false;
		}

		if (bot_manager_ == nullptr)
		{
			CLogger::Error(lang_->at("ids_addbot_null"));
			return false;
		}

		const config_data* config = bot_manager_->GetConfigByUniID(universe);
		if (config == nullptr)
		{
			CLogger::Error(lang_->at("ids_addbot_wrong_uni"), universe);
			return false;
		}

		// into queue
		info.type = 1;
		info.count = count;
		info.universe = universe;

		if (app.IsStarted())
			bot_manager_->PushCmdRequest(info);
		else
			bot_manager_->CreateBots(info);

		CLogger::Info(lang_->at("ids_addbot_received"), count);
	}
	else if (cmd == "/start")
	{
		app.Start();
	}
	else if (cmd == "/help")
	{
		CLogger::Info(lang_->at("ids_help_start"));
		CLogger::Info(lang_->at("ids_help_exit"));
		CLogger::Info(lang_->at("ids_help_addbots"));
		CLogger::Info(lang_->at("ids_help_removebots"));
		CLogger::Info(lang_->at("ids_help_set_lang"));
		CLogger::Info(lang_->at("ids_help_add_metal"));
		CLogger::Info(lang_->at("ids_help_add_crystal"));
		CLogger::Info(lang_->at("ids_help_add_deu"));
		CLogger::Info(lang_->at("ids_help_add_dm"));
	}
	else if (cmd == "/remove_bots")
	{
		if (app.IsStarted())
		{
			info.type = 2;
			bot_manager_->PushCmdRequest(info);
		}
		else
		{
			bot_manager_->RemoveBots();
		}
	}
	else if (cmd == "/exit")
	{
		CLogger::Info(lang_->at("ids_cmd_exit"));
		app.Close();
	}
	else if (cmd == "/set_lang")
	{
		std::string lang_key = "en";
		ss >> lang_key;
		if (language_->ExistsLang(lang_key))
		{
			if (language_->LoadLangFile(lang_key))
			{
				lang_ = language_->GetLangStrings();
				CLogger::Info(lang_->at("ids_lang_change_success"));
			}
			else
			{
				CLogger::Info("loading language file failed");
			}
		}
		else
		{
			CLogger::Error(lang_->at("ids_wrong_lang_key"));
		}
	}
	else if (cmd == "/add_metal")
	{
		int count = 0;
		ss >> count;
		if (count <= 0) return false;

		// into queue
		info.type = 3;
		info.count = count;

		if (app.IsStarted())
			bot_manager_->PushCmdRequest(info);
		else
			bot_manager_->AddMetal(info);
	}
	else if (cmd == "/add_crystal")
	{
		int count = 0;
		ss >> count;
		if (count <= 0) return false;

		// into queue
		info.type = 4;
		info.count = count;

		if (app.IsStarted())
			bot_manager_->PushCmdRequest(info);
		else
			bot_manager_->AddCrystal(info);
	}
	else if (cmd == "/add_deu")
	{
		int count = 0;
		ss >> count;
		if (count <= 0) return false;

		// into queue
		info.type = 5;
		info.count = count;

		if (app.IsStarted())
			bot_manager_->PushCmdRequest(info);
		else
			bot_manager_->AddDeu(info);
	}
	else if (cmd == "/add_dm")
	{
		int count = 0;
		ss >> count;
		if (count <= 0) return false;

		// into queue
		info.type = 6;
		info.count = count;

		if (app.IsStarted())
			bot_manager_->PushCmdRequest(info);
		else
			bot_manager_->AddDm(info);
	}
	else
	{
		CLogger::Info(lang_->at("ids_cmd_wrong"));
	}

	return true;
}