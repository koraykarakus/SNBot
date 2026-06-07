#include "CDatabase.h"
#include "CLogger.h"
#include "CBotManager.h"
#include "SimpleIni.h"
#include "table_users.h"
#include "table_vars.h"
#include "table_config.h"

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
    // 1. EXE ile aynı klasördeki settings.ini dosyasının yolunu buluyoruz
    // std::filesystem hem Windows hem Linux platformlarında sorunsuz çalışır.
    std::filesystem::path iniPath = std::filesystem::current_path() / "settings.ini";
    std::string iniPathStr = iniPath.string();

    CSimpleIniA ini;
    ini.SetUnicode(); // Türkçe veya özel karakter desteği için

    // 2. INI dosyasını yüklemeyi dene
    SI_Error rc = ini.LoadFile(iniPathStr.c_str());

    // create settings.ini if it is not found
    if (rc < 0) 
    {
        CLogger::Error("settings.ini not found check folder and fill required info !\n");

        // Varsayılan değerleri belleğe yaz
        ini.SetValue("Database", "Host", "localhost");
        ini.SetValue("Database", "User", "username");
        ini.SetValue("Database", "Password", "password");
        ini.SetValue("Database", "DBName", "steemnova");
        ini.SetValue("Database", "Prefix", "uni1_"); 

        // Değerleri fiziksel olaraksettings.ini dosyasına kaydet
        rc = ini.SaveFile(iniPathStr.c_str());
        if (rc < 0) 
        {
            CLogger::Error("error : settings.ini cannot be saved !\n");
        }
    }

    // 3. Değerleri INI dosyasından (veya yeni oluşturulan veriden) sınıfın üye değişkenlerine oku
    // GetValue fonksiyonunun 3. parametresi, eğer INI'de o key yoksa dönecek olan "fallback" değerdir.
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
        CLogger::Error("mysql_init failed");
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
        CLogger::Error("MySQL Connection Failed. Error: {} ({})", mysql_error(m_pConn), mysql_errno(m_pConn));

        // Şimdi güvenle kapatabiliriz
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
    // -------------------------------------------------------------------------
    // STEP 1: USERS TABLE
    // -------------------------------------------------------------------------
    std::string strQuery = "SELECT * FROM `" + m_strDBPrefix + "users` WHERE is_bot = 1";

    if (mysql_query(m_pConn, strQuery.c_str()))
    {
        CLogger::Error("Query Error (Users): {}", mysql_error(m_pConn));
        return false;
    }

    MYSQL_RES* result = mysql_store_result(m_pConn);
    if (!result)
    {
        if (mysql_field_count(m_pConn) > 0) {
            CLogger::Error("Retrieve result error (Users): {}", mysql_error(m_pConn));
        }
        return false;
    }

    uint64_t rowCount = mysql_num_rows(result);
    CLogger::Info("Found {} bots in database.", rowCount);

    g_pBotManager->ClearBots();

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)))
    {
        table_users bot;

        // Tablo şemana göre milimetrik index eşlemesi:
        bot.id = row[0] ? std::stoi(row[0]) : 0;
        bot.strUserName = row[1] ? row[1] : "";

        // Araya giren şemadaki diğer kolonları atlayarak struct alanlarını dolduruyoruz
        bot.id_planet = row[9] ? std::stoi(row[9]) : 0;
        bot.universe = row[10] ? std::stoi(row[10]) : 0;
        bot.galaxy = row[11] ? std::stoi(row[11]) : 0;
        bot.system = row[12] ? std::stoi(row[12]) : 0;
        bot.planet = row[13] ? std::stoi(row[13]) : 0;

        // Araştırma kuyruk bilgileri (Tam 32. indexten başlıyor)
        bot.b_tech_planet = row[32] ? std::stoi(row[32]) : 0;
        bot.b_tech = row[33] ? std::stoi(row[33]) : 0;
        bot.b_tech_id = row[34] ? std::stoi(row[34]) : 0;
        bot.b_tech_queue = row[35] ? row[35] : "";

        // Teknolojiler ardışık olarak devam ediyor
        bot.spy_tech = row[36] ? std::stoi(row[36]) : 0;
        bot.computer_tech = row[37] ? std::stoi(row[37]) : 0;
        bot.military_tech = row[38] ? std::stoi(row[38]) : 0;
        bot.armor_tech = row[39] ? std::stoi(row[39]) : 0;
        bot.shield_tech = row[40] ? std::stoi(row[40]) : 0;
        bot.energy_tech = row[41] ? std::stoi(row[41]) : 0;
        bot.hyperspace_tech = row[42] ? std::stoi(row[42]) : 0;
        bot.combustion_tech = row[43] ? std::stoi(row[43]) : 0;
        bot.impulse_motor_tech = row[44] ? std::stoi(row[44]) : 0;
        bot.hyperspace_motor_tech = row[45] ? std::stoi(row[45]) : 0;
        bot.laser_tech = row[46] ? std::stoi(row[46]) : 0;
        bot.ion_tech = row[47] ? std::stoi(row[47]) : 0;
        bot.plasma_tech = row[48] ? std::stoi(row[48]) : 0;
        bot.intergalactic_tech = row[49] ? std::stoi(row[49]) : 0;
        bot.expedition_tech = row[50] ? std::stoi(row[50]) : 0;
        bot.metal_proc_tech = row[51] ? std::stoi(row[51]) : 0;
        bot.crystal_proc_tech = row[52] ? std::stoi(row[52]) : 0;
        bot.deuterium_proc_tech = row[53] ? std::stoi(row[53]) : 0;
        bot.graviton_tech = row[54] ? std::stoi(row[54]) : 0;

        // En sondaki is_bot kolonu (Tam 99. index)
        bot.is_bot = row[99] ? (std::stoi(row[99]) != 0) : false;

        g_pBotManager->AddBot(bot);
        CLogger::Info("Bot Loaded -> ID: {}, Name: {}", bot.id, bot.strUserName);
    }
    mysql_free_result(result);

    // Eğer hiç bot yüklenmediyse gezegen aramaya gerek yok
    if (rowCount == 0) return true;

    // -------------------------------------------------------------------------
    // STEP 2: GEZEGENLERİ (PLANETS) YÜKLE VE BOTLARLA EŞLEŞTİR
    // -------------------------------------------------------------------------
    std::string strPlanetQuery = "SELECT * FROM `" + m_strDBPrefix + "planets` WHERE is_bot = 1";

    if (mysql_query(m_pConn, strPlanetQuery.c_str()))
    {
        CLogger::Error("Query Error (Planets): {}", mysql_error(m_pConn));
        return false;
    }

    MYSQL_RES* plResult = mysql_store_result(m_pConn);
    if (!plResult)
    {
        if (mysql_field_count(m_pConn) > 0) {
            CLogger::Error("Retrieve result error (Planets): {}", mysql_error(m_pConn));
        }
        return false;
    }

    CLogger::Info("Loading planets and linking to bots...");

    MYSQL_ROW plRow;
    while ((plRow = mysql_fetch_row(plResult)))
    {
        // 1. Tablonun kolon sırasına göre table_planets struct'ını dolduruyoruz
        table_planets pl;

        pl.id = plRow[0] ? std::stoi(plRow[0]) : 0;
        pl.name = plRow[1] ? plRow[1] : "";
        pl.id_owner = plRow[2] ? std::stoi(plRow[2]) : 0; // Sahip botun ID'si
        pl.universe = plRow[3] ? std::stoi(plRow[3]) : 0;
        pl.galaxy = plRow[4] ? std::stoi(plRow[4]) : 0;
        pl.system = plRow[5] ? std::stoi(plRow[5]) : 0;
        pl.planet = plRow[6] ? std::stoi(plRow[6]) : 0;
        pl.last_update = plRow[7] ? std::stoi(plRow[7]) : 0;
        pl.planet_type = plRow[8] ? plRow[8] : "1";
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

        // Bina Kademeleri (TINYINT)
        pl.metal_mine = plRow[33] ? std::stoi(plRow[33]) : 0;
        pl.crystal_mine = plRow[34] ? std::stoi(plRow[34]) : 0;
        pl.deuterium_synthesizer = plRow[35] ? std::stoi(plRow[35]) : 0;
        pl.solar_plant = plRow[36] ? std::stoi(plRow[36]) : 0;
        pl.fusion_plant = plRow[37] ? std::stoi(plRow[37]) : 0;
        pl.robot_factory = plRow[38] ? std::stoi(plRow[38]) : 0;
        pl.nanite_factory = plRow[39] ? std::stoi(plRow[39]) : 0;
        pl.shipyard = plRow[40] ? std::stoi(plRow[40]) : 0;
        pl.metal_storage = plRow[41] ? std::stoi(plRow[41]) : 0;
        pl.crystal_storage = plRow[42] ? std::stoi(plRow[42]) : 0;
        pl.deuterium_tank = plRow[43] ? std::stoi(plRow[43]) : 0;
        pl.research_lab = plRow[44] ? std::stoi(plRow[44]) : 0;
        pl.terraformer = plRow[45] ? std::stoi(plRow[45]) : 0;
        pl.university = plRow[46] ? std::stoi(plRow[46]) : 0;
        pl.ally_deposit = plRow[47] ? std::stoi(plRow[47]) : 0;
        pl.missile_silo = plRow[48] ? std::stoi(plRow[48]) : 0;
        pl.lunar_base = plRow[49] ? std::stoi(plRow[49]) : 0;
        pl.phalanx = plRow[50] ? std::stoi(plRow[50]) : 0;
        pl.jump_gate = plRow[51] ? std::stoi(plRow[51]) : 0;

        // Filolar (BIGINT -> unsigned long long için std::stoull (string to unsigned long long))
        pl.small_cargo = plRow[52] ? std::stoull(plRow[52]) : 0;
        pl.big_cargo = plRow[53] ? std::stoull(plRow[53]) : 0;
        pl.light_hunter = plRow[54] ? std::stoull(plRow[54]) : 0;
        pl.heavy_hunter = plRow[55] ? std::stoull(plRow[55]) : 0;
        pl.cruiser = plRow[56] ? std::stoull(plRow[56]) : 0;
        pl.battle_ship = plRow[57] ? std::stoull(plRow[57]) : 0;
        pl.colony_ship = plRow[58] ? std::stoull(plRow[58]) : 0;
        pl.recycler = plRow[59] ? std::stoull(plRow[59]) : 0;
        pl.espionage_probe = plRow[60] ? std::stoull(plRow[60]) : 0;
        pl.bomber_ship = plRow[61] ? std::stoull(plRow[61]) : 0;
        pl.solar_satellite = plRow[62] ? std::stoull(plRow[62]) : 0;
        pl.destroyer = plRow[63] ? std::stoull(plRow[63]) : 0;
        pl.death_star = plRow[64] ? std::stoull(plRow[64]) : 0;
        pl.battle_cruiser = plRow[65] ? std::stoull(plRow[65]) : 0;
        pl.black_moon = plRow[66] ? std::stoull(plRow[66]) : 0;
        pl.ev_transporter = plRow[67] ? std::stoull(plRow[67]) : 0;
        pl.star_crasher = plRow[68] ? std::stoull(plRow[68]) : 0;
        pl.giga_recycler = plRow[69] ? std::stoull(plRow[69]) : 0;
        pl.dm_ship = plRow[70] ? std::stoll(plRow[70]) : 0; // SIGNED BIGINT (stoll)
        pl.orbital_station = plRow[71] ? std::stoull(plRow[71]) : 0;

        // Savunma
        pl.rocket_launcher = plRow[72] ? std::stoull(plRow[72]) : 0;
        pl.light_laser = plRow[73] ? std::stoull(plRow[73]) : 0;
        pl.heavy_laser = plRow[74] ? std::stoull(plRow[74]) : 0;
        pl.gauss_cannon = plRow[75] ? std::stoull(plRow[75]) : 0;
        pl.ion_cannon = plRow[76] ? std::stoull(plRow[76]) : 0;
        pl.plasma_turret = plRow[77] ? std::stoull(plRow[77]) : 0;
        pl.small_protection_shield = plRow[78] ? std::stoi(plRow[78]) : 0;
        pl.planet_protector = plRow[79] ? std::stoi(plRow[79]) : 0;
        pl.big_protection_shield = plRow[80] ? std::stoi(plRow[80]) : 0;
        pl.graviton_cannon = plRow[81] ? std::stoull(plRow[81]) : 0;
        pl.interceptor_misil = plRow[82] ? std::stoull(plRow[82]) : 0;
        pl.interplanetary_misil = plRow[83] ? std::stoull(plRow[83]) : 0;

        // Yüzdeler
        pl.metal_mine_percent = plRow[84] ? plRow[84] : "10";
        pl.crystal_mine_percent = plRow[85] ? plRow[85] : "10";
        pl.deuterium_synthesizer_percent = plRow[86] ? plRow[86] : "10";
        pl.solar_plant_percent = plRow[87] ? plRow[87] : "10";
        pl.fusion_plant_percent = plRow[88] ? plRow[88] : "10";
        pl.solar_satellite_percent = plRow[89] ? plRow[89] : "10";

        // Ekstra Sistem Değerleri
        pl.last_jump_time = plRow[90] ? std::stoi(plRow[90]) : 0;
        pl.debris_metal = plRow[91] ? std::stod(plRow[91]) : 0.0;
        pl.debris_crystal = plRow[92] ? std::stod(plRow[92]) : 0.0;
        pl.id_moon = plRow[93] ? std::stoi(plRow[93]) : 0;
        pl.is_bot = plRow[94] ? (std::stoi(plRow[94]) != 0) : false;
        pl.last_relocate = plRow[95] ? std::stoi(plRow[95]) : 0;
        pl.version = plRow[96] ? std::stoull(plRow[96]) : 0;

        // 2. İşin Büyüsü: Gezegenin sahibini BotManager vasıtasıyla bulup vektörüne atıyoruz
        table_users* pTargetBot = g_pBotManager->GetBotRef(pl.id_owner);
        if (pTargetBot != nullptr)
        {
            pTargetBot->vecPlanets.push_back(pl);
            CLogger::Info("Planet '{}' [ID: {}] linked to Bot ID: {}", pl.name, pl.id, pl.id_owner);
        }
        else
        {
            CLogger::Warn("Planet ID {} has owner ID {} but no matching bot was found!", pl.id, pl.id_owner);
        }
    }

    mysql_free_result(plResult);
    CLogger::Info("All bots and planets loaded successfully.");
    return true;
}

