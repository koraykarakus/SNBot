#pragma once
#include <string>

struct GameVarItem {
    int elementID;
    std::string name;
    double costMetal;     // cost901
    double costCrystal;   // cost902
    double costDeuterium; // cost903
    double factor;
};
