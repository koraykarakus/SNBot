#pragma once

class CLoader
{
public:
	CLoader();
	~CLoader();
	bool Init();
	bool LoadBotsFromDatabase();
	bool LoadVarsFromDatabase();
	bool LoadConfigFromDatabase();
	bool SetBotFactors();
	void ShutDown();
};

extern CLoader* g_pLoader;
extern bool g_bLoaded;