bool CDatabase::UpdateBots()
{
    const std::vector<table_users>& pBots = g_pBotManager->GetBots();

    if (pBots.empty())
    {
        return true;
    }

    if (m_pConn == nullptr) return false;

    const size_t BATCH_SIZE = 50; // Hem botlar hem gezegenler için paket boyutu

    // =========================================================================
    // KISIM 1: BOT TEKNOLOJİLERİNİ GÜNCELLE (USERS TABLOSU)
    // =========================================================================
    const std::string strUserHeader = "INSERT INTO `" + m_strDBPrefix + "users` ("
        "id, b_tech_planet, b_tech, b_tech_id, b_tech_queue, spy_tech, computer_tech, "
        "military_tech, armor_tech, shield_tech, energy_tech, hyperspace_tech, "
        "combustion_tech, impulse_motor_tech, hyperspace_motor_tech, laser_tech, "
        "ion_tech, plasma_tech, intergalactic_tech, expedition_tech, "
        "metal_proc_tech, crystal_proc_tech, deuterium_proc_tech, graviton_tech) VALUES ";

    const std::string strUserFooter = " ON DUPLICATE KEY UPDATE "
        "b_tech_planet = VALUES(b_tech_planet), b_tech = VALUES(b_tech), b_tech_id = VALUES(b_tech_id), b_tech_queue = VALUES(b_tech_queue), "
        "spy_tech = VALUES(spy_tech), computer_tech = VALUES(computer_tech), military_tech = VALUES(military_tech), armor_tech = VALUES(armor_tech), "
        "shield_tech = VALUES(shield_tech), energy_tech = VALUES(energy_tech), hyperspace_tech = VALUES(hyperspace_tech), combustion_tech = VALUES(combustion_tech), "
        "impulse_motor_tech = VALUES(impulse_motor_tech), hyperspace_motor_tech = VALUES(hyperspace_motor_tech), laser_tech = VALUES(laser_tech), "
        "ion_tech = VALUES(ion_tech), plasma_tech = VALUES(plasma_tech), intergalactic_tech = VALUES(intergalactic_tech), expedition_tech = VALUES(expedition_tech), "
        "metal_proc_tech = VALUES(metal_proc_tech), crystal_proc_tech = VALUES(crystal_proc_tech), deuterium_proc_tech = VALUES(deuterium_proc_tech), "
        "graviton_tech = VALUES(graviton_tech);";

    // Tüm gezegenleri tek bir havuzda toplamak için geçici vektör
    std::vector<table_planets> vecAllPlanets;

    // Botları 50'şerli gruplarla veritabanına gönderiyoruz
    for (size_t i = 0; i < pBots.size(); i += BATCH_SIZE)
    {
        std::string strQuery = strUserHeader;
        size_t endIndex = ((i + BATCH_SIZE) < pBots.size()) ? (i + BATCH_SIZE) : pBots.size();

        for (size_t k = i; k < endIndex; ++k)
        {
            const auto& cBot = pBots[k];

            // Gezegenleri daha sonra toplu işlemek üzere havaza topluyoruz
            for (const auto& planet : cBot.vecPlanets) {
                vecAllPlanets.push_back(planet);
            }

            strQuery += "(";
            strQuery += std::to_string(cBot.id) + ", ";
            strQuery += std::to_string(cBot.b_tech_planet) + ", ";
            strQuery += std::to_string(cBot.b_tech) + ", ";
            strQuery += std::to_string(cBot.b_tech_id) + ", ";
            strQuery += "'" + cBot.b_tech_queue + "', ";
            strQuery += std::to_string(cBot.spy_tech) + ", ";
            strQuery += std::to_string(cBot.computer_tech) + ", ";
            strQuery += std::to_string(cBot.military_tech) + ", ";
            strQuery += std::to_string(cBot.armor_tech) + ", ";
            strQuery += std::to_string(cBot.shield_tech) + ", ";
            strQuery += std::to_string(cBot.energy_tech) + ", ";
            strQuery += std::to_string(cBot.hyperspace_tech) + ", ";
            strQuery += std::to_string(cBot.combustion_tech) + ", ";
            strQuery += std::to_string(cBot.impulse_motor_tech) + ", ";
            strQuery += std::to_string(cBot.hyperspace_motor_tech) + ", ";
            strQuery += std::to_string(cBot.laser_tech) + ", ";
            strQuery += std::to_string(cBot.ion_tech) + ", ";
            strQuery += std::to_string(cBot.plasma_tech) + ", ";
            strQuery += std::to_string(cBot.intergalactic_tech) + ", ";
            strQuery += std::to_string(cBot.expedition_tech) + ", ";
            strQuery += std::to_string(cBot.metal_proc_tech) + ", ";
            strQuery += std::to_string(cBot.crystal_proc_tech) + ", ";
            strQuery += std::to_string(cBot.deuterium_proc_tech) + ", ";
            strQuery += std::to_string(cBot.graviton_tech);
            strQuery += ")";

            if (k < endIndex - 1) {
                strQuery += ", ";
            }
        }

        strQuery += strUserFooter;

        if (mysql_query(m_pConn, strQuery.c_str()) != 0)
        {
            CLogger::Error("[CDatabase] UpdateBots (Users) Paket Hatasi (Index {}-{}): {}", i, endIndex - 1, mysql_error(m_pConn));
            return false;
        }
    }

    // =========================================================================
    // KISIM 2: GEZEGENLERİ GÜNCELLE (PLANETS TABLOSU) - 50'şerli Paketlerle
    // =========================================================================
    if (vecAllPlanets.empty())
    {
        return true;
    }

    // Gezegen şemasına göre Insert sorgu başlığı (Milimetrik şema sırası)
    const std::string strPlanetHeader = "INSERT INTO `" + m_strDBPrefix + "planets` ("
        "id, name, id_owner, universe, galaxy, system, planet, last_update, planet_type, destroyed, "
        "b_building, b_building_id, b_shipyard, b_shipyard_id, b_shipyard_plus, image, diameter, field_current, field_max, temp_min, temp_max, "
        "eco_hash, metal, metal_perhour, metal_max, crystal, crystal_perhour, crystal_max, deuterium, deuterium_perhour, deuterium_max, energy_used, energy, "
        "metal_mine, crystal_mine, deuterium_synthesizer, solar_plant, fusion_plant, robot_factory, nanite_factory, shipyard, metal_storage, crystal_storage, deuterium_tank, research_lab, terraformer, university, ally_deposit, missile_silo, "
        "lunar_base, phalanx, jump_gate, small_cargo, big_cargo, light_hunter, heavy_hunter, cruiser, battle_ship, colony_ship, recycler, espionage_probe, bomber_ship, solar_satellite, destroyer, death_star, battle_cruiser, black_moon, ev_transporter, star_crasher, giga_recycler, dm_ship, orbital_station, "
        "rocket_launcher, light_laser, heavy_laser, gauss_cannon, ion_cannon, plasma_turret, small_protection_shield, planet_protector, big_protection_shield, graviton_cannon, interceptor_misil, interplanetary_misil, "
        "metal_mine_percent, crystal_mine_percent, deuterium_synthesizer_percent, solar_plant_percent, fusion_plant_percent, solar_satellite_percent, "
        "last_jump_time, debris_metal, debris_crystal, id_moon, is_bot, last_relocate, version) VALUES ";

    // Çakışma durumunda güncellenecek alanların tamamı
    const std::string strPlanetFooter = " ON DUPLICATE KEY UPDATE "
        "name=VALUES(name), id_owner=VALUES(id_owner), universe=VALUES(universe), galaxy=VALUES(galaxy), system=VALUES(system), planet=VALUES(planet), last_update=VALUES(last_update), planet_type=VALUES(planet_type), destroyed=VALUES(destroyed), "
        "b_building=VALUES(b_building), b_building_id=VALUES(b_building_id), b_shipyard=VALUES(b_shipyard), b_shipyard_id=VALUES(b_shipyard_id), b_shipyard_plus=VALUES(b_shipyard_plus), image=VALUES(image), diameter=VALUES(diameter), field_current=VALUES(field_current), field_max=VALUES(field_max), temp_min=VALUES(temp_min), temp_max=VALUES(temp_max), "
        "eco_hash=VALUES(eco_hash), metal=VALUES(metal), metal_perhour=VALUES(metal_perhour), metal_max=VALUES(metal_max), crystal=VALUES(crystal), crystal_perhour=VALUES(crystal_perhour), crystal_max=VALUES(crystal_max), deuterium=VALUES(deuterium), deuterium_perhour=VALUES(deuterium_perhour), deuterium_max=VALUES(deuterium_max), energy_used=VALUES(energy_used), energy=VALUES(energy), "
        "metal_mine=VALUES(metal_mine), crystal_mine=VALUES(crystal_mine), deuterium_synthesizer=VALUES(deuterium_synthesizer), solar_plant=VALUES(solar_plant), fusion_plant=VALUES(fusion_plant), robot_factory=VALUES(robot_factory), nanite_factory=VALUES(nanite_factory), shipyard=VALUES(shipyard), metal_storage=VALUES(metal_storage), crystal_storage=VALUES(crystal_storage), deuterium_tank=VALUES(deuterium_tank), research_lab=VALUES(research_lab), terraformer=VALUES(terraformer), university=VALUES(university), ally_deposit=VALUES(ally_deposit), missile_silo=VALUES(missile_silo), "
        "lunar_base=VALUES(lunar_base), phalanx=VALUES(phalanx), jump_gate=VALUES(jump_gate), small_cargo=VALUES(small_cargo), big_cargo=VALUES(big_cargo), light_hunter=VALUES(light_hunter), heavy_hunter=VALUES(heavy_hunter), cruiser=VALUES(cruiser), battle_ship=VALUES(battle_ship), colony_ship=VALUES(colony_ship), recycler=VALUES(recycler), espionage_probe=VALUES(espionage_probe), bomber_ship=VALUES(bomber_ship), solar_satellite=VALUES(solar_satellite), destroyer=VALUES(destroyer), death_star=VALUES(death_star), battle_cruiser=VALUES(battle_cruiser), black_moon=VALUES(black_moon), ev_transporter=VALUES(ev_transporter), star_crasher=VALUES(star_crasher), giga_recycler=VALUES(giga_recycler), dm_ship=VALUES(dm_ship), orbital_station=VALUES(orbital_station), "
        "rocket_launcher=VALUES(rocket_launcher), light_laser=VALUES(light_laser), heavy_laser=VALUES(heavy_laser), gauss_cannon=VALUES(gauss_cannon), ion_cannon=VALUES(ion_cannon), plasma_turret=VALUES(plasma_turret), small_protection_shield=VALUES(small_protection_shield), planet_protector=VALUES(planet_protector), big_protection_shield=VALUES(big_protection_shield), graviton_cannon=VALUES(graviton_cannon), interceptor_misil=VALUES(interceptor_misil), interplanetary_misil=VALUES(interplanetary_misil), "
        "metal_mine_percent=VALUES(metal_mine_percent), crystal_mine_percent=VALUES(crystal_mine_percent), deuterium_synthesizer_percent=VALUES(deuterium_synthesizer_percent), solar_plant_percent=VALUES(solar_plant_percent), fusion_plant_percent=VALUES(fusion_plant_percent), solar_satellite_percent=VALUES(solar_satellite_percent), "
        "last_jump_time=VALUES(last_jump_time), debris_metal=VALUES(debris_metal), debris_crystal=VALUES(debris_crystal), id_moon=VALUES(id_moon), is_bot=VALUES(is_bot), last_relocate=VALUES(last_relocate), version=VALUES(version);";

    // Toplanan gezegen havuzunu tam 50'şerli paketleyerek gönderiyoruz
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
            strQuery += "'" + pl.planet_type + "', ";
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

            strQuery += std::to_string(pl.metal_mine) + ", ";
            strQuery += std::to_string(pl.crystal_mine) + ", ";
            strQuery += std::to_string(pl.deuterium_synthesizer) + ", ";
            strQuery += std::to_string(pl.solar_plant) + ", ";
            strQuery += std::to_string(pl.fusion_plant) + ", ";
            strQuery += std::to_string(pl.robot_factory) + ", ";
            strQuery += std::to_string(pl.nanite_factory) + ", ";
            strQuery += std::to_string(pl.shipyard) + ", ";
            strQuery += std::to_string(pl.metal_storage) + ", ";
            strQuery += std::to_string(pl.crystal_storage) + ", ";
            strQuery += std::to_string(pl.deuterium_tank) + ", ";
            strQuery += std::to_string(pl.research_lab) + ", ";
            strQuery += std::to_string(pl.terraformer) + ", ";
            strQuery += std::to_string(pl.university) + ", ";
            strQuery += std::to_string(pl.ally_deposit) + ", ";
            strQuery += std::to_string(pl.missile_silo) + ", ";

            strQuery += std::to_string(pl.lunar_base) + ", ";
            strQuery += std::to_string(pl.phalanx) + ", ";
            strQuery += std::to_string(pl.jump_gate) + ", ";

            strQuery += std::to_string(pl.small_cargo) + ", ";
            strQuery += std::to_string(pl.big_cargo) + ", ";
            strQuery += std::to_string(pl.light_hunter) + ", ";
            strQuery += std::to_string(pl.heavy_hunter) + ", ";
            strQuery += std::to_string(pl.cruiser) + ", ";
            strQuery += std::to_string(pl.battle_ship) + ", ";
            strQuery += std::to_string(pl.colony_ship) + ", ";
            strQuery += std::to_string(pl.recycler) + ", ";
            strQuery += std::to_string(pl.espionage_probe) + ", ";
            strQuery += std::to_string(pl.bomber_ship) + ", ";
            strQuery += std::to_string(pl.solar_satellite) + ", ";
            strQuery += std::to_string(pl.destroyer) + ", ";
            strQuery += std::to_string(pl.death_star) + ", ";
            strQuery += std::to_string(pl.battle_cruiser) + ", ";
            strQuery += std::to_string(pl.black_moon) + ", ";
            strQuery += std::to_string(pl.ev_transporter) + ", ";
            strQuery += std::to_string(pl.star_crasher) + ", ";
            strQuery += std::to_string(pl.giga_recycler) + ", ";
            strQuery += std::to_string(pl.dm_ship) + ", ";
            strQuery += std::to_string(pl.orbital_station) + ", ";

            strQuery += std::to_string(pl.rocket_launcher) + ", ";
            strQuery += std::to_string(pl.light_laser) + ", ";
            strQuery += std::to_string(pl.heavy_laser) + ", ";
            strQuery += std::to_string(pl.gauss_cannon) + ", ";
            strQuery += std::to_string(pl.ion_cannon) + ", ";
            strQuery += std::to_string(pl.plasma_turret) + ", ";
            strQuery += std::to_string(pl.small_protection_shield) + ", ";
            strQuery += std::to_string(pl.planet_protector) + ", ";
            strQuery += std::to_string(pl.big_protection_shield) + ", ";
            strQuery += std::to_string(pl.graviton_cannon) + ", ";
            strQuery += std::to_string(pl.interceptor_misil) + ", ";
            strQuery += std::to_string(pl.interplanetary_misil) + ", ";

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
            strQuery += ")";

            if (k < endIndex - 1) {
                strQuery += ", ";
            }
        }

        strQuery += strPlanetFooter;

        if (mysql_query(m_pConn, strQuery.c_str()) != 0)
        {
            CLogger::Error("[CDatabase] UpdateBots (Planets) Paket Hatasi (Index {}-{}): {}", i, endIndex - 1, mysql_error(m_pConn));
            return false;
        }
    }

    CLogger::Info("[CDatabase] All bots and their planets updated successfully in database.");
    return true;
}

