// implementation of CApplication.h

#include "CApplication.h"

CApplication::CApplication() 
	: m_bRunning(true)
	, m_bLoaded(false)
    , m_bStarted(false)
{
	m_database = std::make_unique<CDatabase>();
	m_botManager = std::make_unique<CBotManager>();
	m_logger = std::make_unique<CLogger>();
    m_commandHandler = std::make_unique<CCommandHandler>();
}

CApplication::~CApplication()
{
    if (m_botThread.joinable()) 
    {
        m_botThread.join();
        m_logger->Info("[Application] : Bot thread ended successfully.\n");
    }

    if (m_consoleThread.joinable())
    {
        m_consoleThread.detach();
    }

    Shutdown();
}

bool CApplication::Init() 
{
    if (!m_database->Connect())
    {
        return false;
    }

    if (!LoadVarsFromDatabase()
        || !LoadVarsRequirementsFromDatabase()
        || !LoadConfigFromDatabase()
        || !LoadBotsFromDatabase())
    {
        return false;
    }

    m_bLoaded = true;
    return true;
}

void CApplication::Run() 
{
    // give db pointer as 2nd param.
    // give app as 3rd param.
    m_botThread = std::thread(&CBotManager::Run, m_botManager.get(),m_database.get(), std::ref(*this));
    
    // give botmanager as 2nd, app as 3rd
    m_consoleThread = std::thread(&CCommandHandler::Run, m_commandHandler.get(),
        m_botManager.get(), std::ref(*this));
    
    std::unique_lock<std::mutex> lock(m_shutdownMutex);
    m_shutdownCV.wait(lock, [this]() { return !m_bRunning; });

    m_logger->Info("Closing App...");
}

void CApplication::Shutdown() 
{
    {
        std::lock_guard<std::mutex> lock(m_shutdownMutex);
        m_bRunning = false;
    }

    m_shutdownCV.notify_one(); // open lock of Run functions wait..
}

bool CApplication::LoadBotsFromDatabase() {
    return m_database->LoadBots();
}

bool CApplication::LoadVarsFromDatabase() {
    return m_database->LoadVars();
}

bool CApplication::LoadVarsRequirementsFromDatabase() {
    return m_database->LoadVarsRequirements();
}

bool CApplication::LoadConfigFromDatabase() {
    return m_database->LoadConfig();
}