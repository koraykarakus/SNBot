#pragma once
#include <string>
#include <curl/curl.h>
#include "GameInfo.h"

struct bot_info {
    int id;
    std::string username;
    std::string email;
    int id_planet;
    int universe;
    int galaxy;
    int system;
    int planet;
    int darkmatter;
    int onlinetime;
    int vacation_mode;
    int vacation_until;
    int spy_tech;
    int computer_tech;
    int military_tech;
    int defence_tech;
    int shield_tech;
    int energy_tech;
    int hyperspace_tech;
    int combustion_tech;
    int impulse_motor_tech;
    int hyperspace_motor_tech;
    int laser_tech;
    int ionic_tech;
    int buster_tech;
    int intergalactic_tech;
    int expedition_tech;
    int metal_proc_tech;
    int crystal_proc_tech;
    int deuterium_proc_tech;
    int graviton_tech;
    int ally_id;
    
};

class Bot 
{
private:
    GameInfo* m_pGameInfo;
    std::string m_strEmail;
    std::string m_strPassword;
    CURL* m_curl;

    // Sunucudan gelen HTML yanıtlarını yakalamak için gereken statik fonksiyon
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    // Pointer referansı alarak içeriği tamamen sıfırlayacağız
    void CurlCleanUp(struct curl_slist*& pHeaders);
    void CurlStartPost(struct curl_slist*& pHeaders); 
public:
    Bot(GameInfo* pGameInfo,const std::string& strEmail, const std::string& strPassword);
    ~Bot();

    bool Init();
    bool Register(const std::string& strRegisterUrl, const std::string& strPostData);
    bool Login();

    bool SendBuilding(int iID);
};