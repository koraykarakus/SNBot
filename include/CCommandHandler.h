#pragma once
#include <string>
#include <unordered_map>

class CBotManager;
class CApplication;
class CLanguage;

class CCommandHandler
{
public:
	std::unordered_map<std::string, std::string>* lang_;
	CBotManager* bot_manager_;
	CCommandHandler(CLanguage* language, CBotManager* botManager);
	~CCommandHandler();
	// console
	void Run(CApplication& app);
	bool ProcessCommand(std::string& line, CApplication& app);
};