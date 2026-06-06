#pragma once
#include <string>

struct bot_planet_info
{
    // Temel Bilgiler ve Koordinatlar
    int id;
    std::string name;
    int id_owner;
    int universe;
    int galaxy;
    int system;
    int planet;
    int last_update;
    std::string planet_type; // ENUM için string
    int destroyed;

    // İnşaat Sıraları ve Görsel
    int b_building;
    std::string b_building_id;
    int b_shipyard;
    std::string b_shipyard_id;
    int b_shipyard_plus;
    std::string image;

    // Gezegen Yapısı ve Sıcaklık
    int diameter;
    int field_current;
    int field_max;
    int temp_min;
    int temp_max;

    // Madenler ve Enerji Bilgileri
    std::string eco_hash;
    double metal;
    double metal_perhour;
    double metal_max;
    double crystal;
    double crystal_perhour;
    double crystal_max;
    double deuterium;
    double deuterium_perhour;
    double deuterium_max;
    double energy_used;
    double energy;

    // Bina Kademeleri (Ekonomi / Altyapı)
    int metal_mine;
    int crystal_mine;
    int deuterium_synthesizer;
    int solar_plant;
    int fusion_plant;
    int robot_factory;
    int nanite_factory;
    int shipyard;
    int metal_storage;
    int crystal_storage;
    int deuterium_tank;
    int research_lab;
    int terraformer;
    int university;
    int ally_deposit;
    int missile_silo;

    // Ay Binaları
    int lunar_base;
    int phalanx;
    int jump_gate;

    // Filo (Büyük Sayılar İçin BIGINT -> unsigned long long)
    unsigned long long small_cargo;
    unsigned long long big_cargo;
    unsigned long long light_hunter;
    unsigned long long heavy_hunter;
    unsigned long long cruiser;
    unsigned long long battle_ship;
    unsigned long long colony_ship;
    unsigned long long recycler;
    unsigned long long espionage_probe;
    unsigned long long bomber_ship;
    unsigned long long solar_satellite;
    unsigned long long destroyer;
    unsigned long long death_star;
    unsigned long long battle_cruiser;
    unsigned long long black_moon;
    unsigned long long ev_transporter;
    unsigned long long star_crasher;
    unsigned long long giga_recycler;
    long long dm_ship; // Tabloda SIGNED BIGINT yapılmış
    unsigned long long orbital_station;

    // Savunma (BIGINT -> unsigned long long)
    unsigned long long rocket_launcher;
    unsigned long long light_laser;
    unsigned long long heavy_laser;
    unsigned long long gauss_cannon;
    unsigned long long ion_cannon;
    unsigned long long plasma_turret;
    int small_protection_shield; // Bunlar TINYINT
    int planet_protector;
    int big_protection_shield;
    unsigned long long graviton_cannon;
    unsigned long long interceptor_misil;
    unsigned long long interplanetary_misil;

    // Üretim Yüzdeleri (ENUM için string)
    std::string metal_mine_percent;
    std::string crystal_mine_percent;
    std::string deuterium_synthesizer_percent;
    std::string solar_plant_percent;
    std::string fusion_plant_percent;
    std::string solar_satellite_percent;

    // Ekstra Bot ve Sistem Bilgileri
    int last_jump_time;
    double debris_metal;
    double debris_crystal;
    int id_moon;
    bool is_bot;
    int last_relocate;
    unsigned long long version;

    // Constructor (Tüm verileri güvenli başlangıç değerlerine çekiyoruz)
    bot_planet_info()
        : id(0)
        , name("")
        , id_owner(0)
        , universe(0)
        , galaxy(0)
        , system(0)
        , planet(0)
        , last_update(0)
        , planet_type("1")
        , destroyed(0)
        , b_building(0)
        , b_building_id("")
        , b_shipyard(0)
        , b_shipyard_id("")
        , b_shipyard_plus(0)
        , image("")
        , diameter(0)
        , field_current(0)
        , field_max(0)
        , temp_min(0)
        , temp_max(0)
        , eco_hash("")
        , metal(0.0)
        , metal_perhour(0.0)
        , metal_max(0.0)
        , crystal(0.0)
        , crystal_perhour(0.0)
        , crystal_max(0.0)
        , deuterium(0.0)
        , deuterium_perhour(0.0)
        , deuterium_max(0.0)
        , energy_used(0.0)
        , energy(0.0)
        , metal_mine(0)
        , crystal_mine(0)
        , deuterium_synthesizer(0)
        , solar_plant(0)
        , fusion_plant(0)
        , robot_factory(0)
        , nanite_factory(0)
        , shipyard(0)
        , metal_storage(0)
        , crystal_storage(0)
        , deuterium_tank(0)
        , research_lab(0)
        , terraformer(0)
        , university(0)
        , ally_deposit(0)
        , missile_silo(0)
        , lunar_base(0)
        , phalanx(0)
        , jump_gate(0)
        , small_cargo(0)
        , big_cargo(0)
        , light_hunter(0)
        , heavy_hunter(0)
        , cruiser(0)
        , battle_ship(0)
        , colony_ship(0)
        , recycler(0)
        , espionage_probe(0)
        , bomber_ship(0)
        , solar_satellite(0)
        , destroyer(0)
        , death_star(0)
        , battle_cruiser(0)
        , black_moon(0)
        , ev_transporter(0)
        , star_crasher(0)
        , giga_recycler(0)
        , dm_ship(0)
        , orbital_station(0)
        , rocket_launcher(0)
        , light_laser(0)
        , heavy_laser(0)
        , gauss_cannon(0)
        , ion_cannon(0)
        , plasma_turret(0)
        , small_protection_shield(0)
        , planet_protector(0)
        , big_protection_shield(0)
        , graviton_cannon(0)
        , interceptor_misil(0)
        , interplanetary_misil(0)
        , metal_mine_percent("10")
        , crystal_mine_percent("10")
        , deuterium_synthesizer_percent("10")
        , solar_plant_percent("10")
        , fusion_plant_percent("10")
        , solar_satellite_percent("10")
        , last_jump_time(0)
        , debris_metal(0.0)
        , debris_crystal(0.0)
        , id_moon(0)
        , is_bot(false)
        , last_relocate(0)
        , version(0)
    {
    }
};

struct bot_info
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
    std::vector<bot_planet_info> vecPlanets;

    // Constructor (Tüm alanlar güvenli değerlerle başlatılıyor)
    bot_info()
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