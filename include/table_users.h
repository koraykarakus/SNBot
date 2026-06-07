#pragma once
#include <string>
#include <vector>
#include "table_planets.h"

struct table_users
{
    // Temel ve Giriş Bilgileri
    int id;
    std::string strUserName;

    // Altyapı ve Konum Bilgileri
    int id_planet;
    int universe;
    int galaxy;
    int system;
    int planet;

    // vacation
    int vacation_mode;
    int vacation_until;

    // Araştırma ve Kuyruk Bilgileri
    int b_tech_planet;
    int b_tech;
    int b_tech_id;
    std::string b_tech_queue;

    // technologies
    uint8_t resource[200];

    // Bot Durumu ve Gezegenleri
    bool is_bot;
    std::vector<table_planets> vecPlanets;

    // Constructor (Tüm alanlar güvenli değerlerle başlatılıyor)
    table_users()
        : id(0)
        , strUserName("")
        , id_planet(0)
        , universe(0)
        , galaxy(0)
        , system(0)
        , planet(0)
        , vacation_mode(0)
        , vacation_until(0)
        , b_tech_planet(0)
        , b_tech(0)
        , b_tech_id(0)
        , b_tech_queue("")
        , is_bot(false)
        , resource{0}
    {
    }
};