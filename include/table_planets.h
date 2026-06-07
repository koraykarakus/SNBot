#pragma once
#include <string>
#include <array>
#include "enums.h"

struct table_planets
{
    int id;
    std::string name;
    int id_owner;
    int universe;
    int galaxy;
    int system;
    int planet;
    int last_update;
    int planet_type;
    int destroyed;

    int b_building;
    std::string b_building_id;
    int b_shipyard;
    std::string b_shipyard_id;
    int b_shipyard_plus;
    std::string image;

    int diameter;
    int field_current;
    int field_max;
    int temp_min;
    int temp_max;

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

    // =========================================================================
    // all data related to planet, ships, buildings, defences, missiles
    // =========================================================================
    unsigned long long resource[1000] = { 0 };

    // production
    std::string metal_mine_percent;
    std::string crystal_mine_percent;
    std::string deuterium_synthesizer_percent;
    std::string solar_plant_percent;
    std::string fusion_plant_percent;
    std::string solar_satellite_percent;

    int last_jump_time;
    double debris_metal;
    double debris_crystal;
    int id_moon;
    bool is_bot;
    int last_relocate;
    unsigned long long version;

    // Constructor
    table_planets()
        : id(0), name(""), id_owner(0), universe(0)
        , galaxy(0), system(0), planet(0), last_update(0)
        , planet_type(1), destroyed(0), b_building(0)
        , b_building_id(""), b_shipyard(0), b_shipyard_id("")
        , b_shipyard_plus(0), image(""), diameter(0), field_current(0)
        , field_max(0), temp_min(0), temp_max(0)
        , eco_hash(""), metal(0.0), metal_perhour(0.0)
        , metal_max(0.0), crystal(0.0), crystal_perhour(0.0)
        , crystal_max(0.0), deuterium(0.0), deuterium_perhour(0.0)
        , deuterium_max(0.0), energy_used(0.0), energy(0.0)
        , metal_mine_percent("10"), crystal_mine_percent("10")
        , deuterium_synthesizer_percent("10")
        , solar_plant_percent("10"), fusion_plant_percent("10")
        , solar_satellite_percent("10")
        , last_jump_time(0), debris_metal(0.0)
        , debris_crystal(0.0), id_moon(0)
        , is_bot(false)
        , last_relocate(0)
        , version(0)
    {
    }
};