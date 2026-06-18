// implementation of CCommandHandler.h
#include "CCommandHandler.h"
#include "CApplication.h"

CCommandHandler::CCommandHandler(CLanguage* language, 
    CBotManager* botManager)
	: bot_manager_(nullptr)
    , lang_(nullptr)
{
    if (language)
    {
        lang_ = language->GetLangStrings();
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
    std::cout << "==========================================================\n" << std::endl;

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
        if (count > 0)
        {
            // into queue
            if (bot_manager_ != nullptr)
            {
                info.type = 1;
                info.count = count;
                info.universe = universe;
                bot_manager_->CreateBots(info);
                // uncomment
                //bot_manager_->PushCmdRequest(info);
                //CLogger::Info(lang_->at("ids_addbot_received"), count);
            }
        }
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
	}
    else if (cmd == "/remove_bots")
    {
        info.type = 2;
        bot_manager_->PushCmdRequest(info);
    }
    else if (cmd == "/exit")
    {
        CLogger::Info(lang_->at("ids_cmd_exid"));
        app.Close();
    }
    else
    {
        CLogger::Info(lang_->at("ids_cmd_wrong"));
    }

    return true;
}