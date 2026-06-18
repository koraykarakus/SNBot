#pragma once

// implemented for later use and cleanup, not used at the moment.

// 0 - 100 buildings
enum buildings_id {
    metal_mine = 1,
    crystal_mine = 2,
    deuterium_synthesizer = 3,
    solar_plant = 4,
    university = 6,
    fusion_plant = 12,
    robot_factory = 14,
    nanite_factory = 15,
    shipyard = 21,
    metal_storage = 22,
    crystal_storage = 23,
    deuterium_tank = 24,
    research_lab = 31,
    terraformer = 33,
    ally_deposit = 34,
    lunar_base = 41,
    phalanx = 42,
    jump_gate = 43,
    missile_silo = 44
};

// 100 - 200 researches
enum research_id {
    spy_tech = 106,
    computer_tech = 108,
    military_tech = 109,
    armor_tech = 110,
    shield_tech = 111,
    energy_tech = 113,
    hyperspace_tech = 114,
    combustion_tech = 115,
    impulse_motor_tech = 117,
    hyperspace_motor_tech = 118,
    laser_tech = 120,
    ion_tech = 121,
    plasma_tech = 122,
    intergalactic_tech = 123,
    expedition_tech = 124,
    metal_proc_tech = 131,
    crystal_proc_tech = 132,
    deuterium_proc_tech = 133,
    graviton_tech = 199
};

// 200 - 300 ships
enum ships_id {
    small_cargo = 202,
    big_cargo = 203,
    light_hunter = 204,
    heavy_hunter = 205,
    cruiser = 206,
    battle_ship = 207,
    colony_ship = 208,
    recycler = 209,
    espionage_probe = 210,
    bomber_ship = 211,
    solar_satellite = 212,
    destroyer = 213,
    death_star = 214,
    battle_cruiser = 215,
    black_moon = 216,
    ev_transporter = 217,
    star_crasher = 218,
    giga_recycler = 219,
    dm_ship = 220
};

// 300 - 400 defences
enum defences_id {
    rocket_launcher = 301, // 401 olan id'ler belirttiğin gibi 300-400 aralığına çekildi
    light_laser = 302,
    heavy_laser = 303,
    gauss_cannon = 304,
    ion_cannon = 305,
    plasma_turret = 306,
    small_protection_shield = 307,
    big_protection_shield = 308,
    planet_protector = 309,
    graviton_cannon = 310,
    orbital_station = 311
};

// 500 - 600 missiles
enum missiles_id {
    interceptor_misil = 502,
    interplanetary_misil = 503
};

// 600 - 700 officers
enum officers_id {
    rpg_geologist = 601, rpg_admiral = 602, rpg_engineer = 603, rpg_technocrat = 604,
    rpg_constructor = 605, rpg_scientist = 606, rpg_stocker = 607, rpg_defender = 608,
    rpg_bunker = 609, rpg_espion = 610, rpg_commander = 611, rpg_destructor = 612,
    rpg_general = 613, rpg_raider = 614, rpg_emperor = 615
};