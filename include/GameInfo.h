#pragma once
#include <string>

class GameInfo
{
public:
	// these are filled on init
	std::string m_strBaseUrl;
	std::string m_strLoginPage;
	std::string m_strRegisterPage;
	std::string m_strBuildingsPage;
	GameInfo()
		: m_strBaseUrl()
		, m_strLoginPage()
		, m_strRegisterPage()
		, m_strBuildingsPage()
	{}
	~GameInfo();
	bool Init();
	inline std::string GetBuildingsPageUrl() const
	{
		return m_strBaseUrl 
			+ m_strBuildingsPage;
	}
	inline std::string GetLoginPageUrl() const
	{
		return m_strBaseUrl + m_strLoginPage;
	}
	inline std::string GetRegisterPageUrl() const
	{
		return m_strBaseUrl + m_strRegisterPage;
	}
};