// implementation of CBotManager.h

#include "CBotManager.h"
#include "CLogger.h"
#include "CDatabase.h"
#include <bcrypt.h>
#include <iostream>
#include <string>
#include <set>
#include <tuple>

bool CBotManager::ProcessPendingRequests()
{
	bool processed = false;

	while (true)
	{
		cmd_queue cmd;

		{
			std::lock_guard<std::mutex> lock(mutex_command_);

			if (commands_.empty())
				break;

			cmd = commands_.front();
			commands_.pop();
		}

		processed = true;

		switch (cmd.type)
		{
			case 1:
				CreateBots(cmd);
				break;
			case 2:
				RemoveBots();
				break;
			// add metal
			case 3:
				AddMetal(cmd);
				break;
			// add crystal
			case 4:
				AddCrystal(cmd);
				break;
			// add deuterium
			case 5:
				AddDeu(cmd);
				break;
			// add dm
			case 6:
				AddDm(cmd);
				break;
			default:
				break;
		}
	}

	return processed;
}

void CBotManager::CreateBots(const cmd_queue& cmd)
{
	if (cmd.count <= 0
		|| cmd.universe <= 0)
	{
		return;
	}

	const auto* config_ptr = GetConfigByUniID(cmd.universe);
	if (config_ptr == nullptr)
	{
		// wrong uni id entered by user.
		return;
	}

	if (database_ == nullptr) return;
	// refresh settlement info
	bool res = database_->LoadSettlementData();
	if (!res) return;
	// refresh memory
	settlement_data_ptr_ = database_->GetSettlementData();

	// make set to fasten
	std::set<std::tuple<int, int, int>> occupied_locations;
	for (const auto& loc : *settlement_data_ptr_)
	{
		occupied_locations.emplace(loc.galaxy, loc.system, loc.planet);
	}

	// search first empty spot in galaxy
	time_t time_now = std::time(nullptr);
	create_info info;
	std::vector<create_info> bots;
	bots.reserve(cmd.count);
	SetEmailStartNum();
	std::string password = database_->GetBotsPass();
	CryptPassword(password);
	int metal_start = database_->GetMetalStart();
	int crystal_start = database_->GetCrystalStart();
	int deu_start = database_->GetDeuStart();
	int dm_start = database_->GetDmStart();

	for (size_t i = 0; i < cmd.count; i++)
	{
		info.Reset();
		SetName(info);
		info.password = password;
		SetEmail(info);
		info.lang = "en";
		info.universe = cmd.universe;
		SetLocation(info, config_ptr, occupied_locations);
		SetImage(info);
		SetTemp(info);
		info.metal = metal_start;
		info.crystal = crystal_start;
		info.deuterium = deu_start;
		info.darkmatter = dm_start;
		info.register_time = static_cast<int>(time_now);
		info.onlinetime = static_cast<int>(time_now);
		info.is_bot = 1;
		bots.push_back(info);

		// testing,logging
		// CLogger::Info("username : {}\n" ,info.username);
		// CLogger::Info("email : {}\n", info.email);
		// CLogger::Info("gsp : {}:{}:{}\n", info.galaxy, info.system, info.planet);
		// CLogger::Info("image: {}\n", info.image);
		// CLogger::Info("temp_min: {}\n", info.temp_min);
		// CLogger::Info("temp_min: {}\n", info.temp_max);
	}

	// send vector to database to insert
	if (database_->AddBots(bots))
	{
		database_->LoadBots();
		database_->LoadSettlementData();
	}
}

void CBotManager::RemoveBots()
{
	if (database_->RemoveBots())
	{
		bots_ptr_ = database_->GetLoadedBots();
		CLogger::Info(lang_->at("ids_bot_remove_succ"));
	}
}

void CBotManager::AddMetal(const cmd_queue& cmd)
{
	if (database_ == nullptr || bots_ptr_ == nullptr) return;
	for (auto& bot: *bots_ptr_)
	{
		for (auto& p: bot.all_planets)
		{
			p.metal += cmd.count;
			p.need_update = true;
		}
	}
	database_->UpdateBots();
	database_->LoadBots();
	CLogger::Info(lang_->at("ids_add_metal_suc"), cmd.count);
}

void CBotManager::AddCrystal(const cmd_queue& cmd)
{
	if (database_ == nullptr || bots_ptr_ == nullptr) return;
	for (auto& bot : *bots_ptr_)
	{
		for (auto& p : bot.all_planets)
		{
			p.crystal += cmd.count;
			p.need_update = true;
		}
	}
	database_->UpdateBots();
	database_->LoadBots();
	CLogger::Info(lang_->at("ids_add_crystal_suc"), cmd.count);
}

void CBotManager::AddDeu(const cmd_queue& cmd)
{
	if (database_ == nullptr || bots_ptr_ == nullptr) return;
	for (auto& bot : *bots_ptr_)
	{
		for (auto& p : bot.all_planets)
		{
			p.deuterium += cmd.count;
			p.need_update = true;
		}
	}
	database_->UpdateBots();
	database_->LoadBots();
	CLogger::Info(lang_->at("ids_add_deu_suc"), cmd.count);
}

