#include "GameInfo.h"

GameInfo::~GameInfo()
{

}

bool GameInfo::Init()
{
	// TODO Read from settings.ini file
	m_strBaseUrl = "http://localhost/steemnova/steemnova-1.8-x/";
	m_strLoginPage = "index.php?page=login&mode=validate&ajax=1";
	m_strRegisterPage = "index.php?page=register";
	m_strBuildingsPage = "game.php?page=buildings";

	return true;
}