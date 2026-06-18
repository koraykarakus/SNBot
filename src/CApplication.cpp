// implementation of CApplication.h

#include "CApplication.h"

CApplication::CApplication() 
	: running_(true)
	, loaded_(false)
    , started_(false)
{
    language_ = std::make_unique<CLanguage>();
    
    database_ = std::make_unique<CDatabase>(
        language_.get()
    );

    bot_manager_ = std::make_unique<CBotManager>(
        language_.get(),
        database_.get()
    );

    logger_ = std::make_unique<CLogger>();
    
    command_handler_ = std::make_unique<CCommandHandler>(
        language_.get(),
        bot_manager_.get()
    );
}

CApplication::~CApplication()
{
    if (bot_thread_.joinable())
    {
        bot_thread_.join();
        logger_->Info("[Application] : Bot thread ended successfully.\n");
    }

    if (console_thread_.joinable())
    {
        console_thread_.detach();
    }

    Shutdown();
}

bool CApplication::Init() 
{
    if (!database_->Connect())
    {
        return false;
    }

    if (!LoadVarsFromDatabase()
        || !LoadVarsRequirementsFromDatabase()
        || !LoadConfigFromDatabase()
        || !LoadBotsFromDatabase()
        || !LoadSettlementDataFromDatabase())
    {
        return false;
    }

    loaded_ = true;
    return true;
}

void CApplication::Run() 
{
    // give db pointer as 2nd param.
    // give app as 3rd param.
    bot_thread_ = std::thread(&CBotManager::Run, 
        bot_manager_.get(),
        std::ref(*this));
    
    // give botmanager as 2nd, app as 3rd
    console_thread_ = std::thread(&CCommandHandler::Run, 
        command_handler_.get(),
        std::ref(*this));
    
    std::unique_lock<std::mutex> lock(mutex_shutdown_);
    cv_shutdown_.wait(lock, [this]() { return !running_; });

    logger_->Info("Closing App...");
}

void CApplication::Shutdown() 
{
    {
        std::lock_guard<std::mutex> lock(mutex_shutdown_);
        running_ = false;
    }

    cv_shutdown_.notify_one(); // open lock of Run functions wait..
}

bool CApplication::LoadBotsFromDatabase() {
	return database_->LoadBots();
}

bool CApplication::LoadVarsFromDatabase() {
    return database_->LoadVars();
}

bool CApplication::LoadVarsRequirementsFromDatabase() {
    return database_->LoadVarsRequirements();
}

bool CApplication::LoadConfigFromDatabase() {
    return database_->LoadConfig();
}

bool CApplication::LoadSettlementDataFromDatabase() {
    return database_->LoadSettlementData();
}