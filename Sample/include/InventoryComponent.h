#pragma once

#include "CreatureData.h"

#include <vector>
#include <cstddef>

/**
 * @brief A captured creature stored in the player's inventory.
 */
struct CapturedCreature
{
    CreatureData data;
};

/**
 * @brief ECS component that gives an entity a creature-capture inventory.
 *
 * Attach this to the player (or any entity that should carry creatures).
 * Limited to 4 slots shared between creatures and loot.
 */
struct InventoryComponent
{
    std::vector<CapturedCreature> captured;
    size_t maxCapacity = 4;

    bool IsFull() const { return captured.size() >= maxCapacity; }

    bool AddCreature(const CreatureData& creatureData)
    {
        if (IsFull())
            return false;
        captured.push_back({ creatureData });
        return true;
    }

    bool RemoveCreature(size_t index)
    {
        if (index >= captured.size())
            return false;
        captured.erase(captured.begin() + static_cast<ptrdiff_t>(index));
        return true;
    }

    /**
     * @brief Releases the most recently captured creature, freeing a slot.
     * @return true if a creature was released.
     */
    bool ReleaseLast()
    {
        if (captured.empty())
            return false;
        captured.pop_back();
        return true;
    }

    size_t GetCount() const { return captured.size(); }
};
