#pragma once

#include "CreatureData.h"

#include <array>
#include <optional>
#include <cstddef>

struct CapturedCreature
{
    CreatureData data;
};

struct InventoryComponent
{
    static constexpr size_t MAX_SLOTS = 4;
    std::array<std::optional<CapturedCreature>, MAX_SLOTS> slots{};
    int selectedSlot = 0;

    bool IsSlotEmpty(int index) const
    {
        return index >= 0 && index < static_cast<int>(MAX_SLOTS) && !slots[index].has_value();
    }

    bool IsFull() const
    {
        for (auto& s : slots)
            if (!s.has_value()) return false;
        return true;
    }

    bool AddCreature(const CreatureData& creatureData)
    {
        for (size_t i = 0; i < MAX_SLOTS; ++i)
        {
            if (!slots[i].has_value())
            {
                slots[i] = CapturedCreature{ creatureData };
                return true;
            }
        }
        return false;
    }

    std::optional<CapturedCreature> RemoveFromSlot(int index)
    {
        if (index < 0 || index >= static_cast<int>(MAX_SLOTS) || !slots[index].has_value())
            return std::nullopt;
        auto creature = std::move(slots[index]);
        slots[index].reset();
        return creature;
    }

    const CapturedCreature* GetSelected() const
    {
        if (selectedSlot >= 0 && selectedSlot < static_cast<int>(MAX_SLOTS) && slots[selectedSlot].has_value())
            return &slots[selectedSlot].value();
        return nullptr;
    }

    size_t GetCount() const
    {
        size_t n = 0;
        for (auto& s : slots)
            if (s.has_value()) ++n;
        return n;
    }
};
