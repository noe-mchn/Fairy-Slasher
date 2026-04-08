#pragma once

#include "LootInventoryComponent.h"

#include <string>
#include <algorithm>

/**
 * @brief ECS component tracking unlocked player upgrades.
 *
 * Upgrades are purchased by consuming loot from a LootInventoryComponent.
 *   - Night Vision : costs 1 "Rare Souls Essences"
 *   - Double Jump  : costs 1 "Fairy Wings"
 */
struct PlayerUpgradeComponent
{
    bool nightVision = false;
    bool doubleJump  = false;

    /**
     * @brief Try to unlock night vision by consuming the required loot.
     * @return true if successfully unlocked.
     */
    static bool TryUnlockNightVision(PlayerUpgradeComponent& upgrades,
                                     LootInventoryComponent& lootInv)
    {
        if (upgrades.nightVision)
            return false;

        const std::string required = "Rare Souls Essences";
        if (lootInv.GetQuantity(required) < 1)
            return false;

        ConsumeLoot(lootInv, required, 1);
        upgrades.nightVision = true;
        return true;
    }

    /**
     * @brief Try to unlock double jump by consuming the required loot.
     * @return true if successfully unlocked.
     */
    static bool TryUnlockDoubleJump(PlayerUpgradeComponent& upgrades,
                                    LootInventoryComponent& lootInv)
    {
        if (upgrades.doubleJump)
            return false;

        const std::string required = "Fairy Wings";
        if (lootInv.GetQuantity(required) < 1)
            return false;

        ConsumeLoot(lootInv, required, 1);
        upgrades.doubleJump = true;
        return true;
    }

private:
    static void ConsumeLoot(LootInventoryComponent& lootInv,
                            const std::string& name, int amount)
    {
        auto it = std::find_if(lootInv.items.begin(), lootInv.items.end(),
            [&](const LootEntry& e) { return e.name == name; });
        if (it != lootInv.items.end())
        {
            it->quantity -= amount;
            if (it->quantity <= 0)
                lootInv.items.erase(it);
        }
    }
};
