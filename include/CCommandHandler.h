#pragma once
#include <string>

class CBotManager;
class CApplication;

class CCommandHandler
{
public:
	CBotManager* m_pBotManager;
	CCommandHandler();
	~CCommandHandler();
	// console
	void Run(CBotManager* pBotManager, CApplication& app);
	bool ProcessCommand(std::string& line, CApplication& app);
};