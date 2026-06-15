#pragma once
#include <string>
#include <unordered_map>

class CBotManager;
class CApplication;

class CCommandHandler
{
public:
	const std::unordered_map<std::string, std::string>& lang_;
	CBotManager* bot_manager_;
	CCommandHandler(
		const std::unordered_map<std::string, std::string>& lang
	);
	~CCommandHandler();
	// console
	void Run(CBotManager* pBotManager, CApplication& app);
	bool ProcessCommand(std::string& line, CApplication& app);
};