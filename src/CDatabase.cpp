#define TOML_ENABLE_UNRELEASED_FEATURES 0 // Eğer varsa bunu da ekleyebilirsiniz
#define TOML_EXPORTED_CLASS              // IntelliSense'in kafasını rahatlatır

#include "CDatabase.h"
#include "CLogger.h"
#include "CLanguage.h"
#include <toml++/toml.hpp>
#include <fmt/ranges.h>
#include <filesystem>

using namespace std::literals;

CDatabase::CDatabase(
    const std::unordered_map<std::string, std::string>& lang
)
    : conn_(nullptr)
    , db_user_()
    , db_pass_()
    , db_host_()
    , db_name_()
    , db_ssl_(false)
    , db_uni_prefix_()
    , vars_{}
    , vars_requirements_{}
    , resource_{}
    , combatcaps_{}
    , pricelist_{}
    , prodgrid_{}
    , config_{}
    , reslist_{}
    , temp_bots_{}
    , loop_time_(30)
    , last_load_time_(std::chrono::steady_clock::time_point{})
    , reload_time_(300)
    , lang_(lang)
{
    Init();
}

CDatabase::~CDatabase()
{
    Disconnect();
}

void CDatabase::Init()
{
    std::filesystem::path config_path = std::filesystem::current_path() / "settings.toml";
    toml::table config;

    // 1. Dosya var mı kontrol et, yoksa varsayılanlarla oluştur
    if (!std::filesystem::exists(config_path))
    {
        CLogger::Error(lang_.at("ids_settings_not_found"));

        // defaults
        config = toml::table{
            { "database", toml::table{
                { "host", "localhost" },
                { "user", "username" },
                { "password", "password" },
                { "db_name", "steemnova" },
                { "prefix", "uni1_" },
                { "ssl", false}
            }},
            { "general", toml::table{
                {"loop_time", 30},
                {"bot_reload_time", 300}
            }}
        };

        // write file (std::ofstream )
        std::ofstream out_file(config_path);
        if (out_file.is_open())
        {
            out_file << config; // toml++ tablosunu doğrudan dosyaya akıtabilirsiniz
            out_file.close();
        }
        else
        {
            CLogger::Error(lang_.at("ids_settings_cannot_be_saved"));
        }
    }
    else
    {
        // 2. Dosya zaten varsa oku
        try
        {
            config = toml::parse_file(config_path.string());
        }
        catch (const toml::parse_error& err)
        {
            CLogger::Error(lang_.at("ids_settings_cannot_be_parsed"), err.description());
            return;
        }
    }

    // 3. Değerleri oku ve sınıf üyelerine ata (Veri yoksa fallback olarak ""sv kullanır)
    db_host_ = config["database"]["host"].value_or(""sv);
    db_user_ = config["database"]["user"].value_or(""sv);
    db_pass_ = config["database"]["password"].value_or(""sv);
    db_name_ = config["database"]["db_name"].value_or(""sv);
    db_ssl_ = config["database"]["ssl"].value_or(false);
    db_uni_prefix_ = config["database"]["prefix"].value_or(""sv);
    
    // general settings
    loop_time_ = config["general"]["loop_time"].value_or(30);
    reload_time_ = config["general"]["bot_reload_time"].value_or(300);

    CLogger::Info(lang_.at("ids_settings_read"), db_host_);
}

bool CDatabase::Connect()
{
    conn_ = mysql_init(nullptr);

    if (conn_ == nullptr)
    {
        CLogger::Error(lang_.at("ids_mysql_init_failed"));
        return false;
    }

    // close ssl
    if (!db_ssl_)
    {
        int ssl_verify = 0;
        mysql_optionsv(conn_, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &ssl_verify);
    }
    else
    {
        CLogger::Info(lang_.at("ids_ssl_is_active"));
    }
    // ----------------------------------------

    if (!mysql_real_connect(
        conn_,
        db_host_.c_str(),
        db_user_.c_str(),
        db_pass_.c_str(),
        db_name_.c_str(),
        3306,
        nullptr,
        0))
    {
        CLogger::Error(lang_.at("ids_mysql_conn_failed"), mysql_error(conn_), mysql_errno(conn_));

        // close db always
        mysql_close(conn_);
        conn_ = nullptr;
        return false;
    }

    return true;
}

void CDatabase::Disconnect()
{
    if (conn_ != nullptr)
    {
        mysql_close(conn_);
        conn_ = nullptr;
    }
}