bool CDatabase::LoadVars()
{
    if (m_pConn == nullptr)
    {
        CLogger::Error("[CDatabase] LoadVars Hatasi: Veritabanı bağlantısı yok.");
        return false;
    }

    std::string strQuery = "SELECT * FROM `" + m_strDBPrefix + "vars`";

    if (mysql_query(m_pConn, strQuery.c_str()) != 0)
    {
        CLogger::Error("[CDatabase] LoadVars Sorgu Hatasi: {}", mysql_error(m_pConn));
        return false;
    }

    MYSQL_RES* result = mysql_store_result(m_pConn);
    if (result == nullptr)
    {
        CLogger::Error("[CDatabase] LoadVars Result Alinamadi: {}", mysql_error(m_pConn));
        return false;
    }

    g_pBotManager->ClearVars();

    MYSQL_ROW row;
    int loadedCount = 0;

    while ((row = mysql_fetch_row(result)))
    {
        // Şemana göre milimetrik index haritası:
        // row[0] = element_id
        // row[1] = name
        // row[5] = factor
        // row[7] = cost901 (metal)
        // row[8] = cost902 (crystal)
        // row[9] = cost903 (deuterium)

        if (!row[0] || !row[1]) continue; // Kritik alanlar boşsa atla

        table_vars item;
        item.elementID = std::stoi(row[0]);
        item.name = row[1];
        item.factor = row[5] ? std::stod(row[5]) : 1.0;
        item.costMetal = row[7] ? std::stod(row[7]) : 0.0;
        item.costCrystal = row[8] ? std::stod(row[8]) : 0.0;
        item.costDeuterium = row[9] ? std::stod(row[9]) : 0.0;

        // İsme göre hızlıca erişebilmek için haritaya (map) ekliyoruz
        g_pBotManager->AddGameVar(item.elementID, item);
        loadedCount++;
    }

    mysql_free_result(result);
    CLogger::Info("[CDatabase] {} adet oyun degiskeni (vars) basariyla RAM'e yuklendi.", loadedCount);

    return true;
}

bool CDatabase::LoadConfig() 
{
    if (m_pConn == nullptr)
    {
        CLogger::Error("[CDatabase] LoadConfig - no connection.");
        return false;
    }

    std::string strQuery = "SELECT * FROM `" + m_strDBPrefix + "config`";

    if (mysql_query(m_pConn, strQuery.c_str()) != 0)
    {
        CLogger::Error("[CDatabase] LoadConfig query error: {}", mysql_error(m_pConn));
        return false;
    }

    MYSQL_RES* result = mysql_store_result(m_pConn);
    if (result == nullptr)
    {
        CLogger::Error("[CDatabase] LoadConfig Result Error: {}", mysql_error(m_pConn));
        return false;
    }

	g_pBotManager->ClearConfig();

    MYSQL_ROW row;
    int loadedCount = 0;

    while ((row = mysql_fetch_row(result)))
    {
        table_config item;
        item.uni = std::stoi(row[0]);
        item.game_speed = std::stoll(row[4]);
        g_pBotManager->AddConfig(item.uni, item);
        loadedCount++;
    }

    mysql_free_result(result);
    CLogger::Info("[CDatabase] {}x config row has been loaded. ", loadedCount);

    return true;
}