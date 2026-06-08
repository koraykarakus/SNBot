#pragma once

#include <map>
#include <vector>
#include <string>

// Struct yapıların kalıyor...
struct CombatCaps { double attack; double shield; };
struct BonusData { double value; int unit; };
struct PriceListData {
	std::map<int, double> cost;
	double factor;
	int max;
	double consumption;
	double consumption2;
	double speed;
	double speed2;
	double capacity;
	int tech;
	double time;
	std::map<std::string, BonusData> bonus;
};
struct ProdGridData {
	std::map<int, std::string> production;
	std::map<int, std::string> storage;
};
struct ResListData {
	std::vector<int> prod;
	std::vector<int> storage;
	std::vector<int> bonus;
	std::vector<int> one;
	std::vector<int> build;
	std::vector<int> tech;
	std::vector<int> fleet;
	std::vector<int> defense;
	std::vector<int> missile;
	std::vector<int> officers;
	std::vector<int> dmfunc;
	std::map<int, std::vector<int>> allow;
	std::vector<int> ressources;
	std::map<int, std::vector<int>> resstype;
};

extern std::map<int, std::string> G_RESOURCE;
extern std::map<int, CombatCaps> G_COMBATCAPS;
extern std::map<int, PriceListData> G_PRICELIST;
extern std::map<int, ProdGridData> G_PRODGRID;
extern ResListData G_RESLIST;