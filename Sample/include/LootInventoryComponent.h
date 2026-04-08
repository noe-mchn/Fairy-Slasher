#pragma once

#include <vector>
#include <string>
#include <algorithm>

/**
 * @brief A single loot item with a name and stacked quantity.
 */
struct LootEntry
{
    std::string name;
    int quantity = 0;
};

/**
 * @brief ECS component that stores collected loot (potions, essences, etc.).
 *
 * Attach this to the player or any entity that should accumulate crafted items.
 */
struct LootInventoryComponent
{
    std::vector<LootEntry> items;

    /**
     * @brief Adds @p quantity units of loot named @p name.
     *
     * If the loot already exists in the list its quantity is increased;
     * otherwise a new entry is created.
     */
    void AddLoot(const std::string& name, int quantity = 1)
    {
        auto it = std::find_if(items.begin(), items.end(),
            [&](const LootEntry& e) { return e.name == name; });

        if (it != items.end())
            it->quantity += quantity;
        else
            items.push_back({ name, quantity });
    }

    /**
     * @brief Returns the total quantity of a given loot item.
     */
    int GetQuantity(const std::string& name) const
    {
        auto it = std::find_if(items.begin(), items.end(),
            [&](const LootEntry& e) { return e.name == name; });
        return (it != items.end()) ? it->quantity : 0;
    }
};
