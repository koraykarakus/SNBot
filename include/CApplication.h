#pragma once

#include <iostream>
#include <csignal>


#include "CBotManager.h"
#include "CDatabase.h"
#include "CLogger.h"
#include "globals.h"
#include "table_fleets.h"
#include "table_users.h"

class CApplication
{
private:
	bool m_bRunning;
	bool m_bLoaded;
	std::mutex m_shutdownMutex;
	std::condition_variable m_shutdownCV;

	// thread
	std::thread m_botThread;

	// classes
	std::unique_ptr<CBotManager> m_botManager;
	std::unique_ptr<CDatabase> m_database;
	std::unique_ptr<CLogger> m_logger;
public:

public:
	CApplication();
	~CApplication();
	bool Init();
	void Run();
	void Shutdown();
	bool LoadBotsFromDatabase();
	bool LoadVarsFromDatabase();
	bool LoadConfigFromDatabase();
	bool IsRunning() const { return m_bRunning; }
	bool IsLoaded() const { return m_bLoaded; }
};