#include "CBotManager.h"
#include "CLogger.h"
#include "CDatabase.h"

extern bool g_bRunning;

CBotManager g_BotManager;

CBotManager::CBotManager()
{
	// todo: reserve if constant number of boss as user input
	m_vecBots = {};
    m_bFirstRun = true;
    m_timeLastRun = 0;
}

CBotManager::~CBotManager()
{
	m_vecBots.clear();
}

void CBotManager::Run()
{
    CLogger::Info("Bot yoneticisi thread'i baslatildi.");

    // --- ÖLÜMCÜL NOKTA: Thread'in ölmemesi için döngü şart! ---
    while (g_bRunning)
    {
        time_t timeNow = std::time(nullptr);

        // Eğer ilk run değilse ve 15 saniye dolmadıysa...
        if (!m_bFirstRun && (timeNow < m_timeLastRun + static_cast<time_t>(wait_time)))
        {
            // return; YAPARSAN THREAD ÖLÜR! 
            // O yüzden 'continue' diyerek döngünün başına dönüyoruz ve beklemeye devam ediyoruz.
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // İşlemciyi yormamak için kısa uyku
            continue;
        }

        HandleBuildings();
        g_Database.UpdateBots();
        g_Database.LoadBots();
        CLogger::Info("Bot buildings has been handled.");

        // Durumları güncelle
        m_bFirstRun = false;
        m_timeLastRun = timeNow;

        // Bir sonraki kontrol için 1 saniye uyu
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    CLogger::Info("Bot yoneticisi döngüden cikti, thread kapatiliyor...");
}


void CBotManager::HandleBuildings()
{
    // Doğrudan uni1_vars tablosundaki gerçek element_id listeleri (Hepsi 1 arttırıldı)
    const std::vector<int> building_list_destroyer = { 4, 1, 1, 4, 1, 1, 4, 1, 4, 2, 2, 2, 4, 1, 2, 4, 3, 3, 3, 4, 4, 3, 4, 3, 2, 4, 3, 6, 6, 12, 22, 24, 24, 8, 8, 12, 12, 24, 8, 17, 17, 18, 18, 18, 25, 25, 17, 10, 25, 22, 22, 12, 12, 12, 24, 24, 24, 20, 20, 4, 1, 1, 4, 2, 2, 2, 2, 1, 4, 3, 1, 9, 8, 21, 21, 31, 4, 1, 2, 1, 9, 10, 11, 4, 2, 2, 1, 4, 3, 2, 8, 22, 27, 27, 27, 27, 27, 28, 28, 25, 9, 10, 11, 4, 22, 1, 2, 3, 12, 12, 32, 33, 34, 32, 33, 32, 33, 34, 32, 33, 32, 33, 34, 34, 4, 3, 1, 18, 10, 9, 11, 31, 21, 21, 21, 4, 3, 2, 1, 34, 33, 32, 34, 18, 8, 8, 20, 20, 20, 23, 23, 23, 9, 10, 11, 26, 26, 26, 26, 6, 6, 6, 4, 1, 2, 18, 31, 19, 20, 21, 9, 10, 11, 1, 19, 8, 8, 4, 1, 3, 33, 34, 32, 23, 23, 6, 6, 4, 2, 3, 1, 9, 10, 26, 33, 34, 9, 4, 11, 2, 2, 3, 26, 18, 18, 18, 6, 6, 6, 18, 7, 19, 19, 19, 31, 31, 19 };
    const std::vector<int> building_list_deathstar = { 4, 1, 1, 4, 1, 1, 4, 1, 4, 2, 2, 2, 4, 1, 2, 4, 3, 3, 3, 4, 4, 3, 4, 3, 2, 4, 3, 6, 6, 12, 22, 24, 24, 8, 8, 12, 12, 24, 8, 17, 17, 18, 18, 18, 25, 25, 17, 10, 25, 22, 22, 12, 12, 12, 24, 24, 24, 20, 20, 4, 1, 1, 4, 2, 2, 2, 2, 1, 4, 3, 1, 9, 8, 21, 21, 31, 4, 1, 2, 1, 9, 10, 11, 4, 2, 2, 1, 4, 3, 2, 8, 22, 27, 27, 27, 27, 27, 28, 28, 25, 9, 10, 11, 4, 22, 1, 2, 3, 12, 12, 32, 33, 34, 32, 33, 32, 33, 34, 32, 33, 32, 33, 34, 34, 4, 3, 1, 18, 10, 9, 11, 31, 21, 21, 21, 4, 3, 2, 1, 34, 33, 32, 34, 18, 8, 8, 20, 20, 20, 23, 23, 23, 9, 10, 11, 26, 26, 26, 26, 6, 6, 6, 4, 1, 2, 18, 31, 19, 20, 21, 9, 10, 11, 1, 19, 8, 8, 4, 1, 3, 33, 34, 32, 23, 23, 6, 6, 4, 2, 3, 1, 9, 10, 26, 33, 34, 9, 4, 11, 2, 2, 3, 26, 18, 18, 18, 6, 6, 6, 18, 7, 19, 19, 19, 31, 31, 19, 4, 3, 1, 4, 2, 1, 3, 32, 32, 33, 34, 32, 33, 34, 23, 10, 9, 11, 26, 4, 1, 2, 3, 12, 12, 8, 12, 12, 35, 8, 8, 9, 10, 11, 10, 9, 19, 20, 21, 19, 20, 21, 19, 20, 21, 19, 20, 21, 4, 2, 3, 4, 1, 2, 3, 31, 31, 31, 31 };

    double game_speed = 1.0;
    time_t currentTime = std::time(nullptr);

    for (auto& bot : m_vecBots)
    {
        const std::vector<int>* pTargetList = (bot.id % 2 != 0) ? &building_list_destroyer : &building_list_deathstar;
        const std::vector<int>& target_building_list = *pTargetList;

        for (auto& planet : bot.vecPlanets)
        {
            if (planet.b_building > 0 || bot.b_tech > 0)
            {
                continue;
            }

            // Simülasyon için kademeleri eşleştirirken yine elementID'leri doğrudan indeks gibi kullanabiliriz.
            // un1_vars tablosunda element_id'ler maksimum 199 (graviton) olduğu için 200 elemanlık bir dizi açıyoruz.
            int current_levels[200] = { 0 };

            // Binalar (Gerçek ID'leri ile)
            current_levels[1] = planet.metal_mine;
            current_levels[2] = planet.crystal_mine;
            current_levels[3] = planet.deuterium_synthesizer;
            current_levels[4] = planet.solar_plant;
            current_levels[12] = planet.fusion_plant;
            current_levels[14] = planet.robot_factory;
            current_levels[15] = planet.nanite_factory;
            current_levels[21] = planet.shipyard;
            current_levels[22] = planet.metal_storage;
            current_levels[23] = planet.crystal_storage;
            current_levels[24] = planet.deuterium_tank;
            current_levels[31] = planet.research_lab;
            current_levels[33] = planet.terraformer;
            current_levels[34] = planet.university;
            current_levels[36] = planet.ally_deposit;
            current_levels[44] = planet.missile_silo;

            // Teknolojiler (Gerçek ID'leri ile)
            current_levels[106] = bot.spy_tech;
            current_levels[108] = bot.computer_tech;
            current_levels[109] = bot.military_tech;
            current_levels[110] = bot.armor_tech;
            current_levels[111] = bot.shield_tech;
            current_levels[113] = bot.energy_tech;
            current_levels[114] = bot.hyperspace_tech;
            current_levels[115] = bot.combustion_tech;
            current_levels[116] = bot.impulse_motor_tech;
            current_levels[117] = bot.hyperspace_motor_tech;
            current_levels[120] = bot.laser_tech;
            current_levels[121] = bot.ion_tech;
            current_levels[122] = bot.plasma_tech;
            current_levels[123] = bot.intergalactic_tech;
            current_levels[124] = bot.expedition_tech;
            current_levels[131] = bot.metal_proc_tech;
            current_levels[132] = bot.crystal_proc_tech;
            current_levels[133] = bot.deuterium_proc_tech;
            current_levels[199] = bot.graviton_tech;

            int simulated_levels[200] = { 0 };
            int target_element_id = -1;

            // Listeyi tarıyoruz (Artık listeden çıkan değer direkt gerçek ID)
            for (size_t m = 0; m < target_building_list.size(); ++m)
            {
                int element_id = target_building_list[m];
                simulated_levels[element_id]++;

                if (simulated_levels[element_id] > current_levels[element_id])
                {
                    target_element_id = element_id;
                    break;
                }
            }

            if (target_element_id == -1)
            {
                CLogger::Info("The list to build has been completed for planet: {}", planet.name);
                continue;
            }

            // ARTIK MAPPING YOK! Doğrudan listeden bulduğumuz ID'yi haritada aratıyoruz
            auto it = m_mapGameVars.find(target_element_id);
            if (it == m_mapGameVars.end())
            {
                CLogger::Error("Vars tablosunda element_id '{}' bulunamadi!", target_element_id);
                continue;
            }

            const table_vars& varItem = it->second;
            int current_level = current_levels[target_element_id];
            int level_up = current_level + 1;

            double required_metal = std::round(varItem.costMetal * std::pow(varItem.factor, current_level));
            double required_crystal = std::round(varItem.costCrystal * std::pow(varItem.factor, current_level));
            double required_deuterium = std::round(varItem.costDeuterium * std::pow(varItem.factor, current_level));

            if (planet.metal < required_metal || planet.crystal < required_crystal || planet.deuterium < required_deuterium)
            {
                continue;
            }

            planet.metal -= required_metal;
            planet.crystal -= required_crystal;
            planet.deuterium -= required_deuterium;

            // Bina mı yoksa Teknoloji mi ayrımı (Ogame standartlarında ID'si 100'den küçük olanlar binadır)
            if (target_element_id < 100)
            {
                double baseTime = (required_metal + required_crystal + 3.0) / (game_speed * (1.0 + planet.robot_factory));
                double buildTime = baseTime * std::pow(0.5, planet.nanite_factory) * varItem.factor;
                time_t endTime = currentTime + static_cast<time_t>(buildTime);

                planet.b_building = endTime;

                planet.b_building_id = "a:1:{i:0;a:5:{i:0;i:" + std::to_string(varItem.elementID) +
                    ";i:1;i:" + std::to_string(level_up) +
                    ";i:2;i:" + std::to_string(static_cast<int>(buildTime)) +
                    ";i:3;i:" + std::to_string(endTime) + ";i:4;s:5:\"build\";}}";

                CLogger::Info("Bot ID {} [Planet: {}] -> Started Building: {} Level {}", bot.id, planet.name, varItem.name, level_up);
            }
            else // ID >= 100 ise Teknolojidir (106, 108 vb.)
            {
                double buildTime = ((required_metal + required_crystal + 3.0) / (1000.0 * level_up)) / game_speed * (1.0 + planet.research_lab);
                time_t endTime = currentTime + static_cast<time_t>(buildTime);

                bot.b_tech_planet = planet.id;
                bot.b_tech = endTime;
                bot.b_tech_id = varItem.elementID;

                bot.b_tech_queue = "a:1:{i:0;a:5:{i:0;i:" + std::to_string(varItem.elementID) +
                    ";i:1;i:" + std::to_string(level_up) +
                    ";i:2;i:" + std::to_string(static_cast<int>(buildTime)) +
                    ";i:3;i:" + std::to_string(endTime) +
                    ";i:4;i:" + std::to_string(planet.id) + ";}}";

                CLogger::Info("Bot ID {} [Planet: {}] -> Started Research: {} Level {}", bot.id, planet.name, varItem.name, level_up);
            }
        }
    }
}