void CBotManager::AddDm(const cmd_queue& cmd)
{
	if (database_ == nullptr || bots_ptr_ == nullptr) return;
	for (auto& bot : *bots_ptr_)
	{
		bot.darkmatter += cmd.count;
		bot.need_update = true;
	}
	database_->UpdateBots();
	database_->LoadBots();
	CLogger::Info(lang_->at("ids_add_dm_suc"), cmd.count);
}

void CBotManager::SetName(create_info& st)
{
	st.username.clear();
	int rand_index = std::rand() % bot_title.size();
	const std::string title = bot_title[rand_index];
	rand_index = std::rand() % bot_name.size();
	const std::string name = bot_name[rand_index];
	st.username.reserve(title.size() + name.size() + 1);

	st.username.append(title);
	st.username.push_back(' ');
	st.username.append(name);
}

void CBotManager::SetLocation(create_info& st,
	const config_data* config,
	std::set<std::tuple<int, int, int>>& occupied_locations)
{
	if (!config
		|| !settlement_data_ptr_)
	{
		return;
	}

	for (int galaxy = 1; galaxy <= config->max_galaxy; galaxy++)
	{
		for (int system = 1; system <= config->max_system; system++)
		{
			for (int planet = 1; planet <= config->max_planet; planet++)
			{
				if (occupied_locations.count({galaxy, system, planet}) == 0)
				{
					st.galaxy = galaxy;
					st.system = system;
					st.planet = planet;
					occupied_locations.emplace(galaxy, system, planet);
					return;
				}
			}
		}
	}
}

void CBotManager::CryptPassword(std::string& pass)
{
	pass = bcrypt::generateHash(pass, 13);
}

void CBotManager::SetEmail(create_info& st)
{
	st.email = "bot" + std::to_string(bot_max_email_num_) + "@2moons.de";
	st.email_2 = st.email;
	bot_max_email_num_++;
}

void CBotManager::SetImage(create_info& st)
{
	switch (st.planet)
	{
		case 1:
		case 2:
		case 3:
			if (std::rand() % 2 == 0)
			{
				st.image = "trocken";
				st.image += "planet";
				st.image += std::format("{:02}", (std::rand() % 10 + 1));
			}
			else
			{
				st.image = "wuesten";
				st.image += "planet";
				st.image += std::format("{:02}", (std::rand() % 4 + 1));
			}
			break;
		case 4:
		case 5:
		case 6:
			st.image = "dschjungel";
			st.image += "planet";
			st.image += std::format("{:02}", (std::rand() % 10 + 1));
			break;
		case 7:
		case 8:
			st.image = "normaltemp";
			st.image += "planet";
			st.image += std::format("{:02}", (std::rand() % 7 + 1));
			break;
		case 9:
		case 10:
		case 11:
		case 12:
			if (std::rand() % 2 == 0)
			{
				st.image = "normaltemp";
				st.image += "planet";
				st.image += std::format("{:02}", (std::rand() % 7 + 1));
			}
			else
			{
				st.image = "wasser";
				st.image += "planet";
				st.image += std::format("{:02}", (std::rand() % 9 + 1));
			}
			break;
		case 13:
		case 14:
		case 15:
			st.image = "eis";
			st.image += "planet";
			st.image += std::format("{:02}", (std::rand() % 10 + 1));
			break;
		default:
			st.image = "normaltemp";
			st.image += "planet";
			st.image += std::format("{:02}", (std::rand() % 7 + 1));
			break;
	}
}

void CBotManager::SetTemp(create_info& st)
{
	int temp = 0;
	switch (st.planet)
	{
		case 1:
			temp = rand() % 41 + 220;
			break;
		case 2:
			temp = rand() % 41 + 170;
			break;
		case 3:
			temp = rand() % 41 + 120;
			break;
		case 4:
			temp = rand() % 41 + 70;
			break;
		case 5:
			temp = rand() % 41 + 60;
			break;
		case 6:
			temp = rand() % 41 + 50;
			break;
		case 7:
			temp = rand() % 41 + 40;
			break;
		case 8:
			temp = rand() % 41 + 30;
			break;
		case 9:
			temp = rand() % 41 + 20;
			break;
		case 10:
			temp = rand() % 41 + 10;
			break;
		case 11:
			temp = rand() % 41 + 0;
			break;
		case 12:
			temp = rand() % 41 + (-10);
			break;
		case 13:
			temp = -(rand() % 41) + (-10);
			break;
		case 14:
			temp = -(rand() % 41) + (-50);
			break;
		case 15:
			temp = -(rand() % 41) + (-90);
			break;
		default:
			temp = rand() % 41 + 30;
			break;
	}
	st.temp_max = temp;
	st.temp_min = temp - 40;
}

void CBotManager::SetEmailStartNum()
{
	int max_num = 0;

	for (const auto& bot : *bots_ptr_)
	{
		const std::string& email = bot.email;

		if (email.rfind("bot", 0) != 0)
			continue;

		const size_t at_pos = email.find('@');
		if (at_pos == std::string::npos)
			continue;

		const int num = std::stoi(email.substr(3, at_pos - 3));

		if (num > max_num)
			max_num = num;
	}

	bot_max_email_num_ = max_num + 1;
}