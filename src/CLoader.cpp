/*
#include "CLoader.h"
#include "CLogger.h"
#include "CDatabase.h"
#include "CBotManager.h"


CLoader::CLoader()
{
}

CLoader::~CLoader()
{
}

bool CLoader::Init()
{
    if (!g_pDatabase->Connect())
    {
        return false;
    }

    if (!LoadVarsFromDatabase()
        || !LoadConfigFromDatabase()
        || !LoadBotsFromDatabase())
    {
        return false;
    }

    g_bLoaded = true;
    return true;
}

bool CLoader::LoadBotsFromDatabase()
{
    return g_pDatabase->LoadBots();
}

bool CLoader::LoadVarsFromDatabase() 
{
    return g_pDatabase->LoadVars();
}

bool CLoader::LoadConfigFromDatabase()
{
    return g_pDatabase->LoadConfig();
}

void CLoader::ShutDown()
{
    // cleanup memory
}
*/