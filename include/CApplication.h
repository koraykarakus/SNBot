#pragma once

#include <iostream>
#pragma once

#include "CBotManager.h"
#include "CDatabase.h"
#include "CLogger.h"
#include "CCommandHandler.h"
#include "CLanguage.h"
#include "CPhpHelper.h"
#include "globals.h"
#include "table_fleets.h"
#include "table_users.h"

#include <csignal>

class CApplication
{
private:
	bool running_;
	bool loaded_;
	// is it started with command line ?
	bool started_;
	std::mutex mutex_shutdown_;
	std::condition_variable cv_shutdown_;

	// thread
	std::thread bot_thread_;
	std::thread console_thread_;

	// classes
	std::unique_ptr<CBotManager> bot_manager_;
	std::unique_ptr<CDatabase> database_;
	std::unique_ptr<CLogger> logger_;
	std::unique_ptr<CCommandHandler> command_handler_;
	std::unique_ptr<CLanguage> language_;
	std::unique_ptr<CPhpHelper> phphelper_;

public:
	CApplication();
	~CApplication();
	bool Init();
	void Run();
	void Shutdown();
	bool LoadBotsFromDatabase();
	bool LoadVarsFromDatabase();
	bool LoadConfigFromDatabase();
	bool LoadVarsRequirementsFromDatabase();
	bool LoadSettlementDataFromDatabase();

	bool IsRunning() const { return running_; }
	bool IsLoaded() const { return loaded_; }
	bool IsStarted() const { return started_; }
	inline void Close()
	{
		running_ = false;
		cv_shutdown_.notify_all();
	}
	inline void Start()
	{
		started_ = true;
	}
};