#pragma once

#include "CreatureData.h"

#include <vector>
#include <cstddef>

struct CauldronSlot
{
    CreatureData data;
    float elapsed  = 0.0f;
    bool  finished = false;
};

struct CauldronComponent
{
    std::vector<CauldronSlot> slots;
    size_t maxSlots         = 1;
    float  interactionRange = 3.0f;

    bool HasFreeSlot() const { return slots.size() < maxSlots; }

    bool AddCreature(const CreatureData& creatureData)
    {
        if (!HasFreeSlot())
            return false;
        slots.push_back({ creatureData, 0.0f, false });
        return true;
    }

    size_t FinishedCount() const
    {
        size_t n = 0;
        for (const auto& s : slots)
            if (s.finished)
                ++n;
        return n;
    }
};
