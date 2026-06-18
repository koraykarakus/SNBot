#pragma once
#include <array>

inline constexpr std::array bot_title = {
	"Marshal", "Czar", "Governor", "Technocrat", "Geologist", "Commander",
	"Lord", "Commodore", "Chancellor", "Emperor", "Mogul", "Sovereign", "Proconsul",
	"Stadtholder", "Renegade", "Lieutenant", "Admiral", "Vice", "Consul", "Chief",
	"President", "Procurator", "Engineer", "Constable", "Bandit", "Senator", "Viceregent",
	"Captain", "Director", "Kualla", "Padme"};

inline constexpr std::array bot_name = {
	"Yakini", "Astra", "Cosmos", "Skat", "Nemesis", "Mars", "Icarus", "Helix", "Cetus",
	"Hydra", "Genesis", "Octans", "Remus", "Sigma", "Pavo", "Navi", "Rocket", "Erdemas",
	"Europa", "Ceres", "Ferret", "Cupid", "Sirius", "Antimatter", "Centauri", "Midas",
	"Quantum", "Dorado", "Deimos", "Keid", "Andromeda", "Apollo",
	"Saturn", "Spica", "Majoris", "Vega", "Pathfinder", "Kuma", "Cosmo",
	"Gravity", "Uranus", "Ares", "Janus", "Transit", "Uriel",
	"Scorpius", "Omicron", "Sol", "Mimas", "Euler", "Castor",
	"Probe", "Neso", "Retina", "Io", "Leda", "Ceti", "Moon", "Herschel",
	"Varilla", "Tarvos", "Pollux", "Sunspot", "Mariner", "Zuben", "Nestor",
	"Grus", "Themis", "Klio", "Puck", "Japetus", "Scout", "Solar", "Kale", "Lambda",
	"Leto", "Amidala", "Zagadra", "Seti", "Tycho", "Sputnik", "Navi", "Starburst",
	"Comet", "Sagan", "Atik", "Gamma", "Dorado", "Jones", "Lepus", "Taurus", "Owl",
	"Zenith", "Auriga", "Jericho", "Mimas", "Voyager", "Spirit", "Explorer", "Palma",
	"Gliese", "Cassini", "Pan", "Neptune", "Discory", "Polaris", "Barym", "Spacewalk",
	"Ganimed", "Forma", "Pulsar", "Holmes", "Rhea", "Deneb",
	"Nova", "Omega", "Zagadra", "Hunter", "Ranger", "Zibal", "Asteroid"};

struct create_info
{
	std::string username = "";
	std::string password = "";
	std::string email = "";
	std::string email_2 = "";
	std::string lang = "en";
	uint8_t universe = 0;
	uint8_t galaxy = 0;
	uint16_t system = 0;
	uint8_t planet = 0;
	int darkmatter = 0;
	int register_time = 0;
	int onlinetime = 0;
	uint8_t is_bot = 1;

	// planet related..
	std::string planet_name = "Main Planet";
	int last_update = 0;
	uint8_t planet_type = 1;
	std::string image = "normaltempplanet01";
	uint16_t field_max = 163;
	int temp_min = -17;
	int temp_max = 23;
	int metal = 10000;
	int crystal = 10000;
	int deuterium = 0;

	void Reset()
	{
		*this = create_info();
	}
};