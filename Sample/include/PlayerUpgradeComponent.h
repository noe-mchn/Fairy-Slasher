#pragma once

#include "LootInventoryComponent.h"

#include <string>
#include <algorithm>

struct PlayerUpgradeComponent
{
    bool captureBoost = false;
    bool doubleJump   = false;

    static bool TryUnlockCaptureBoost(PlayerUpgradeComponent& upgrades,
                                      LootInventoryComponent& lootInv)
    {
        if (upgrades.captureBoost)
            return false;

        const std::string required = "Rare Fairy Wings";
        if (lootInv.GetQuantity(required) < 1)
            return false;

        ConsumeLoot(lootInv, required, 1);
        upgrades.captureBoost = true;
        return true;
    }

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
