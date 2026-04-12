#pragma once

#include <vector>
#include <string>
#include <algorithm>

struct LootEntry
{
    std::string name;
    int quantity = 0;
};

struct LootInventoryComponent
{
    std::vector<LootEntry> items;

    void AddLoot(const std::string& name, int quantity = 1)
    {
        auto it = std::find_if(items.begin(), items.end(),
            [&](const LootEntry& e) { return e.name == name; });

        if (it != items.end())
            it->quantity += quantity;
        else
            items.push_back({ name, quantity });
    }

    int GetQuantity(const std::string& name) const
    {
        auto it = std::find_if(items.begin(), items.end(),
            [&](const LootEntry& e) { return e.name == name; });
        return (it != items.end()) ? it->quantity : 0;
    }
};
