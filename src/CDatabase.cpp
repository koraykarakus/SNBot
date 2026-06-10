#include "CDatabase.h"
#include "CLogger.h"
#include "CBotManager.h"
#include "SimpleIni.h"
#include "table_users.h"
#include <fmt/ranges.h>
#include <filesystem>

CDatabase::CDatabase()
    : m_pConn(nullptr)
    , m_strDBUser()
    , m_strDBPass()
    , m_strDBHost()
    , m_strDBName()
    , m_strDBPrefix()
{
    Init();
}

CDatabase::~CDatabase()
{
    Disconnect();
}

void CDatabase::Init()
{
    // find path to settings.ini file which is in the same folder with exe
    std::filesystem::path iniPath = std::filesystem::current_path() / "settings.ini";
    std::string iniPathStr = iniPath.string();

    CSimpleIniA ini;
    ini.SetUnicode(); // support special char set

    // load ini file
    SI_Error rc = ini.LoadFile(iniPathStr.c_str());

    // create settings.ini if it is not found
    if (rc < 0) 
    {
        CLogger::Error("settings.ini not found check folder and fill required info !\n");

        // defaults
        ini.SetValue("Database", "Host", "localhost");
        ini.SetValue("Database", "User", "username");
        ini.SetValue("Database", "Password", "password");
        ini.SetValue("Database", "DBName", "steemnova");
        ini.SetValue("Database", "Prefix", "uni1_"); 

        // save defaults to ini
        rc = ini.SaveFile(iniPathStr.c_str());
        if (rc < 0) 
        {
            CLogger::Error("error : settings.ini cannot be saved !\n");
        }
    }

    // get values and assign members.
    m_strDBHost = ini.GetValue("Database", "Host");
    m_strDBUser = ini.GetValue("Database", "User");
    m_strDBPass = ini.GetValue("Database", "Password");
    m_strDBName = ini.GetValue("Database", "DBName");
    m_strDBPrefix = ini.GetValue("Database", "Prefix");

    CLogger::Info("[DBAgent] settings read from settings.ini Host: {}", m_strDBHost);
}

bool CDatabase::Connect()
{
    m_pConn = mysql_init(nullptr);

    if (m_pConn == nullptr)
    {
        CLogger::Error("[CDatabase] - mysql_init failed");
        return false;
    }

    // close ssl
    int ssl_verify = 0; 
    mysql_optionsv(m_pConn, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &ssl_verify);
    // ----------------------------------------

    if (!mysql_real_connect(
        m_pConn,
        m_strDBHost.c_str(),
        m_strDBUser.c_str(),
        m_strDBPass.c_str(),
        m_strDBName.c_str(),
        3306,
        nullptr,
        0))
    {
        CLogger::Error("[CDatabase] - MySQL Connection Failed. Error: {} ({})", mysql_error(m_pConn), mysql_errno(m_pConn));

        // close db always
        mysql_close(m_pConn);
        m_pConn = nullptr;
        return false;
    }

    return true;
}

void CDatabase::Disconnect()
{
    if (m_pConn != nullptr)
    {
        mysql_close(m_pConn);
        m_pConn = nullptr;
    }
}

