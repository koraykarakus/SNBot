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
    // init game constants

    if (!g_Database.Connect())
    {
        return false;
    }

    if (!LoadBotsFromDatabase()
        || !LoadVarsFromDatabase())
    {
        return false;
    }

    return true;
}

bool CLoader::LoadBotsFromDatabase()
{
    return g_Database.LoadBots();
}

bool CLoader::LoadVarsFromDatabase() 
{
    return g_Database.LoadVars();
}

void CLoader::ShutDown()
{
    // cleanup memory

}