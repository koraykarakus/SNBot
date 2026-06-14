#pragma once

#include <iostream>
#include <csignal>


#include "CBotManager.h"
#include "CDatabase.h"
#include "CLogger.h"
#include "CCommandHandler.h"
#include "globals.h"
#include "table_fleets.h"
#include "table_users.h"

class CApplication
{
private:
	bool m_bRunning;
	bool m_bLoaded;
	// is it started with command line ?
	bool m_bStarted;
	std::mutex m_shutdownMutex;
	std::condition_variable m_shutdownCV;

	// thread
	std::thread m_botThread;
	std::thread m_consoleThread;

	// classes
	std::unique_ptr<CBotManager> m_botManager;
	std::unique_ptr<CDatabase> m_database;
	std::unique_ptr<CLogger> m_logger;
	std::unique_ptr<CCommandHandler> m_commandHandler;
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

	bool IsRunning() const { return m_bRunning; }
	bool IsLoaded() const { return m_bLoaded; }
	bool IsStarted() const { return m_bStarted; }
	inline void Close() 
	{
		m_bRunning = false; m_shutdownCV.notify_all();
	}
	inline void Start() {
		m_bStarted = true;
	}
};