bool CDatabase::LoadBots()
{
    auto start = GetTimeNow();
	std::string strQuery = "SELECT * FROM `" + m_strDBPrefix + "users` WHERE is_bot = 1";

    if (mysql_query(m_pConn, strQuery.c_str()))
    {
        CLogger::Error("[CDatabase] - Query Error (Users): {}", mysql_error(m_pConn));
        return false;
    }

    MYSQL_RES* result = mysql_store_result(m_pConn);
    if (result == nullptr)
    {
        if (mysql_field_count(m_pConn) > 0) 
        {
            CLogger::Error("[CDatabase] - Retrieve result error (Users): {}", mysql_error(m_pConn));
        }

        return false;
    }

    uint64_t rowCount = mysql_num_rows(result);
    CLogger::Info("[CDatabase] - Found {} bots in database.", rowCount);

    g_pBotManager->ClearBots();

    MYSQL_ROW row;

    time_t now = std::time(nullptr); 
    int ibotCounter = 0; int iPlanetCounter = 0;
    while ((row = mysql_fetch_row(result)))
    {
        table_users bot;
        
        bot.id = row[0] ? std::stoi(row[0]) : 0;
        bot.type = bot.id % 10;
        bot.SetPlayStyle();
        bot.strUserName = row[1] ? row[1] : "";
        bot.email = row[3] ? row[3] : "";

        // fill the struct with respect to index order.
        bot.id_planet = row[9] ? std::stoi(row[9]) : 0;
        bot.universe = row[10] ? std::stoi(row[10]) : 0;
        bot.galaxy = row[11] ? std::stoi(row[11]) : 0;
        bot.system = row[12] ? std::stoi(row[12]) : 0;
        bot.planet = row[13] ? std::stoi(row[13]) : 0;
        bot.onlinetime = row[18] ? std::stoi(row[18]) : 0;
        bot.vacation_mode = row[29] ? std::stoi(row[29]) : 0;
        bot.vacation_until = row[30] ? std::stoi(row[30]) : 0;

        // Research queue starts at index 32
        bot.b_tech_planet = row[32] ? std::stoi(row[32]) : 0;
        bot.b_tech = row[33] ? std::stoi(row[33]) : 0;
        bot.b_tech_id = row[34] ? std::stoi(row[34]) : 0;
        bot.b_tech_queue = row[35] ? row[35] : "";

        // technologies
        bot.resource[106] = row[36] ? std::stoi(row[36]) : 0;
        bot.resource[108] = row[37] ? std::stoi(row[37]) : 0;
        bot.resource[109] = row[38] ? std::stoi(row[38]) : 0;
        bot.resource[110] = row[39] ? std::stoi(row[39]) : 0;
        bot.resource[111] = row[40] ? std::stoi(row[40]) : 0;
        bot.resource[113] = row[41] ? std::stoi(row[41]) : 0;
        bot.resource[114] = row[42] ? std::stoi(row[42]) : 0;
        bot.resource[115] = row[43] ? std::stoi(row[43]) : 0;
        bot.resource[117] = row[44] ? std::stoi(row[44]) : 0;
        bot.resource[118] = row[45] ? std::stoi(row[45]) : 0;
        bot.resource[120] = row[46] ? std::stoi(row[46]) : 0;
        bot.resource[121] = row[47] ? std::stoi(row[47]) : 0;
        bot.resource[122] = row[48] ? std::stoi(row[48]) : 0;
        bot.resource[123] = row[49] ? std::stoi(row[49]) : 0;
        bot.resource[124] = row[50] ? std::stoi(row[50]) : 0;
        bot.resource[131] = row[51] ? std::stoi(row[51]) : 0;
        bot.resource[132] = row[52] ? std::stoi(row[52]) : 0;
        bot.resource[133] = row[53] ? std::stoi(row[53]) : 0;
        bot.resource[199] = row[54] ? std::stoi(row[54]) : 0;

        // column is_bot to simplify
        bot.is_bot = row[99] ? (std::atoi(row[99]) != 0) : false;
        bot.SetFactor(now);
        g_pBotManager->AddBot(bot);
        ++ibotCounter;
    }
    mysql_free_result(result);
    
    // if bot number is 0, no point searching for bot planets
    if (rowCount == 0) 
    {
        return true;
    }

    // -------------------------------------------------------------------------
    // STEP 2: Load Bot planets and match them
    // -------------------------------------------------------------------------
    std::string strPlanetQuery = "SELECT * FROM `" + m_strDBPrefix + "planets` WHERE is_bot = 1";

    if (mysql_query(m_pConn, strPlanetQuery.c_str()))
    {
        CLogger::Error("[CDatabase] - Query Error (Planets): {}", mysql_error(m_pConn));
        return false;
    }

    MYSQL_RES* plResult = mysql_store_result(m_pConn);
    if (plResult == nullptr)
    {
        if (mysql_field_count(m_pConn) > 0) 
        {
            CLogger::Error("[CDatabase] - Retrieve result error (Planets): {}", mysql_error(m_pConn));
        }

        return false;
    }

    MYSQL_ROW plRow;
    while ((plRow = mysql_fetch_row(plResult)))
    {
        table_planets pl;

        pl.id = plRow[0] ? std::stoi(plRow[0]) : 0;
        pl.name = plRow[1] ? plRow[1] : "";
        pl.id_owner = plRow[2] ? std::stoi(plRow[2]) : 0; 
        pl.universe = plRow[3] ? std::stoi(plRow[3]) : 0;
        pl.galaxy = plRow[4] ? std::stoi(plRow[4]) : 0;
        pl.system = plRow[5] ? std::stoi(plRow[5]) : 0;
        pl.planet = plRow[6] ? std::stoi(plRow[6]) : 0;
        pl.last_update = plRow[7] ? std::stoi(plRow[7]) : 0;
        pl.planet_type = plRow[8] ? std::stoi(plRow[8]) : 1;
        pl.destroyed = plRow[9] ? std::stoi(plRow[9]) : 0;

        pl.b_building = plRow[10] ? std::stoi(plRow[10]) : 0;
        pl.b_building_id = plRow[11] ? plRow[11] : "";
        pl.b_shipyard = plRow[12] ? std::stoi(plRow[12]) : 0;
        pl.b_shipyard_id = plRow[13] ? plRow[13] : "";
        pl.b_shipyard_plus = plRow[14] ? std::stoi(plRow[14]) : 0;
        pl.image = plRow[15] ? plRow[15] : "";

        pl.diameter = plRow[16] ? std::stoi(plRow[16]) : 0;
        pl.field_current = plRow[17] ? std::stoi(plRow[17]) : 0;
        pl.field_max = plRow[18] ? std::stoi(plRow[18]) : 0;
        pl.temp_min = plRow[19] ? std::stoi(plRow[19]) : 0;
        pl.temp_max = plRow[20] ? std::stoi(plRow[20]) : 0;

        pl.eco_hash = plRow[21] ? plRow[21] : "";
        // DOUBLE kolonları için std::stod (string to double) kullanıyoruz:
        pl.metal = plRow[22] ? std::stod(plRow[22]) : 0.0;
        pl.metal_perhour = plRow[23] ? std::stod(plRow[23]) : 0.0;
        pl.metal_max = plRow[24] ? std::stod(plRow[24]) : 0.0;
        pl.crystal = plRow[25] ? std::stod(plRow[25]) : 0.0;
        pl.crystal_perhour = plRow[26] ? std::stod(plRow[26]) : 0.0;
        pl.crystal_max = plRow[27] ? std::stod(plRow[27]) : 0.0;
        pl.deuterium = plRow[28] ? std::stod(plRow[28]) : 0.0;
        pl.deuterium_perhour = plRow[29] ? std::stod(plRow[29]) : 0.0;
        pl.deuterium_max = plRow[30] ? std::stod(plRow[30]) : 0.0;
        pl.energy_used = plRow[31] ? std::stod(plRow[31]) : 0.0;
        pl.energy = plRow[32] ? std::stod(plRow[32]) : 0.0;

        // buildings
        pl.resource[1] = plRow[33] ? std::stoull(plRow[33]) : 0; // metal_mine
        pl.resource[2] = plRow[34] ? std::stoull(plRow[34]) : 0; // crystal_mine
        pl.resource[3] = plRow[35] ? std::stoull(plRow[35]) : 0; // deuterium_synthesizer
        pl.resource[4] = plRow[36] ? std::stoull(plRow[36]) : 0; // solar_plant
        pl.resource[12] = plRow[37] ? std::stoull(plRow[37]) : 0; // fusion_plant
        pl.resource[14] = plRow[38] ? std::stoull(plRow[38]) : 0; // robot_factory
        pl.resource[15] = plRow[39] ? std::stoull(plRow[39]) : 0; // nanite_factory
        pl.resource[21] = plRow[40] ? std::stoull(plRow[40]) : 0; // shipyard
        pl.resource[22] = plRow[41] ? std::stoull(plRow[41]) : 0; // metal_storage
        pl.resource[23] = plRow[42] ? std::stoull(plRow[42]) : 0; // crystal_storage
        pl.resource[24] = plRow[43] ? std::stoull(plRow[43]) : 0; // deuterium_tank
        pl.resource[31] = plRow[44] ? std::stoull(plRow[44]) : 0; // research_lab
        pl.resource[33] = plRow[45] ? std::stoull(plRow[45]) : 0; // terraformer
        pl.resource[6] = plRow[46] ? std::stoull(plRow[46]) : 0; // university
        pl.resource[34] = plRow[47] ? std::stoull(plRow[47]) : 0; // ally_deposit
        pl.resource[44] = plRow[48] ? std::stoull(plRow[48]) : 0; // missile_silo
        pl.resource[41] = plRow[49] ? std::stoull(plRow[49]) : 0; // lunar_base
        pl.resource[42] = plRow[50] ? std::stoull(plRow[50]) : 0; // phalanx
        pl.resource[43] = plRow[51] ? std::stoull(plRow[51]) : 0; // jump_gate

        // fleet
        pl.resource[202] = plRow[52] ? std::stoull(plRow[52]) : 0; // small_cargo
        pl.resource[203] = plRow[53] ? std::stoull(plRow[53]) : 0; // big_cargo
        pl.resource[204] = plRow[54] ? std::stoull(plRow[54]) : 0; // light_hunter
        pl.resource[205] = plRow[55] ? std::stoull(plRow[55]) : 0; // heavy_hunter
        pl.resource[206] = plRow[56] ? std::stoull(plRow[56]) : 0; // cruiser
        pl.resource[207] = plRow[57] ? std::stoull(plRow[57]) : 0; // battle_ship
        pl.resource[208] = plRow[58] ? std::stoull(plRow[58]) : 0; // colony_ship
        pl.resource[209] = plRow[59] ? std::stoull(plRow[59]) : 0; // recycler
        pl.resource[210] = plRow[60] ? std::stoull(plRow[60]) : 0; // espionage_probe
        pl.resource[211] = plRow[61] ? std::stoull(plRow[61]) : 0; // bomber_ship
        pl.resource[212] = plRow[62] ? std::stoull(plRow[62]) : 0; // solar_satellite
        pl.resource[213] = plRow[63] ? std::stoull(plRow[63]) : 0; // destroyer
        pl.resource[214] = plRow[64] ? std::stoull(plRow[64]) : 0; // death_star
        pl.resource[215] = plRow[65] ? std::stoull(plRow[65]) : 0; // battle_cruiser
        pl.resource[216] = plRow[66] ? std::stoull(plRow[66]) : 0; // black_moon
        pl.resource[217] = plRow[67] ? std::stoull(plRow[67]) : 0; // ev_transporter
        pl.resource[218] = plRow[68] ? std::stoull(plRow[68]) : 0; // star_crasher
        pl.resource[219] = plRow[69] ? std::stoull(plRow[69]) : 0; // giga_recycler
        pl.resource[220] = plRow[70] ? std::stoull(plRow[70]) : 0; // dm_ship
        pl.resource[411] = plRow[71] ? std::stoull(plRow[71]) : 0; // orbital_station

        // defence & missiles
        pl.resource[401] = plRow[72] ? std::stoull(plRow[72]) : 0; // rocket_launcher
        pl.resource[402] = plRow[73] ? std::stoull(plRow[73]) : 0; // light_laser
        pl.resource[403] = plRow[74] ? std::stoull(plRow[74]) : 0; // heavy_laser
        pl.resource[404] = plRow[75] ? std::stoull(plRow[75]) : 0; // gauss_cannon
        pl.resource[405] = plRow[76] ? std::stoull(plRow[76]) : 0; // ion_cannon
        pl.resource[406] = plRow[77] ? std::stoull(plRow[77]) : 0; // plasma_turret
        pl.resource[407] = plRow[78] ? std::stoull(plRow[78]) : 0; // small_protection_shield
        pl.resource[409] = plRow[79] ? std::stoull(plRow[79]) : 0; // planet_protector
        pl.resource[408] = plRow[80] ? std::stoull(plRow[80]) : 0; // big_protection_shield
        pl.resource[410] = plRow[81] ? std::stoull(plRow[81]) : 0; // graviton_cannon
        pl.resource[502] = plRow[82] ? std::stoull(plRow[82]) : 0; // interceptor_misil
        pl.resource[503] = plRow[83] ? std::stoull(plRow[83]) : 0; // interplanetary_misil

        // production
        pl.metal_mine_percent = plRow[84] ? plRow[84] : "10";
        pl.crystal_mine_percent = plRow[85] ? plRow[85] : "10";
        pl.deuterium_synthesizer_percent = plRow[86] ? plRow[86] : "10";
        pl.solar_plant_percent = plRow[87] ? plRow[87] : "10";
        pl.fusion_plant_percent = plRow[88] ? plRow[88] : "10";
        pl.solar_satellite_percent = plRow[89] ? plRow[89] : "10";

        // galaxy related
        pl.last_jump_time = plRow[90] ? std::stoi(plRow[90]) : 0;
        pl.debris_metal = plRow[91] ? std::stod(plRow[91]) : 0.0;
        pl.debris_crystal = plRow[92] ? std::stod(plRow[92]) : 0.0;
        pl.id_moon = plRow[93] ? std::stoi(plRow[93]) : 0;
        pl.is_bot = plRow[94] ? (std::stoi(plRow[94]) != 0) : false;
        pl.last_relocate = plRow[95] ? std::stoi(plRow[95]) : 0;
        pl.version = plRow[96] ? std::stoull(plRow[96]) : 0;

        // find bot with planet id_owner
        table_users* pTargetBot = g_pBotManager->GetBotRef(pl.id_owner);
        if (pTargetBot != nullptr)
        {
            pTargetBot->vecPlanets.push_back(std::move(pl));
            ++iPlanetCounter;
        }
        else
        {
            CLogger::Warn("Planet ID {} has owner ID {} but no matching bot was found!", pl.id, pl.id_owner);
        }
    }
   
    mysql_free_result(plResult);
   
    auto end = GetTimeNow();
    auto duration_micros = GetElapsedMicroseconds(start, end);
    double duration_millis = GetElapsedMilliseconds(start, end);
	CLogger::Info("[CDatabase] ### {} bots and {} planets"
		"loaded successfully. ### [time:{} us / {} ms]\n",
		ibotCounter, iPlanetCounter, duration_micros, duration_millis);
    return true;
}

