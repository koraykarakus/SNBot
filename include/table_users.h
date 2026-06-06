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

    // Araştırma ve Kuyruk Bilgileri
    int b_tech_planet;
    int b_tech;
    int b_tech_id;
    std::string b_tech_queue;

    // Teknolojiler
    int spy_tech;
    int computer_tech;
    int military_tech;
    int armor_tech;
    int shield_tech;
    int energy_tech;
    int hyperspace_tech;
    int combustion_tech;
    int impulse_motor_tech;
    int hyperspace_motor_tech;
    int laser_tech;
    int ion_tech;
    int plasma_tech;
    int intergalactic_tech;
    int expedition_tech;
    int metal_proc_tech;
    int crystal_proc_tech;
    int deuterium_proc_tech;
    int graviton_tech;

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
        , b_tech_planet(0)
        , b_tech(0)
        , b_tech_id(0)
        , b_tech_queue("")
        , spy_tech(0)
        , computer_tech(0)
        , military_tech(0)
        , armor_tech(0)
        , shield_tech(0)
        , energy_tech(0)
        , hyperspace_tech(0)
        , combustion_tech(0)
        , impulse_motor_tech(0)
        , hyperspace_motor_tech(0)
        , laser_tech(0)
        , ion_tech(0)
        , plasma_tech(0)
        , intergalactic_tech(0)
        , expedition_tech(0)
        , metal_proc_tech(0)
        , crystal_proc_tech(0)
        , deuterium_proc_tech(0)
        , graviton_tech(0)
        , is_bot(false)
    {
    }
};