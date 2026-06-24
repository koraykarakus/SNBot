#pragma once
#include "globals.h"
#include <unordered_map>
#include <chrono>
#include <string>

using time_var = std::chrono::steady_clock::time_point;
using vars_umap = std::unordered_map<int, vars_data>;
using vars_requirements_umap = std::unordered_map<int, std::vector<vars_requirements_data>>;
using resource_umap = std::unordered_map<int, std::string>;
using combatcaps_umap = std::unordered_map<int, combat_caps_data>;
using pricelist_umap = std::unordered_map<int, pricelist_data>;
using prodgrid_umap = std::unordered_map<int, prodgrid_data>;
using config_umap = std::unordered_map<int, config_data>;