bool CDatabase::UpdateBots()
{
    auto start = GetTimeNow();
    std::vector<table_users>& pBots = g_pBotManager->GetBots();

    if (pBots.empty())
    {
        return true;
    }

    if (m_pConn == nullptr) 
    {
        return false;
    }

    const size_t BATCH_SIZE = 50; // update amount per query

    // =========================================================================
    // PART 1: UPDATE BOT RESEARCH (USERS TABLE)
    // =========================================================================
    const std::string strUserHeader = "INSERT INTO `" + m_strDBPrefix + "users` ("
        "id, b_tech_planet, b_tech, b_tech_id, b_tech_queue, spy_tech, computer_tech, "
        "military_tech, armor_tech, shield_tech, energy_tech, hyperspace_tech, "
        "combustion_tech, impulse_motor_tech, hyperspace_motor_tech, laser_tech, "
        "ion_tech, plasma_tech, intergalactic_tech, expedition_tech, "
        "metal_proc_tech, crystal_proc_tech, deuterium_proc_tech, graviton_tech, onlinetime) VALUES ";

    const std::string strUserFooter = " ON DUPLICATE KEY UPDATE "
        "b_tech_planet = VALUES(b_tech_planet), b_tech = VALUES(b_tech), b_tech_id = VALUES(b_tech_id), b_tech_queue = VALUES(b_tech_queue), "
        "spy_tech = VALUES(spy_tech), computer_tech = VALUES(computer_tech), military_tech = VALUES(military_tech), armor_tech = VALUES(armor_tech), "
        "shield_tech = VALUES(shield_tech), energy_tech = VALUES(energy_tech), hyperspace_tech = VALUES(hyperspace_tech), combustion_tech = VALUES(combustion_tech), "
        "impulse_motor_tech = VALUES(impulse_motor_tech), hyperspace_motor_tech = VALUES(hyperspace_motor_tech), laser_tech = VALUES(laser_tech), "
        "ion_tech = VALUES(ion_tech), plasma_tech = VALUES(plasma_tech), intergalactic_tech = VALUES(intergalactic_tech), expedition_tech = VALUES(expedition_tech), "
        "metal_proc_tech = VALUES(metal_proc_tech), crystal_proc_tech = VALUES(crystal_proc_tech), deuterium_proc_tech = VALUES(deuterium_proc_tech), "
        "graviton_tech = VALUES(graviton_tech), onlinetime = VALUES(onlinetime);";

    // store all planets in same vector
    std::vector<table_planets> vecAllPlanets;
    std::vector<int> vecIDPlanetsNoUpdate;
    std::vector<int> vecIDBotsNoUpdate;

    int iPlanetCounter = 0, iBotCounter = 0;
    for (size_t i = 0; i < pBots.size(); i += BATCH_SIZE)
    {
        std::string strQuery = strUserHeader;
        size_t endIndex = ((i + BATCH_SIZE) < pBots.size()) ? (i + BATCH_SIZE) : pBots.size();

        for (size_t k = i; k < endIndex; ++k)
        {
            auto& cBot = pBots[k];

            for (auto& planet : cBot.vecPlanets) 
            {

                if (!planet.need_update)
                {
                    //CLogger::Info("PID:{} does not need an update", planet.id);
                    vecIDPlanetsNoUpdate.push_back(planet.id);
                    continue;
                }
                
                planet.need_update = false;
                vecAllPlanets.push_back(planet);
            }

            // planet might need update.. (finished building etc.)
            if (!cBot.need_update)
            {
                //CLogger::Info("Bot does not need update {}", cBot.id);
                vecIDBotsNoUpdate.push_back(cBot.id);
                continue;
            }

            strQuery += "(";
            strQuery += std::to_string(cBot.id) + ", ";
            strQuery += std::to_string(cBot.b_tech_planet) + ", ";
            strQuery += std::to_string(cBot.b_tech) + ", ";
            strQuery += std::to_string(cBot.b_tech_id) + ", ";
            strQuery += "'" + cBot.b_tech_queue + "', ";
            strQuery += std::to_string(cBot.resource[106]) + ", ";
            strQuery += std::to_string(cBot.resource[108]) + ", ";
            strQuery += std::to_string(cBot.resource[109]) + ", ";
            strQuery += std::to_string(cBot.resource[110]) + ", ";
            strQuery += std::to_string(cBot.resource[111]) + ", ";
            strQuery += std::to_string(cBot.resource[113]) + ", ";
            strQuery += std::to_string(cBot.resource[114]) + ", ";
            strQuery += std::to_string(cBot.resource[115]) + ", ";
            strQuery += std::to_string(cBot.resource[117]) + ", ";
            strQuery += std::to_string(cBot.resource[118]) + ", ";
            strQuery += std::to_string(cBot.resource[120]) + ", ";
            strQuery += std::to_string(cBot.resource[121]) + ", ";
            strQuery += std::to_string(cBot.resource[122]) + ", ";
            strQuery += std::to_string(cBot.resource[123]) + ", ";
            strQuery += std::to_string(cBot.resource[124]) + ", ";
            strQuery += std::to_string(cBot.resource[131]) + ", ";
            strQuery += std::to_string(cBot.resource[132]) + ", ";
            strQuery += std::to_string(cBot.resource[133]) + ", ";
            strQuery += std::to_string(cBot.resource[199]) + ", ";
            strQuery += std::to_string(cBot.onlinetime);
            strQuery += "), ";

            cBot.need_update = false;
            ++iBotCounter;
        }

        if (strQuery != strUserHeader)
        {
			if (strQuery.size() >= 2
				&& strQuery.substr(strQuery.size() - 2) == ", ")
			{
				// remove comma and space
				strQuery.pop_back();
				strQuery.pop_back();
			}

            strQuery += strUserFooter;

            if (mysql_query(m_pConn, strQuery.c_str()) != 0)
            {
                CLogger::Error("[CDatabase] - UpdateBots (Users) Paket Hatasi (Index {}-{}): {}", i, endIndex - 1, mysql_error(m_pConn));
                return false;
            }
        }

    }

    if (!vecIDPlanetsNoUpdate.empty()
        || !vecIDBotsNoUpdate.empty())
    {
        /*
        spdlog::info("PID:[{}], UID:[{}] does not need update",
            fmt::join(vecIDPlanetsNoUpdate, "],["),
            fmt::join(vecIDBotsNoUpdate, "],[")
        );*/
    }
    
    // =========================================================================
    // PART 2: UPDATE PLANETS (PLANETS TABLE) 
    // =========================================================================
    if (vecAllPlanets.empty())
    {
        return true;
    }

    // order is important !
    const std::string strPlanetHeader = "INSERT INTO `" + m_strDBPrefix + "planets` ("
        "id, name, id_owner, universe, galaxy, system, planet, last_update, planet_type, destroyed, "
        "b_building, b_building_id, b_shipyard, b_shipyard_id, b_shipyard_plus, image, diameter, field_current, field_max, temp_min, temp_max, "
        "eco_hash, metal, metal_perhour, metal_max, crystal, crystal_perhour, crystal_max, deuterium, deuterium_perhour, deuterium_max, energy_used, energy, "
        "metal_mine, crystal_mine, deuterium_synthesizer, solar_plant, fusion_plant, robot_factory, nanite_factory, shipyard, metal_storage, crystal_storage, deuterium_tank, research_lab, terraformer, university, ally_deposit, missile_silo, "
        "lunar_base, phalanx, jump_gate, small_cargo, big_cargo, light_hunter, heavy_hunter, cruiser, battle_ship, colony_ship, recycler, espionage_probe, bomber_ship, solar_satellite, destroyer, death_star, battle_cruiser, black_moon, ev_transporter, star_crasher, giga_recycler, dm_ship, orbital_station, "
        "rocket_launcher, light_laser, heavy_laser, gauss_cannon, ion_cannon, plasma_turret, small_protection_shield, planet_protector, big_protection_shield, graviton_cannon, interceptor_misil, interplanetary_misil, "
        "metal_mine_percent, crystal_mine_percent, deuterium_synthesizer_percent, solar_plant_percent, fusion_plant_percent, solar_satellite_percent, "
        "last_jump_time, debris_metal, debris_crystal, id_moon, is_bot, last_relocate, version) VALUES ";

    // footer
    const std::string strPlanetFooter = " ON DUPLICATE KEY UPDATE "
        "name=VALUES(name), id_owner=VALUES(id_owner), universe=VALUES(universe), galaxy=VALUES(galaxy), system=VALUES(system), planet=VALUES(planet), last_update=VALUES(last_update), planet_type=VALUES(planet_type), destroyed=VALUES(destroyed), "
        "b_building=VALUES(b_building), b_building_id=VALUES(b_building_id), b_shipyard=VALUES(b_shipyard), b_shipyard_id=VALUES(b_shipyard_id), b_shipyard_plus=VALUES(b_shipyard_plus), image=VALUES(image), diameter=VALUES(diameter), field_current=VALUES(field_current), field_max=VALUES(field_max), temp_min=VALUES(temp_min), temp_max=VALUES(temp_max), "
        "eco_hash=VALUES(eco_hash), metal=VALUES(metal), metal_perhour=VALUES(metal_perhour), metal_max=VALUES(metal_max), crystal=VALUES(crystal), crystal_perhour=VALUES(crystal_perhour), crystal_max=VALUES(crystal_max), deuterium=VALUES(deuterium), deuterium_perhour=VALUES(deuterium_perhour), deuterium_max=VALUES(deuterium_max), energy_used=VALUES(energy_used), energy=VALUES(energy), "
        "metal_mine=VALUES(metal_mine), crystal_mine=VALUES(crystal_mine), deuterium_synthesizer=VALUES(deuterium_synthesizer), solar_plant=VALUES(solar_plant), fusion_plant=VALUES(fusion_plant), robot_factory=VALUES(robot_factory), nanite_factory=VALUES(nanite_factory), shipyard=VALUES(shipyard), metal_storage=VALUES(metal_storage), crystal_storage=VALUES(crystal_storage), deuterium_tank=VALUES(deuterium_tank), research_lab=VALUES(research_lab), terraformer=VALUES(terraformer), university=VALUES(university), ally_deposit=VALUES(ally_deposit), missile_silo=VALUES(missile_silo), "
        "lunar_base=VALUES(lunar_base), phalanx=VALUES(phalanx), jump_gate=VALUES(jump_gate), small_cargo=VALUES(small_cargo), big_cargo=VALUES(big_cargo), light_hunter=VALUES(light_hunter), heavy_hunter=VALUES(heavy_hunter), cruiser=VALUES(cruiser), battle_ship=VALUES(battle_ship), colony_ship=VALUES(colony_ship), recycler=VALUES(recycler), espionage_probe=VALUES(espionage_probe), bomber_ship=VALUES(bomber_ship), solar_satellite=VALUES(solar_satellite), destroyer=VALUES(destroyer), death_star=VALUES(death_star), battle_cruiser=VALUES(battle_cruiser), black_moon=VALUES(black_moon), ev_transporter=VALUES(ev_transporter), star_crasher=VALUES(star_crasher), giga_recycler=VALUES(giga_recycler), dm_ship=VALUES(dm_ship), orbital_station=VALUES(orbital_station), "
        "rocket_launcher=VALUES(rocket_launcher), light_laser=VALUES(light_laser), heavy_laser=VALUES(heavy_laser), gauss_cannon=VALUES(gauss_cannon), ion_cannon=VALUES(ion_cannon), plasma_turret=VALUES(plasma_turret), small_protection_shield=VALUES(small_protection_shield), planet_protector=VALUES(planet_protector), big_protection_shield=VALUES(big_protection_shield), graviton_cannon=VALUES(graviton_cannon), interceptor_misil=VALUES(interceptor_misil), interplanetary_misil=VALUES(interplanetary_misil), "
        "metal_mine_percent=VALUES(metal_mine_percent), crystal_mine_percent=VALUES(crystal_mine_percent), deuterium_synthesizer_percent=VALUES(deuterium_synthesizer_percent), solar_plant_percent=VALUES(solar_plant_percent), fusion_plant_percent=VALUES(fusion_plant_percent), solar_satellite_percent=VALUES(solar_satellite_percent), "
        "last_jump_time=VALUES(last_jump_time), debris_metal=VALUES(debris_metal), debris_crystal=VALUES(debris_crystal), id_moon=VALUES(id_moon), is_bot=VALUES(is_bot), last_relocate=VALUES(last_relocate), version=VALUES(version);";

    // batch size 50 as default
    for (size_t i = 0; i < vecAllPlanets.size(); i += BATCH_SIZE)
    {
        std::string strQuery = strPlanetHeader;
        size_t endIndex = ((i + BATCH_SIZE) < vecAllPlanets.size()) ? (i + BATCH_SIZE) : vecAllPlanets.size();

        for (size_t k = i; k < endIndex; ++k)
        {
            const auto& pl = vecAllPlanets[k];

            strQuery += "(";
            strQuery += std::to_string(pl.id) + ", ";
            strQuery += "'" + pl.name + "', ";
            strQuery += std::to_string(pl.id_owner) + ", ";
            strQuery += std::to_string(pl.universe) + ", ";
            strQuery += std::to_string(pl.galaxy) + ", ";
            strQuery += std::to_string(pl.system) + ", ";
            strQuery += std::to_string(pl.planet) + ", ";
            strQuery += std::to_string(pl.last_update) + ", ";
            strQuery += std::to_string(pl.planet_type) + ", ";
            strQuery += std::to_string(pl.destroyed) + ", ";

            strQuery += std::to_string(pl.b_building) + ", ";
            strQuery += "'" + pl.b_building_id + "', ";
            strQuery += std::to_string(pl.b_shipyard) + ", ";
            strQuery += "'" + pl.b_shipyard_id + "', ";
            strQuery += std::to_string(pl.b_shipyard_plus) + ", ";
            strQuery += "'" + pl.image + "', ";
            strQuery += std::to_string(pl.diameter) + ", ";
            strQuery += std::to_string(pl.field_current) + ", ";
            strQuery += std::to_string(pl.field_max) + ", ";
            strQuery += std::to_string(pl.temp_min) + ", ";
            strQuery += std::to_string(pl.temp_max) + ", ";

            strQuery += "'" + pl.eco_hash + "', ";
            strQuery += std::to_string(pl.metal) + ", ";
            strQuery += std::to_string(pl.metal_perhour) + ", ";
            strQuery += std::to_string(pl.metal_max) + ", ";
            strQuery += std::to_string(pl.crystal) + ", ";
            strQuery += std::to_string(pl.crystal_perhour) + ", ";
            strQuery += std::to_string(pl.crystal_max) + ", ";
            strQuery += std::to_string(pl.deuterium) + ", ";
            strQuery += std::to_string(pl.deuterium_perhour) + ", ";
            strQuery += std::to_string(pl.deuterium_max) + ", ";
            strQuery += std::to_string(pl.energy_used) + ", ";
            strQuery += std::to_string(pl.energy) + ", ";

            strQuery += std::to_string(pl.resource[1]) + ", ";
            strQuery += std::to_string(pl.resource[2]) + ", ";
            strQuery += std::to_string(pl.resource[3]) + ", ";
            strQuery += std::to_string(pl.resource[4]) + ", ";
            strQuery += std::to_string(pl.resource[12]) + ", ";
            strQuery += std::to_string(pl.resource[14]) + ", ";
            strQuery += std::to_string(pl.resource[15]) + ", ";
            strQuery += std::to_string(pl.resource[21]) + ", ";
            strQuery += std::to_string(pl.resource[22]) + ", ";
            strQuery += std::to_string(pl.resource[23]) + ", ";
            strQuery += std::to_string(pl.resource[24]) + ", ";
            strQuery += std::to_string(pl.resource[31]) + ", ";
            strQuery += std::to_string(pl.resource[33]) + ", ";
            strQuery += std::to_string(pl.resource[6]) + ", ";
            strQuery += std::to_string(pl.resource[34]) + ", ";
            strQuery += std::to_string(pl.resource[44]) + ", ";

            strQuery += std::to_string(pl.resource[41]) + ", ";
            strQuery += std::to_string(pl.resource[42]) + ", ";
            strQuery += std::to_string(pl.resource[43]) + ", ";

            strQuery += std::to_string(pl.resource[202]) + ", ";
            strQuery += std::to_string(pl.resource[203]) + ", ";
            strQuery += std::to_string(pl.resource[204]) + ", ";
            strQuery += std::to_string(pl.resource[205]) + ", ";
            strQuery += std::to_string(pl.resource[206]) + ", ";
            strQuery += std::to_string(pl.resource[207]) + ", ";
            strQuery += std::to_string(pl.resource[208]) + ", ";
            strQuery += std::to_string(pl.resource[209]) + ", ";
            strQuery += std::to_string(pl.resource[210]) + ", ";
            strQuery += std::to_string(pl.resource[211]) + ", ";
            strQuery += std::to_string(pl.resource[212]) + ", ";
            strQuery += std::to_string(pl.resource[213]) + ", ";
            strQuery += std::to_string(pl.resource[214]) + ", ";
            strQuery += std::to_string(pl.resource[215]) + ", ";
            strQuery += std::to_string(pl.resource[216]) + ", ";
            strQuery += std::to_string(pl.resource[217]) + ", ";
            strQuery += std::to_string(pl.resource[218]) + ", ";
            strQuery += std::to_string(pl.resource[219]) + ", ";
            strQuery += std::to_string(pl.resource[220]) + ", ";
            strQuery += std::to_string(pl.resource[411]) + ", ";

            strQuery += std::to_string(pl.resource[401]) + ", ";
            strQuery += std::to_string(pl.resource[402]) + ", ";
            strQuery += std::to_string(pl.resource[403]) + ", ";
            strQuery += std::to_string(pl.resource[404]) + ", ";
            strQuery += std::to_string(pl.resource[405]) + ", ";
            strQuery += std::to_string(pl.resource[406]) + ", ";
            strQuery += std::to_string(pl.resource[407]) + ", ";
            strQuery += std::to_string(pl.resource[409]) + ", ";
            strQuery += std::to_string(pl.resource[408]) + ", ";
            strQuery += std::to_string(pl.resource[410]) + ", ";
            strQuery += std::to_string(pl.resource[502]) + ", ";
            strQuery += std::to_string(pl.resource[503]) + ", ";

            strQuery += "'" + pl.metal_mine_percent + "', ";
            strQuery += "'" + pl.crystal_mine_percent + "', ";
            strQuery += "'" + pl.deuterium_synthesizer_percent + "', ";
            strQuery += "'" + pl.solar_plant_percent + "', ";
            strQuery += "'" + pl.fusion_plant_percent + "', ";
            strQuery += "'" + pl.solar_satellite_percent + "', ";

            strQuery += std::to_string(pl.last_jump_time) + ", ";
            strQuery += std::to_string(pl.debris_metal) + ", ";
            strQuery += std::to_string(pl.debris_crystal) + ", ";
            strQuery += std::to_string(pl.id_moon) + ", ";
            strQuery += std::to_string(pl.is_bot ? 1 : 0) + ", ";
            strQuery += std::to_string(pl.last_relocate) + ", ";
            strQuery += std::to_string(pl.version);
            strQuery += "), ";

            ++iPlanetCounter;
        }

        if (strQuery != strPlanetHeader)
        {
			if (strQuery.size() >= 2
				&& strQuery.substr(strQuery.size() - 2) == ", ")
			{
				// remove comma and space
				strQuery.pop_back();
				strQuery.pop_back();
			}

            strQuery += strPlanetFooter;

            if (mysql_query(m_pConn, strQuery.c_str()) != 0)
            {
                CLogger::Error("[CDatabase] - UpdateBots (Planets) Paket Hatasi (Index {}-{}): {}", i, endIndex - 1, mysql_error(m_pConn));
                return false;
            }
        }
        
    }

    auto end = GetTimeNow();
    auto duration_ms = GetElapsedMilliseconds(start, end);
    auto duration_us = GetElapsedMicroseconds(start, end);
    CLogger::Info("[CDatabase] - {} bots and {} "
        "planets updated successfully in database.- time {}ms - {}us]",
        iBotCounter,iPlanetCounter, duration_ms, duration_us);
    return true;
}

