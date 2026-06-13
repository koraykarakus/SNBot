// implementation of CCommandHandler.h
#include "CCommandHandler.h"
#include "CApplication.h"

CCommandHandler::CCommandHandler()
{
    m_pBotManager = nullptr;
}

CCommandHandler::~CCommandHandler()
{
}

void CCommandHandler::Run(CBotManager* pBotManager, CApplication& app)
{
    std::cout << "\n==========================================================" << std::endl;
    std::cout << "[Console] App is ready! Write /help to display commands." << std::endl;
    std::cout << "==========================================================\n" << std::endl;

    m_pBotManager = pBotManager;

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
        int count = 0; ss >> count;
        if (count > 0)
        {
            // into queue
            if (m_pBotManager != nullptr)
            {
                info.type = 1;
                info.count = count;
                m_pBotManager->PushCmdRequest(info);
                std::cout << "[Console] " << count << " bot add request accepted.\n";
            }
        }
    }
    else if (cmd == "/start")
    {
        app.Start();
    }
    else if (cmd == "/help")
    {
        std::cout << "type /start to start bot processing\n";
        std::cout << "type /exit to close\n";
        std::cout << "type /add_bot 100 -> to add 100 bots\n";
        std::cout << "type /remove_bots -> to delete all bots and their planet\n";
    }
    else if (cmd == "/remove_bots")
    {

    }
    else if (cmd == "/exit")
    {
        std::cout << "[Console] exitting...\n";
        app.Close();
    }
    else
    {
        std::cout << "Wrong command, type /help to display commands.\n";
    }

    return true;
}