#pragma once

#include "CreatureData.h"

#include <vector>
#include <cstddef>

/**
 * @brief One creature being infused inside the cauldron.
 */
struct CauldronSlot
{
    CreatureData data;
    float elapsed  = 0.0f;  ///< Seconds spent infusing so far.
    bool  finished = false;  ///< True once elapsed >= data.infusionTime.
};

/**
 * @brief ECS component that turns an entity into a cauldron.
 *
 * The cauldron accepts captured creatures, infuses them over time, and
 * signals when loot is ready to be collected.
 */
struct CauldronComponent
{
    std::vector<CauldronSlot> slots;
    size_t maxSlots         = 1;     ///< How many creatures can infuse at once.
    float  interactionRange = 3.0f;  ///< Max XZ distance for the player to interact.

    bool HasFreeSlot() const { return slots.size() < maxSlots; }

    /**
     * @brief Drops a creature into the cauldron for infusion.
     * @return true if accepted, false if the cauldron is full.
     */
    bool AddCreature(const CreatureData& creatureData)
    {
        if (!HasFreeSlot())
            return false;
        slots.push_back({ creatureData, 0.0f, false });
        return true;
    }

    /**
     * @brief Returns the number of slots whose infusion is complete.
     */
    size_t FinishedCount() const
    {
        size_t n = 0;
        for (const auto& s : slots)
            if (s.finished)
                ++n;
        return n;
    }
};