bool CDatabase::LoadVars()
{
    if (m_pConn == nullptr)
    {
        CLogger::Error("[CDatabase] - LoadVars error: no database connection.");
        return false;
    }

    std::string strQuery = "SELECT * FROM `" + m_strDBPrefix + "vars`";

    if (mysql_query(m_pConn, strQuery.c_str()) != 0)
    {
        CLogger::Error("[CDatabase] - LoadVars Query Error: {}", mysql_error(m_pConn));
        return false;
    }

    MYSQL_RES* result = mysql_store_result(m_pConn);
    if (result == nullptr)
    {
        CLogger::Error("[CDatabase] - LoadVars Result Missing: {}", mysql_error(m_pConn));
        return false;
    }

    G_VARS.clear();

    MYSQL_ROW row;
    int iLoadNum = 0;

    while ((row = mysql_fetch_row(result)))
    {
        // row[0] = element_id, row[1] = name
        if (!row[0] || !row[1])
        {
            continue;
        }

        table_vars item;
        item.element_id = std::stoi(row[0]);
        item.name = row[1];

        item.factor = row[5] ? std::stof(row[5]) : 1.0;
        item.cost901 = row[7] ? std::stoll(row[7]) : 0;  
        item.cost902 = row[8] ? std::stoll(row[8]) : 0;
        item.cost903 = row[9] ? std::stoll(row[9]) : 0;

        int id = item.element_id;

        // 1. PHP: $RESOURCE
        G_RESOURCE[id] = item.name;

        // 2. PHP: $COMBATCAPS (with correct order attack ve defend)
        G_COMBATCAPS[id].attack = row[22] ? std::stod(row[22]) : 0.0; // attack
        G_COMBATCAPS[id].shield = row[23] ? std::stod(row[23]) : 0.0; // defend (shield)

        // 3. PHP: $PRICELIST -> cost
        G_PRICELIST[id].cost[901] = item.cost901;
        G_PRICELIST[id].cost[902] = item.cost902;
        G_PRICELIST[id].cost[903] = item.cost903;
        G_PRICELIST[id].cost[911] = row[10] ? std::stod(row[10]) : 0.0; // cost911 (energy)
        G_PRICELIST[id].cost[921] = row[11] ? std::stod(row[11]) : 0.0; // cost921 (dark matter)

        G_PRICELIST[id].factor = item.factor;
        G_PRICELIST[id].max = row[6] ? std::stoi(row[6]) : 0;   // max_level
        G_PRICELIST[id].consumption = row[12] ? std::stod(row[12]) : 0.0; // consumption1
        G_PRICELIST[id].consumption2 = row[13] ? std::stod(row[13]) : 0.0; // consumption2
        G_PRICELIST[id].speed = row[15] ? std::stod(row[15]) : 0.0; // speed1
        G_PRICELIST[id].speed2 = row[16] ? std::stod(row[16]) : 0.0; // speed2
        G_PRICELIST[id].capacity = row[21] ? std::stod(row[21]) : 0.0; // capacity
        G_PRICELIST[id].tech = row[14] ? std::stoi(row[14]) : 0;   // speed_tech
        G_PRICELIST[id].time = row[24] ? std::stod(row[24]) : 0.0; // time_bonus

        // PHP: $PRICELIST -> bonus & _unit
        G_PRICELIST[id].bonus["Attack"] = { row[25] ? std::stod(row[25]) : 0.0, row[43] ? std::stoi(row[43]) : 0 }; // bonus_attack , _unit
        G_PRICELIST[id].bonus["Defensive"] = { row[26] ? std::stod(row[26]) : 0.0, row[44] ? std::stoi(row[44]) : 0 }; // bonus_defensive , _unit
        G_PRICELIST[id].bonus["Shield"] = { row[27] ? std::stod(row[27]) : 0.0, row[45] ? std::stoi(row[45]) : 0 }; // bonus_shield , _unit
        G_PRICELIST[id].bonus["BuildTime"] = { row[28] ? std::stod(row[28]) : 0.0, row[46] ? std::stoi(row[46]) : 0 }; // bonus_build_time , _unit
        G_PRICELIST[id].bonus["ResearchTime"] = { row[29] ? std::stod(row[29]) : 0.0, row[47] ? std::stoi(row[47]) : 0 }; // bonus_research_time , _unit
        G_PRICELIST[id].bonus["Resource"] = { row[32] ? std::stod(row[32]) : 0.0, row[50] ? std::stoi(row[50]) : 0 }; // bonus_resource , _unit
        G_PRICELIST[id].bonus["Energy"] = { row[33] ? std::stod(row[33]) : 0.0, row[51] ? std::stoi(row[51]) : 0 }; // bonus_energy , _unit

        // 4. PHP: $PRODGRID -> production
        G_PRODGRID[id].production[901] = row[62] ? row[62] : ""; // production901
        G_PRODGRID[id].production[902] = row[63] ? row[63] : ""; // production902
        G_PRODGRID[id].production[903] = row[64] ? row[64] : ""; // production903
        G_PRODGRID[id].production[911] = row[65] ? row[65] : ""; // production911
        G_PRODGRID[id].production[921] = row[66] ? row[66] : ""; // production921

        // PHP: $PRODGRID -> storage
        G_PRODGRID[id].storage[901] = row[67] ? row[67] : ""; // storage901
        G_PRODGRID[id].storage[902] = row[68] ? row[68] : ""; // storage902
        G_PRODGRID[id].storage[903] = row[69] ? row[69] : ""; // storage903

        // PHP: array_filter($PRODGRID...['production'])
        if (!G_PRODGRID[id].production[901].empty()
            || !G_PRODGRID[id].production[902].empty()
            || !G_PRODGRID[id].production[903].empty()
            || !G_PRODGRID[id].production[911].empty())
        {
            G_RESLIST.prod.push_back(id);
        }

        // PHP: array_filter($PRODGRID...['storage'])
        if (!G_PRODGRID[id].storage[901].empty()
            || !G_PRODGRID[id].storage[902].empty()
            || !G_PRODGRID[id].storage[903].empty())
        {
            G_RESLIST.storage.push_back(id);
        }

        // PHP: Bonus total check
        double checkBonus = 0.0;
        for (const auto& b : G_PRICELIST[id].bonus) {
            checkBonus += b.second.value;
        }
        if (checkBonus != 0.0)
        {
            G_RESLIST.bonus.push_back(id);
        }

        // PHP: if ($varsRow['one_per_planet'] == 1)
        if (row[4] && std::stoi(row[4]) == 1) // one_per_planet
        {
            G_RESLIST.one.push_back(id);
        }

        // PHP: switch ($varsRow['class'])
        int itemClass = row[2] ? std::stoi(row[2]) : -1; // class column (0, 100, 200 vb.)
        switch (itemClass)
        {
        case 0:
            G_RESLIST.build.push_back(id);
            // PHP: $tmp = explode(',', $varsRow['on_planet_type']);
            if (row[4]) // on_planet_type kolonu ("1,2,3")
            {
                std::stringstream ss(row[4]);
                std::string token;
                while (std::getline(ss, token, ',')) {
                    if (!token.empty()) {
                        int type = std::stoi(token);
                        G_RESLIST.allow[type].push_back(id);
                    }
                }
            }
            break;
        case 100:  G_RESLIST.tech.push_back(id);      break;
        case 200:  G_RESLIST.fleet.push_back(id);     break;
        case 400:  G_RESLIST.defense.push_back(id);   break;
        case 500:  G_RESLIST.missile.push_back(id);   break;
        case 600:  G_RESLIST.officers.push_back(id);  break;
        case 700:  G_RESLIST.dmfunc.push_back(id);    break;
        }

        G_VARS.emplace(item.element_id, item);
        iLoadNum++;
    }

    // --- overrites, which are not in vars table as default ---

    // 1. G_RESOURCE 
    G_RESOURCE[901] = "metal";
    G_RESOURCE[902] = "crystal";
    G_RESOURCE[903] = "deuterium";
    G_RESOURCE[911] = "energy";
    G_RESOURCE[921] = "darkmatter";

    // 2. G_RESLIST -> ressources list [901, 902, 903, 911, 921]
    G_RESLIST.ressources.insert(G_RESLIST.ressources.end(), { 901, 902, 903, 911, 921 });

    // 3. G_RESLIST -> resstype list
    G_RESLIST.resstype[1].insert(G_RESLIST.resstype[1].end(), { 901, 902, 903 });
    G_RESLIST.resstype[2].push_back(911);
    G_RESLIST.resstype[3].push_back(921);

    mysql_free_result(result);
    CLogger::Info("[CDatabase] - Vars NUM : {} (vars) has been successfully loaded.", iLoadNum);

    return true;
}

