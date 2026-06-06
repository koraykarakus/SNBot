#include "Loader.h"
#include "logger.h"
#include <curl/curl.h>
#include "Bot.h"
#include "GameInfo.h"
#include "DBAgent.h"

GameInfo* g_pGameInfo = nullptr;
DBAgent* g_pDBAgent = nullptr;

Loader::Loader()
{
}

Loader::~Loader()
{
}

bool Loader::Init()
{
    // init game constants
    g_pGameInfo = new GameInfo();
    g_pDBAgent = new DBAgent();

    if (!g_pDBAgent->Connect())
    {
        return false;
    }

    if (!g_pGameInfo->Init())
    {
        return false;
    }

    if (!LoadBotsFromDatabase())
    {
        return false;
    }

    // init curl lib, for later use
    if (curl_global_init(CURL_GLOBAL_ALL) != 0) 
    {
        return false;
    }

    return true;
}

bool Loader::LoadBotsFromDatabase() 
{
    return g_pDBAgent->LoadBots();
}

void Loader::ShutDown()
{
    // cleanup memory

    if (g_pGameInfo != nullptr)
    {
        delete g_pGameInfo;
        g_pGameInfo = nullptr;
    }

    if (g_pDBAgent != nullptr)
    {
        delete g_pDBAgent;
        g_pDBAgent = nullptr;
    }

    // cleanup curl
    curl_global_cleanup();
}