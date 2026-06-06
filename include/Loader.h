#pragma once

class Loader
{
public:
	Loader();
	~Loader();
	bool Init();
	bool LoadBotsFromDatabase();
	void ShutDown();
};