bool CDatabase::LoadConfig() 
{
    if (m_pConn == nullptr)
    {
        CLogger::Error("[CDatabase] - LoadConfig - no connection.");
        return false;
    }

    std::string strQuery = "SELECT * FROM `" + m_strDBPrefix + "config`";

    if (mysql_query(m_pConn, strQuery.c_str()) != 0)
    {
        CLogger::Error("[CDatabase] - LoadConfig query error: {}", mysql_error(m_pConn));
        return false;
    }

    MYSQL_RES* result = mysql_store_result(m_pConn);
    if (result == nullptr)
    {
        CLogger::Error("[CDatabase] - LoadConfig Result Error: {}", mysql_error(m_pConn));
        return false;
    }

    G_CONFIG.clear();

    MYSQL_ROW row;
    int loadedCount = 0;

    while ((row = mysql_fetch_row(result)))
    {
        table_config item;
        item.uni = std::stoi(row[0]);
        item.game_speed = std::stoll(row[4]);
        item.resource_multiplier = std::stoi(row[6]);
        item.storage_multiplier = std::stoi(row[7]);
        item.metal_basic_income = std::stoi(row[18]);
        item.crystal_basic_income = std::stoi(row[19]);
        item.deuterium_basic_income = std::stoi(row[20]);
        item.max_overflow = std::stod(row[92]);
        item.energySpeed = std::stoi(row[114]);

        G_CONFIG.emplace(item.uni, item);
        loadedCount++;
    }

    mysql_free_result(result);
    CLogger::Info("[CDatabase] - {}x config row has been loaded. ", loadedCount);

    return true;
}