bool CDatabase::LoadBots()
{
    // only load bots evey x seconds
    if (GetTimeNow() < last_load_time_ + std::chrono::seconds(reload_time_))
    {
        return false;
    }

    auto start = GetTimeNow();
    std::string strQuery = std::format(
        "SELECT "
        "`id`, `username`, `email`, "
        "`id_planet`, `universe`, `galaxy`, "
        "`system`, `planet`, `onlinetime`, "
        "`vacation_mode`, `vacation_until`, `b_tech_planet`, "
        "`b_tech`, `b_tech_id`, `b_tech_queue`, "
        "`spy_tech`, `computer_tech`, `military_tech`, "
        "`armor_tech`, `shield_tech`, `energy_tech`, "
        "`hyperspace_tech`, `combustion_tech`, `impulse_motor_tech`, "
        "`hyperspace_motor_tech`, `laser_tech`, `ion_tech`, "
        "`plasma_tech`, `intergalactic_tech`, `expedition_tech`, "
        "`metal_proc_tech`, `crystal_proc_tech`, `deuterium_proc_tech`, "
        "`graviton_tech`, `rpg_geologist`, `rpg_admiral`, "
        "`rpg_engineer`, `rpg_technocrat`, `rpg_espion`, "
        "`rpg_constructor`, `rpg_scientist`, `rpg_commander`, "
        "`rpg_stocker`, `rpg_defender`, `rpg_destructor`, "
        "`rpg_general`, `rpg_bunker`, `rpg_raider`, "
        "`rpg_emperor` "
        "FROM {}users "
        "WHERE is_bot = 1"
    , db_uni_prefix_);

    if (mysql_query(conn_, strQuery.c_str()))
    {
        CLogger::Error(lang_.at("ids_mysql_query_error"), mysql_error(conn_));
        return false;
    }

    MYSQL_RES* result = mysql_store_result(conn_);
    if (result == nullptr)
    {
        if (mysql_field_count(conn_) > 0)
        {
            CLogger::Error(lang_.at("ids_mysql_retrieve_error"), mysql_error(conn_));
        }

        return false;
    }

    uint64_t rowCount = mysql_num_rows(result);
    CLogger::Info(lang_.at("ids_found_num_bots"), rowCount);

    temp_bots_.clear();
    temp_bots_.reserve(rowCount);

    MYSQL_ROW row;

    time_t now = std::time(nullptr); 
    int ibotCounter = 0;
    table_users bot;

    while ((row = mysql_fetch_row(result)))
    {
        size_t i = 0;

        bot.Reset();

        bot.id = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.type = bot.id % 10;
        bot.SetPlayStyle();

        bot.strUserName = row[i] ? row[i] : ""; i++;
        bot.email = row[i] ? row[i] : ""; i++;

        // location
        bot.id_planet = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.universe = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.galaxy = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.system = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.planet = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.onlinetime = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.vacation_mode = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.vacation_until = row[i] ? std::stoi(row[i]) : 0; i++;

        // research queue
        bot.b_tech_planet = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.b_tech = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.b_tech_id = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.b_tech_queue = row[i] ? row[i] : ""; i++;

        // technologies
        bot.resource[106] = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.resource[108] = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.resource[109] = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.resource[110] = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.resource[111] = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.resource[113] = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.resource[114] = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.resource[115] = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.resource[117] = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.resource[118] = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.resource[120] = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.resource[121] = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.resource[122] = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.resource[123] = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.resource[124] = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.resource[131] = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.resource[132] = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.resource[133] = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.resource[199] = row[i] ? std::stoi(row[i]) : 0; i++;

        // commanders
        bot.rpg_geologist = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.rpg_admiral = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.rpg_engineer = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.rpg_technocrat = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.rpg_espion = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.rpg_constructor = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.rpg_scientist = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.rpg_commander = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.rpg_stocker = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.rpg_defender = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.rpg_destructor = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.rpg_general = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.rpg_bunker = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.rpg_raider = row[i] ? std::stoi(row[i]) : 0; i++;
        bot.rpg_emperor = row[i] ? std::stoi(row[i]) : 0; i++;

        // column is_bot to simplify
        bot.is_bot = 1;
        bot.SetFactor(now, reslist_, pricelist_);
        temp_bots_.push_back(std::move(bot));
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

    std::string strPlanetQuery = std::format(
        "SELECT "
        "`id`, `name`, `id_owner`, "
        "`universe`, `galaxy`, `system`, "
        "`planet`, `last_update`, `planet_type`, "
        "`destroyed`, `b_building`, `b_building_id`, "
        "`b_shipyard`, `b_shipyard_id`, `b_shipyard_plus`, "
        "`image`, `diameter`, `field_current`, "
        "`field_max`, `temp_min`, `temp_max`, "
        "`eco_hash`, `metal`, `metal_perhour`, "
        "`metal_max`, `crystal`, `crystal_perhour`, "
        "`crystal_max`, `deuterium`, `deuterium_perhour`, "
        "`deuterium_max`, `energy_used`, `energy`, "

        "`metal_mine`, `crystal_mine`, `deuterium_synthesizer`, "
        "`solar_plant`, `fusion_plant`, `robot_factory`, "
        "`nanite_factory`, `shipyard`, `metal_storage`, "
        "`crystal_storage`, `deuterium_tank`, `research_lab`, "
        "`terraformer`, `university`, `ally_deposit`, "
        "`missile_silo`, `lunar_base`, `phalanx`, "
        "`jump_gate`, `small_cargo`, `big_cargo`, "
        "`light_hunter`, `heavy_hunter`, `cruiser`, "
        "`battle_ship`, `colony_ship`, `recycler`, "
        "`espionage_probe`, `bomber_ship`, `solar_satellite`, "
        "`destroyer`, `death_star`, `battle_cruiser`, "
        "`black_moon`, `ev_transporter`, `star_crasher`, "
        "`giga_recycler`, `dm_ship`, `orbital_station`, "

        "`rocket_launcher`, `light_laser`, `heavy_laser`, "
        "`gauss_cannon`, `ion_cannon`, `plasma_turret`, "
        "`small_protection_shield`, `planet_protector`, `big_protection_shield`, "
        "`graviton_cannon`, `interceptor_misil`, `interplanetary_misil`, "

        "`metal_mine_percent`, `crystal_mine_percent`, `deuterium_synthesizer_percent`, "
        "`solar_plant_percent`, `fusion_plant_percent`, `solar_satellite_percent`, "
        "`last_jump_time`, `debris_metal`, `debris_crystal`, "
        "`id_moon`, `last_relocate` "

        "FROM `{}planets` "
        "WHERE is_bot = 1",
        db_uni_prefix_);

    if (mysql_query(conn_, strPlanetQuery.c_str()))
    {
        CLogger::Error(lang_.at("ids_mysql_query_error"), mysql_error(conn_));
        return false;
    }

    MYSQL_RES* plResult = mysql_store_result(conn_);
    if (plResult == nullptr)
    {
        if (mysql_field_count(conn_) > 0) 
        {
            CLogger::Error(lang_.at("ids_mysql_retrieve_error"), mysql_error(conn_));
        }

        return false;
    }

    MYSQL_ROW plRow; int iPlanetCounter = 0;
    table_planets pl;

    while ((plRow = mysql_fetch_row(plResult)))
    {
        pl.Reset();
        size_t i = 0;
        pl.id = plRow[i] ? std::stoi(plRow[i]) : 0; i++;
        pl.name = plRow[i] ? plRow[i] : ""; i++;
        pl.id_owner = plRow[i] ? std::stoi(plRow[i]) : 0; i++;
        pl.universe = plRow[i] ? std::stoi(plRow[i]) : 0; i++;
        pl.galaxy = plRow[i] ? std::stoi(plRow[i]) : 0; i++;
        pl.system = plRow[i] ? std::stoi(plRow[i]) : 0; i++;
        pl.planet = plRow[i] ? std::stoi(plRow[i]) : 0; i++;
        pl.last_update = plRow[i] ? std::stoi(plRow[i]) : 0; i++;
        pl.planet_type = plRow[i] ? std::stoi(plRow[i]) : 1; i++;
        pl.destroyed = plRow[i] ? std::stoi(plRow[i]) : 0; i++;

        pl.b_building = plRow[i] ? std::stoi(plRow[i]) : 0; i++;
        pl.b_building_id = plRow[i] ? plRow[i] : ""; i++;
        pl.b_shipyard = plRow[i] ? std::stoi(plRow[i]) : 0; i++;
        pl.b_shipyard_id = plRow[i] ? plRow[i] : ""; i++;
        pl.b_shipyard_plus = plRow[i] ? std::stoi(plRow[i]) : 0; i++;
        pl.image = plRow[i] ? plRow[i] : ""; i++;

        pl.diameter = plRow[i] ? std::stoi(plRow[i]) : 0; i++;
        pl.field_current = plRow[i] ? std::stoi(plRow[i]) : 0; i++;
        pl.field_max = plRow[i] ? std::stoi(plRow[i]) : 0; i++;
        pl.temp_min = plRow[i] ? std::stoi(plRow[i]) : 0; i++;
        pl.temp_max = plRow[i] ? std::stoi(plRow[i]) : 0; i++;

        pl.eco_hash = plRow[i] ? plRow[i] : ""; i++;
        pl.metal = plRow[i] ? std::stod(plRow[i]) : 0.0; i++;
        pl.metal_perhour = plRow[i] ? std::stod(plRow[i]) : 0.0; i++;
        pl.metal_max = plRow[i] ? std::stod(plRow[i]) : 0.0; i++;
        pl.crystal = plRow[i] ? std::stod(plRow[i]) : 0.0; i++;
        pl.crystal_perhour = plRow[i] ? std::stod(plRow[i]) : 0.0; i++;
        pl.crystal_max = plRow[i] ? std::stod(plRow[i]) : 0.0; i++;
        pl.deuterium = plRow[i] ? std::stod(plRow[i]) : 0.0; i++;
        pl.deuterium_perhour = plRow[i] ? std::stod(plRow[i]) : 0.0; i++;
        pl.deuterium_max = plRow[i] ? std::stod(plRow[i]) : 0.0; i++;
        pl.energy_used = plRow[i] ? std::stod(plRow[i]) : 0.0; i++;
        pl.energy = plRow[i] ? std::stod(plRow[i]) : 0.0; i++;

        // buildings
        pl.resource[1] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[2] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[3] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[4] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[12] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[14] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[15] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[21] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[22] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[23] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[24] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[31] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[33] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[6] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[34] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[44] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[41] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[42] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[43] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;

        // fleet
        pl.resource[202] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[203] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[204] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[205] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[206] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[207] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[208] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[209] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[210] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[211] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[212] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[213] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[214] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[215] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[216] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[217] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[218] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[219] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[220] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[411] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;

        // defence & missiles
        pl.resource[401] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[402] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[403] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[404] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[405] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[406] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[407] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[409] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[408] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[410] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[502] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;
        pl.resource[503] = plRow[i] ? std::stoull(plRow[i]) : 0; i++;

        // production
        pl.metal_mine_percent = plRow[i] ? plRow[i] : "10"; i++;
        pl.crystal_mine_percent = plRow[i] ? plRow[i] : "10"; i++;
        pl.deuterium_synthesizer_percent = plRow[i] ? plRow[i] : "10"; i++;
        pl.solar_plant_percent = plRow[i] ? plRow[i] : "10"; i++;
        pl.fusion_plant_percent = plRow[i] ? plRow[i] : "10"; i++;
        pl.solar_satellite_percent = plRow[i] ? plRow[i] : "10"; i++;

        // galaxy related
        pl.last_jump_time = plRow[i] ? std::stoi(plRow[i]) : 0; i++;
        pl.debris_metal = plRow[i] ? std::stod(plRow[i]) : 0.0; i++;
        pl.debris_crystal = plRow[i] ? std::stod(plRow[i]) : 0.0; i++;
        pl.id_moon = plRow[i] ? std::stoi(plRow[i]) : 0; i++;
        pl.last_relocate = plRow[i] ? std::stoi(plRow[i]) : 0; i++;

        pl.is_bot = 1;
        // find bot with planet id_owner
        table_users* pTargetBot = GetBotRef(pl.id_owner);
        if (pTargetBot != nullptr)
        {
            pTargetBot->vecPlanets.push_back(std::move(pl));
            ++iPlanetCounter;
        }
        else
        {
            CLogger::Warn(lang_.at("ids_no_match_planet_bot"), pl.id, pl.id_owner);
        }
    }
   
    mysql_free_result(plResult);

    last_load_time_ = std::chrono::steady_clock::now();
   
    auto end = GetTimeNow();
    auto duration_micros = GetElapsedMicroseconds(start, end);
    double duration_millis = GetElapsedMilliseconds(start, end);
	CLogger::Info(lang_.at("ids_load_planet_bots_succ"),
		ibotCounter, iPlanetCounter, duration_micros, duration_millis);
    return true;
}

bool CDatabase::LoadVars()
{
    if (conn_ == nullptr)
    {
        CLogger::Error(lang_.at("ids_mysql_conn_failed"));
        return false;
    }

    std::string strQuery = "SELECT * FROM `" + db_uni_prefix_ + "vars`";

    if (mysql_query(conn_, strQuery.c_str()) != 0)
    {
        CLogger::Error(lang_.at("ids_mysql_query_error"), mysql_error(conn_));
        return false;
    }

    MYSQL_RES* result = mysql_store_result(conn_);
    if (result == nullptr)
    {
        CLogger::Error(lang_.at("ids_mysql_retrieve_error"), mysql_error(conn_));
        return false;
    }

    vars_.clear();

    MYSQL_ROW row;
    int iLoadNum = 0;

    table_vars item;
    while ((row = mysql_fetch_row(result)))
    {
        // row[0] = element_id, row[1] = name
        if (!row[0] || !row[1])
        {
            continue;
        }

        item.Reset();
        item.element_id = std::stoi(row[0]);
        item.name = row[1];

        item.factor = row[5] ? std::stof(row[5]) : 1.0;
        item.cost901 = row[7] ? std::stoll(row[7]) : 0;  
        item.cost902 = row[8] ? std::stoll(row[8]) : 0;
        item.cost903 = row[9] ? std::stoll(row[9]) : 0;

        int id = item.element_id;

        // 1. PHP: $RESOURCE
        resource_[id] = item.name;

        // 2. PHP: $COMBATCAPS (with correct order attack ve defend)
        combatcaps_[id].attack = row[22] ? std::stod(row[22]) : 0.0; // attack
        combatcaps_[id].shield = row[23] ? std::stod(row[23]) : 0.0; // defend (shield)

        // 3. PHP: $PRICELIST -> cost
        pricelist_[id].cost[901] = item.cost901;
        pricelist_[id].cost[902] = item.cost902;
        pricelist_[id].cost[903] = item.cost903;
        pricelist_[id].cost[911] = row[10] ? std::stod(row[10]) : 0.0; // cost911 (energy)
        pricelist_[id].cost[921] = row[11] ? std::stod(row[11]) : 0.0; // cost921 (dark matter)

        pricelist_[id].factor = item.factor;
        pricelist_[id].max = row[6] ? std::stoi(row[6]) : 0;   // max_level
        pricelist_[id].consumption = row[12] ? std::stod(row[12]) : 0.0; // consumption1
        pricelist_[id].consumption2 = row[13] ? std::stod(row[13]) : 0.0; // consumption2
        pricelist_[id].speed = row[15] ? std::stod(row[15]) : 0.0; // speed1
        pricelist_[id].speed2 = row[16] ? std::stod(row[16]) : 0.0; // speed2
        pricelist_[id].capacity = row[21] ? std::stod(row[21]) : 0.0; // capacity
        pricelist_[id].tech = row[14] ? std::stoi(row[14]) : 0;   // speed_tech
        pricelist_[id].time = row[24] ? std::stod(row[24]) : 0.0; // time_bonus

        // PHP: $PRICELIST -> bonus & _unit
        pricelist_[id].bonus["Attack"] = { row[25] ? std::stod(row[25]) : 0.0, row[43] ? std::stoi(row[43]) : 0 }; // bonus_attack , _unit
        pricelist_[id].bonus["Defensive"] = { row[26] ? std::stod(row[26]) : 0.0, row[44] ? std::stoi(row[44]) : 0 }; // bonus_defensive , _unit
        pricelist_[id].bonus["Shield"] = { row[27] ? std::stod(row[27]) : 0.0, row[45] ? std::stoi(row[45]) : 0 }; // bonus_shield , _unit
        pricelist_[id].bonus["BuildTime"] = { row[28] ? std::stod(row[28]) : 0.0, row[46] ? std::stoi(row[46]) : 0 }; // bonus_build_time , _unit
        pricelist_[id].bonus["ResearchTime"] = { row[29] ? std::stod(row[29]) : 0.0, row[47] ? std::stoi(row[47]) : 0 }; // bonus_research_time , _unit
        pricelist_[id].bonus["Resource"] = { row[32] ? std::stod(row[32]) : 0.0, row[50] ? std::stoi(row[50]) : 0 }; // bonus_resource , _unit
        pricelist_[id].bonus["Energy"] = { row[33] ? std::stod(row[33]) : 0.0, row[51] ? std::stoi(row[51]) : 0 }; // bonus_energy , _unit

        // 4. PHP: $PRODGRID -> production
        prodgrid_[id].production[901] = row[62] ? row[62] : ""; // production901
        prodgrid_[id].production[902] = row[63] ? row[63] : ""; // production902
        prodgrid_[id].production[903] = row[64] ? row[64] : ""; // production903
        prodgrid_[id].production[911] = row[65] ? row[65] : ""; // production911
        prodgrid_[id].production[921] = row[66] ? row[66] : ""; // production921

        // PHP: $PRODGRID -> storage
        prodgrid_[id].storage[901] = row[67] ? row[67] : ""; // storage901
        prodgrid_[id].storage[902] = row[68] ? row[68] : ""; // storage902
        prodgrid_[id].storage[903] = row[69] ? row[69] : ""; // storage903

        // PHP: array_filter($PRODGRID...['production'])
        if (!prodgrid_[id].production[901].empty()
            || !prodgrid_[id].production[902].empty()
            || !prodgrid_[id].production[903].empty()
            || !prodgrid_[id].production[911].empty())
        {
            reslist_.prod.push_back(id);
        }

        // PHP: array_filter($PRODGRID...['storage'])
        if (!prodgrid_[id].storage[901].empty()
            || !prodgrid_[id].storage[902].empty()
            || !prodgrid_[id].storage[903].empty())
        {
            reslist_.storage.push_back(id);
        }

        // PHP: Bonus total check
        double checkBonus = 0.0;
        for (const auto& b : pricelist_[id].bonus) {
            checkBonus += b.second.value;
        }
        if (checkBonus != 0.0)
        {
            reslist_.bonus.push_back(id);
        }

        // PHP: if ($varsRow['one_per_planet'] == 1)
        if (row[4] && std::stoi(row[4]) == 1) // one_per_planet
        {
            reslist_.one.push_back(id);
        }

        // PHP: switch ($varsRow['class'])
        int itemClass = row[2] ? std::stoi(row[2]) : -1; // class column (0, 100, 200 vb.)
        switch (itemClass)
        {
        case 0:
            reslist_.build.push_back(id);
            // PHP: $tmp = explode(',', $varsRow['on_planet_type']);
            if (row[4]) // on_planet_type kolonu ("1,2,3")
            {
                std::stringstream ss(row[4]);
                std::string token;
                while (std::getline(ss, token, ',')) {
                    if (!token.empty()) {
                        int type = std::stoi(token);
                        reslist_.allow[type].push_back(id);
                    }
                }
            }
            break;
        case 100:  reslist_.tech.push_back(id);      break;
        case 200:  reslist_.fleet.push_back(id);     break;
        case 400:  reslist_.defense.push_back(id);   break;
        case 500:  reslist_.missile.push_back(id);   break;
        case 600:  reslist_.officers.push_back(id);  break;
        case 700:  reslist_.dmfunc.push_back(id);    break;
        }

        vars_.emplace(item.element_id, item);
        iLoadNum++;
    }

    // --- overrites, which are not in vars table as default ---

    // 1. resource_ 
    resource_[901] = "metal";
    resource_[902] = "crystal";
    resource_[903] = "deuterium";
    resource_[911] = "energy";
    resource_[921] = "darkmatter";

    // 2. reslist_ -> ressources list [901, 902, 903, 911, 921]
    reslist_.ressources.insert(reslist_.ressources.end(), { 901, 902, 903, 911, 921 });

    // 3. reslist_ -> resstype list
    reslist_.resstype[1].insert(reslist_.resstype[1].end(), { 901, 902, 903 });
    reslist_.resstype[2].push_back(911);
    reslist_.resstype[3].push_back(921);

    mysql_free_result(result);
    CLogger::Info(lang_.at("ids_load_vars_succ"), iLoadNum);

    return true;
}

bool CDatabase::LoadVarsRequirements() 
{
    if (conn_ == nullptr)
    {
        CLogger::Error(lang_.at("ids_mysql_conn_failed"));
        return false;
    }

    std::string strQuery = "SELECT * FROM `" + db_uni_prefix_ + "vars_requirements`";

    if (mysql_query(conn_, strQuery.c_str()) != 0)
    {
        CLogger::Error(lang_.at("ids_mysql_query_error"), mysql_error(conn_));
        return false;
    }

    MYSQL_RES* result = mysql_store_result(conn_);
    if (result == nullptr)
    {
        CLogger::Error(lang_.at("ids_mysql_retrieve_error"), mysql_error(conn_));
        return false;
    }

    vars_requirements_.clear();

    MYSQL_ROW row;
    int load_num = 0;

    table_vars_requirements item;
    while ((row = mysql_fetch_row(result)))
    {
        if (!row[0] || !row[1] || !row[2])
        {
            continue;
        }

        item.Reset();
        int element_id = std::stoi(row[0]);
        item.require_id = std::stoi(row[1]);
        item.require_level = std::stoi(row[2]);

        vars_requirements_[element_id].push_back(item);
        load_num++;
    }

    mysql_free_result(result);
    CLogger::Info(lang_.at("ids_load_varsreq_succ"), load_num);

    return true;
}

bool CDatabase::LoadConfig() 
{
    if (conn_ == nullptr)
    {
        CLogger::Error(lang_.at("ids_mysql_conn_failed"));
        return false;
    }

    std::string query = std::format(
        "SELECT `uni`, `game_speed`, `resource_multiplier`, "
        "`storage_multiplier`, `metal_basic_income`, `crystal_basic_income`, "
        "`deuterium_basic_income`, `max_galaxy`, `max_system`, "
        "`max_planets`, `max_overflow`, `energySpeed` "
        "FROM `{}config`",
        db_uni_prefix_);

    if (mysql_query(conn_, query.c_str()) != 0)
    {
        CLogger::Error(lang_.at("ids_mysql_query_error"), mysql_error(conn_));
        return false;
    }

    MYSQL_RES* result = mysql_store_result(conn_);
    if (result == nullptr)
    {
        CLogger::Error(lang_.at("ids_mysql_retrieve_error"), mysql_error(conn_));
        return false;
    }

    config_.clear();

    MYSQL_ROW row;
    int loadedCount = 0;

    table_config item;
    while ((row = mysql_fetch_row(result)))
    {
        size_t i = 0;

        item.Reset();

        item.uni = std::stoi(row[i]); i++;
        item.game_speed = std::stoll(row[i]); i++;
        item.resource_multiplier = std::stoi(row[i]); i++;
        item.storage_multiplier = std::stoi(row[i]); i++;
        item.metal_basic_income = std::stoi(row[i]); i++;
        item.crystal_basic_income = std::stoi(row[i]); i++;
        item.deuterium_basic_income = std::stoi(row[i]); i++;
        item.max_galaxy = std::stoi(row[i]); i++;
        item.max_system = std::stoi(row[i]); i++;
        item.max_planet = std::stoi(row[i]); i++;
        item.max_overflow = std::stod(row[i]); i++;
        item.energySpeed = std::stoi(row[i]); i++;

        config_.emplace(item.uni, item);
        loadedCount++;
    }

    mysql_free_result(result);
    CLogger::Info(lang_.at("ids_load_config_succ"), loadedCount);

    return true;
}

bool CDatabase::UpdateBots(std::vector<table_users>& vecBots)
{

    if (vecBots.empty()) return false;
    if (conn_ == nullptr) return false;
    // =========================================================================
    // PART 1: UPDATE BOT RESEARCH (USERS TABLE)
    // =========================================================================
    const std::string user_header = std::format("INSERT INTO `{}users` ("
        "id, b_tech_planet, b_tech, b_tech_id, b_tech_queue, spy_tech, computer_tech, "
        "military_tech, armor_tech, shield_tech, energy_tech, hyperspace_tech, "
        "combustion_tech, impulse_motor_tech, hyperspace_motor_tech, laser_tech, "
        "ion_tech, plasma_tech, intergalactic_tech, expedition_tech, "
        "metal_proc_tech, crystal_proc_tech, deuterium_proc_tech, graviton_tech, onlinetime) VALUES "
        , db_uni_prefix_);


    const std::string user_footer = " ON DUPLICATE KEY UPDATE "
        "b_tech_planet = VALUES(b_tech_planet), b_tech = VALUES(b_tech), b_tech_id = VALUES(b_tech_id), b_tech_queue = VALUES(b_tech_queue), "
        "spy_tech = VALUES(spy_tech), computer_tech = VALUES(computer_tech), military_tech = VALUES(military_tech), armor_tech = VALUES(armor_tech), "
        "shield_tech = VALUES(shield_tech), energy_tech = VALUES(energy_tech), hyperspace_tech = VALUES(hyperspace_tech), combustion_tech = VALUES(combustion_tech), "
        "impulse_motor_tech = VALUES(impulse_motor_tech), hyperspace_motor_tech = VALUES(hyperspace_motor_tech), laser_tech = VALUES(laser_tech), "
        "ion_tech = VALUES(ion_tech), plasma_tech = VALUES(plasma_tech), intergalactic_tech = VALUES(intergalactic_tech), expedition_tech = VALUES(expedition_tech), "
        "metal_proc_tech = VALUES(metal_proc_tech), crystal_proc_tech = VALUES(crystal_proc_tech), deuterium_proc_tech = VALUES(deuterium_proc_tech), "
        "graviton_tech = VALUES(graviton_tech), onlinetime = VALUES(onlinetime);";

    // store all planets in same vector
    std::vector<table_planets*> all_planets;
    // std::vector<int> vecIDPlanetsNoUpdate; // testing
    // std::vector<int> vecIDBotsNoUpdate;

    int planet_counter = 0, bot_counter = 0, count = (int) vecBots.size();
    auto start = GetTimeNow();
    std::string query = user_header;
    fmt::memory_buffer buf;
    for (size_t i = 0; i < count; i += BATCH_SIZE)
    {
        query = user_header;
        size_t endIndex = ((i + BATCH_SIZE) < count) ? (i + BATCH_SIZE) : count;

        for (size_t k = i; k < endIndex; ++k)
        {
            auto& cBot = vecBots[k];

            for (auto& planet : cBot.vecPlanets)
            {

                if (!planet.need_update)
                {
                    //CLogger::Info("PID:{} does not need an update", planet.id);
                    // vecIDPlanetsNoUpdate.push_back(planet.id);
                    continue;
                }

                planet.need_update = false;
                all_planets.push_back(&planet);
            }

            // planet might need update.. (finished building etc.)
            if (!cBot.need_update)
            {
                //CLogger::Info("Bot does not need update {}", cBot.id);
                // vecIDBotsNoUpdate.push_back(cBot.id);
                continue;
            }

            fmt::format_to(
                std::back_inserter(query),
                "({}, {}, {}, {}, '{}', {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}), ",
                cBot.id,
                cBot.b_tech_planet,
                cBot.b_tech,
                cBot.b_tech_id,
                cBot.b_tech_queue,
                cBot.resource[106],
                cBot.resource[108],
                cBot.resource[109],
                cBot.resource[110],
                cBot.resource[111],
                cBot.resource[113],
                cBot.resource[114],
                cBot.resource[115],
                cBot.resource[117],
                cBot.resource[118],
                cBot.resource[120],
                cBot.resource[121],
                cBot.resource[122],
                cBot.resource[123],
                cBot.resource[124],
                cBot.resource[131],
                cBot.resource[132],
                cBot.resource[133],
                cBot.resource[199],
                cBot.onlinetime
            );

            cBot.need_update = false;
            ++bot_counter;
        }

        if (query != user_header)
        {
            size_t len = query.size();
            if (len >= 2
                && query.substr(len - 2) == ", ")
            {
                // remove comma and space
                query.pop_back();
                query.pop_back();
            }

            query += user_footer;

            if (mysql_query(conn_, query.c_str()) != 0)
            {
                CLogger::Error(lang_.at("ids_mysql_query_error"), mysql_error(conn_));
                return false;
            }
        }

    }
    auto end = GetTimeNow();

    /*
    if (!vecIDPlanetsNoUpdate.empty()
        || !vecIDBotsNoUpdate.empty())
    {

        spdlog::info("PID:[{}], UID:[{}] does not need update",
            fmt::join(vecIDPlanetsNoUpdate, "],["),
            fmt::join(vecIDBotsNoUpdate, "],[")
        );
    }
    */

    // =========================================================================
    // PART 2: UPDATE PLANETS (PLANETS TABLE) 
    // =========================================================================
    if (all_planets.empty())
    {
        return true;
    }

    // order is important !
    const std::string planet_header = std::format(
        "INSERT INTO `{}planets` ("
        "id, last_update, "
        "b_building, b_building_id, b_shipyard, b_shipyard_id, b_shipyard_plus, field_current, field_max, "
        "eco_hash, metal, metal_perhour, metal_max, crystal, crystal_perhour, crystal_max, deuterium, deuterium_perhour, deuterium_max, energy_used, energy, "
        "metal_mine, crystal_mine, deuterium_synthesizer, solar_plant, fusion_plant, robot_factory, nanite_factory, shipyard, metal_storage, crystal_storage, deuterium_tank, research_lab, terraformer, university, ally_deposit, missile_silo, "
        "lunar_base, phalanx, jump_gate, small_cargo, big_cargo, light_hunter, heavy_hunter, cruiser, battle_ship, colony_ship, recycler, espionage_probe, bomber_ship, solar_satellite, destroyer, death_star, battle_cruiser, black_moon, ev_transporter, star_crasher, giga_recycler, dm_ship, orbital_station, "
        "rocket_launcher, light_laser, heavy_laser, gauss_cannon, ion_cannon, plasma_turret, small_protection_shield, planet_protector, big_protection_shield, graviton_cannon, interceptor_misil, interplanetary_misil, "
        "metal_mine_percent, crystal_mine_percent, deuterium_synthesizer_percent, solar_plant_percent, fusion_plant_percent, solar_satellite_percent "
        ") VALUES "
        , db_uni_prefix_);

    // footer
    const std::string planet_footer = " ON DUPLICATE KEY UPDATE "
        "last_update=VALUES(last_update), "
        "b_building=VALUES(b_building), b_building_id=VALUES(b_building_id), b_shipyard=VALUES(b_shipyard), b_shipyard_id=VALUES(b_shipyard_id), b_shipyard_plus=VALUES(b_shipyard_plus), field_current=VALUES(field_current), field_max=VALUES(field_max), "
        "eco_hash=VALUES(eco_hash), metal=VALUES(metal), metal_perhour=VALUES(metal_perhour), metal_max=VALUES(metal_max), crystal=VALUES(crystal), crystal_perhour=VALUES(crystal_perhour), crystal_max=VALUES(crystal_max), deuterium=VALUES(deuterium), deuterium_perhour=VALUES(deuterium_perhour), deuterium_max=VALUES(deuterium_max), energy_used=VALUES(energy_used), energy=VALUES(energy), "
        "metal_mine=VALUES(metal_mine), crystal_mine=VALUES(crystal_mine), deuterium_synthesizer=VALUES(deuterium_synthesizer), solar_plant=VALUES(solar_plant), fusion_plant=VALUES(fusion_plant), robot_factory=VALUES(robot_factory), nanite_factory=VALUES(nanite_factory), shipyard=VALUES(shipyard), metal_storage=VALUES(metal_storage), crystal_storage=VALUES(crystal_storage), deuterium_tank=VALUES(deuterium_tank), research_lab=VALUES(research_lab), terraformer=VALUES(terraformer), university=VALUES(university), ally_deposit=VALUES(ally_deposit), missile_silo=VALUES(missile_silo), "
        "lunar_base=VALUES(lunar_base), phalanx=VALUES(phalanx), jump_gate=VALUES(jump_gate), small_cargo=VALUES(small_cargo), big_cargo=VALUES(big_cargo), light_hunter=VALUES(light_hunter), heavy_hunter=VALUES(heavy_hunter), cruiser=VALUES(cruiser), battle_ship=VALUES(battle_ship), colony_ship=VALUES(colony_ship), recycler=VALUES(recycler), espionage_probe=VALUES(espionage_probe), bomber_ship=VALUES(bomber_ship), solar_satellite=VALUES(solar_satellite), destroyer=VALUES(destroyer), death_star=VALUES(death_star), battle_cruiser=VALUES(battle_cruiser), black_moon=VALUES(black_moon), ev_transporter=VALUES(ev_transporter), star_crasher=VALUES(star_crasher), giga_recycler=VALUES(giga_recycler), dm_ship=VALUES(dm_ship), orbital_station=VALUES(orbital_station), "
        "rocket_launcher=VALUES(rocket_launcher), light_laser=VALUES(light_laser), heavy_laser=VALUES(heavy_laser), gauss_cannon=VALUES(gauss_cannon), ion_cannon=VALUES(ion_cannon), plasma_turret=VALUES(plasma_turret), small_protection_shield=VALUES(small_protection_shield), planet_protector=VALUES(planet_protector), big_protection_shield=VALUES(big_protection_shield), graviton_cannon=VALUES(graviton_cannon), interceptor_misil=VALUES(interceptor_misil), interplanetary_misil=VALUES(interplanetary_misil), "
        "metal_mine_percent=VALUES(metal_mine_percent), crystal_mine_percent=VALUES(crystal_mine_percent), deuterium_synthesizer_percent=VALUES(deuterium_synthesizer_percent), solar_plant_percent=VALUES(solar_plant_percent), fusion_plant_percent=VALUES(fusion_plant_percent), solar_satellite_percent=VALUES(solar_satellite_percent);";
    // batch size 50 as default
    int planet_count = all_planets.size();
    for (size_t i = 0; i < planet_count; i += BATCH_SIZE)
    {
        std::string query = planet_header;
        size_t endIndex = ((i + BATCH_SIZE) < planet_count) ? (i + BATCH_SIZE) : planet_count;

        for (size_t k = i; k < endIndex; ++k)
        {
            const auto& pl = *all_planets[k];

            query += "(";
            query += std::to_string(pl.id) + ", ";
            //query += "'" + pl.name + "', ";
            //query += std::to_string(pl.id_owner) + ", ";
            //query += std::to_string(pl.universe) + ", ";
            //query += std::to_string(pl.galaxy) + ", ";
            //query += std::to_string(pl.system) + ", ";
            //query += std::to_string(pl.planet) + ", ";
            query += std::to_string(pl.last_update) + ", ";
            //query += std::to_string(pl.planet_type) + ", ";
            //query += std::to_string(pl.destroyed) + ", ";

            query += std::to_string(pl.b_building) + ", ";
            query += "'" + pl.b_building_id + "', ";
            query += std::to_string(pl.b_shipyard) + ", ";
            query += "'" + pl.b_shipyard_id + "', ";
            query += std::to_string(pl.b_shipyard_plus) + ", ";
            //query += "'" + pl.image + "', ";
            //query += std::to_string(pl.diameter) + ", ";
            query += std::to_string(pl.field_current) + ", ";
            query += std::to_string(pl.field_max) + ", ";
            //query += std::to_string(pl.temp_min) + ", ";
            //query += std::to_string(pl.temp_max) + ", ";

            query += "'" + pl.eco_hash + "', ";
            query += std::to_string(pl.metal) + ", ";
            query += std::to_string(pl.metal_perhour) + ", ";
            query += std::to_string(pl.metal_max) + ", ";
            query += std::to_string(pl.crystal) + ", ";
            query += std::to_string(pl.crystal_perhour) + ", ";
            query += std::to_string(pl.crystal_max) + ", ";
            query += std::to_string(pl.deuterium) + ", ";
            query += std::to_string(pl.deuterium_perhour) + ", ";
            query += std::to_string(pl.deuterium_max) + ", ";
            query += std::to_string(pl.energy_used) + ", ";
            query += std::to_string(pl.energy) + ", ";

            query += std::to_string(pl.resource[1]) + ", ";
            query += std::to_string(pl.resource[2]) + ", ";
            query += std::to_string(pl.resource[3]) + ", ";
            query += std::to_string(pl.resource[4]) + ", ";
            query += std::to_string(pl.resource[12]) + ", ";
            query += std::to_string(pl.resource[14]) + ", ";
            query += std::to_string(pl.resource[15]) + ", ";
            query += std::to_string(pl.resource[21]) + ", ";
            query += std::to_string(pl.resource[22]) + ", ";
            query += std::to_string(pl.resource[23]) + ", ";
            query += std::to_string(pl.resource[24]) + ", ";
            query += std::to_string(pl.resource[31]) + ", ";
            query += std::to_string(pl.resource[33]) + ", ";
            query += std::to_string(pl.resource[6]) + ", ";
            query += std::to_string(pl.resource[34]) + ", ";
            query += std::to_string(pl.resource[44]) + ", ";

            query += std::to_string(pl.resource[41]) + ", ";
            query += std::to_string(pl.resource[42]) + ", ";
            query += std::to_string(pl.resource[43]) + ", ";

            query += std::to_string(pl.resource[202]) + ", ";
            query += std::to_string(pl.resource[203]) + ", ";
            query += std::to_string(pl.resource[204]) + ", ";
            query += std::to_string(pl.resource[205]) + ", ";
            query += std::to_string(pl.resource[206]) + ", ";
            query += std::to_string(pl.resource[207]) + ", ";
            query += std::to_string(pl.resource[208]) + ", ";
            query += std::to_string(pl.resource[209]) + ", ";
            query += std::to_string(pl.resource[210]) + ", ";
            query += std::to_string(pl.resource[211]) + ", ";
            query += std::to_string(pl.resource[212]) + ", ";
            query += std::to_string(pl.resource[213]) + ", ";
            query += std::to_string(pl.resource[214]) + ", ";
            query += std::to_string(pl.resource[215]) + ", ";
            query += std::to_string(pl.resource[216]) + ", ";
            query += std::to_string(pl.resource[217]) + ", ";
            query += std::to_string(pl.resource[218]) + ", ";
            query += std::to_string(pl.resource[219]) + ", ";
            query += std::to_string(pl.resource[220]) + ", ";
            query += std::to_string(pl.resource[411]) + ", ";

            query += std::to_string(pl.resource[401]) + ", ";
            query += std::to_string(pl.resource[402]) + ", ";
            query += std::to_string(pl.resource[403]) + ", ";
            query += std::to_string(pl.resource[404]) + ", ";
            query += std::to_string(pl.resource[405]) + ", ";
            query += std::to_string(pl.resource[406]) + ", ";
            query += std::to_string(pl.resource[407]) + ", ";
            query += std::to_string(pl.resource[409]) + ", ";
            query += std::to_string(pl.resource[408]) + ", ";
            query += std::to_string(pl.resource[410]) + ", ";
            query += std::to_string(pl.resource[502]) + ", ";
            query += std::to_string(pl.resource[503]) + ", ";

            query += "'" + pl.metal_mine_percent + "', ";
            query += "'" + pl.crystal_mine_percent + "', ";
            query += "'" + pl.deuterium_synthesizer_percent + "', ";
            query += "'" + pl.solar_plant_percent + "', ";
            query += "'" + pl.fusion_plant_percent + "', ";
            query += "'" + pl.solar_satellite_percent + "'";

            //query += std::to_string(pl.last_jump_time) + ", ";
            //query += std::to_string(pl.debris_metal) + ", ";
            //query += std::to_string(pl.debris_crystal) + ", ";
            //query += std::to_string(pl.id_moon) + ", ";
            //query += std::to_string(pl.is_bot ? 1 : 0) + ", ";
            //query += std::to_string(pl.last_relocate) + ", ";
            //query += std::to_string(pl.version);
            query += "), ";

            ++planet_counter;
        }

        if (query != planet_header)
        {
            size_t len = query.size();
            if (len >= 2
                && query.substr(len - 2) == ", ")
            {
                // remove comma and space
                query.pop_back();
                query.pop_back();
            }

            query += planet_footer;

            if (mysql_query(conn_, query.c_str()) != 0)
            {
                CLogger::Error(lang_.at("ids_mysql_query_error"), mysql_error(conn_));
                return false;
            }
        }

    }

    auto duration_ms = GetElapsedMilliseconds(start, end);
    auto duration_us = GetElapsedMicroseconds(start, end);
    CLogger::Info(lang_.at("ids_update_planet_bots_succ"),
        bot_counter, planet_counter, duration_ms, duration_us);
    return true;
}

bool CDatabase::AddBots(int count) 
{
    if (count <= 0)
    {
        return false;
    }
}

bool CDatabase::RemoveBots() 
{
    if (conn_ == nullptr)
    {
        return false;
    }

    std::string usersQuery =
        "DELETE FROM `" + db_uni_prefix_ + "users` WHERE `is_bot` = 1";

    std::string planetsQuery =
        "DELETE FROM `" + db_uni_prefix_ + "planets` WHERE `is_bot` = 1";

    // Transaction start
    if (mysql_autocommit(conn_, 0) != 0)
    {
        CLogger::Error(lang_.at("ids_mysql_autocommit_fail"),
            mysql_error(conn_));
        return false;
    }

    bool success = true;

    if (mysql_query(conn_, usersQuery.c_str()) != 0)
    {
        CLogger::Error(lang_.at("ids_mysql_query_error"),
            mysql_error(conn_));
        success = false;
    }

    if (success &&
        mysql_query(conn_, planetsQuery.c_str()) != 0)
    {
        CLogger::Error(lang_.at("ids_mysql_query_error"),
            mysql_error(conn_));
        success = false;
    }

    if (success)
    {
        if (mysql_commit(conn_) != 0)
        {
            CLogger::Error(lang_.at("ids_mysql_commit_fail"),
                mysql_error(conn_));
            success = false;
        }
    }

    if (!success)
    {
        mysql_rollback(conn_);
    }

    mysql_autocommit(conn_, 1);
    temp_bots_.clear();

    return success;
}
