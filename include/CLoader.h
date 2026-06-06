#pragma once

class CLoader
{
public:
	CLoader();
	~CLoader();
	bool Init();
	bool LoadBotsFromDatabase();
	bool LoadVarsFromDatabase();
	void ShutDown();
};

extern CLoader g_Loader;