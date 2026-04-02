#pragma once

#include "CauldronComponent.h"
#include "InventoryComponent.h"
#include "LootInventoryComponent.h"
#include "Core/TrasformComponent.h"
#include "ECS/Registry.h"

#include <cstdlib>
#include <algorithm>
#include <glm/glm.hpp>

/**
 * @brief System that drives the cauldron infusion loop.
 *
 * Each frame:
 *   1. Tick infusion timers on every cauldron.
 *   2. Optionally, let the player deposit a creature (DepositCreature).
 *   3. Optionally, let the player collect finished loot (CollectLoot).
 */
struct CauldronSystem
{
    // -----------------------------------------------------------------
    // Per-frame update — tick every cauldron timer
    // -----------------------------------------------------------------

    template<typename RegistryType>
    static void Update(RegistryType& registry, float dt)
    {
        auto cauldrons = registry.template GetAllComponentsView<CauldronComponent>();
        for (auto& e : cauldrons)
        {
            auto& cauldron = registry.template GetComponent<CauldronComponent>(e);

            for (auto& slot : cauldron.slots)
            {
                if (slot.finished)
                    continue;

                slot.elapsed += dt;

                if (slot.elapsed >= slot.data.infusionTime)
                    slot.finished = true;
            }
        }
    }

    // -----------------------------------------------------------------
    // Deposit — move a captured creature from inventory into cauldron
    // -----------------------------------------------------------------

    /**
     * @brief Deposits the first captured creature from the player's
     *        inventory into the nearest cauldron with a free slot.
     *
     * @return true if a creature was deposited.
     */
    template<typename RegistryType>
    static bool DepositCreature(RegistryType& registry,
                                typename RegistryType::type playerEntity)
    {
        if (!registry.template HasComponent<InventoryComponent>(playerEntity))
            return false;

        auto& inventory   = registry.template GetComponent<InventoryComponent>(playerEntity);
        auto& playerTrans = registry.template GetComponent<TransformComponent>(playerEntity);
        glm::vec3 playerPos = playerTrans.GetPosition();

        if (inventory.captured.empty())
            return false;

        // Find the nearest cauldron in interaction range with a free slot.
        auto cauldrons = registry.template GetAllComponentsView<CauldronComponent, TransformComponent>();

        using EntityType = typename RegistryType::type;
        EntityType bestCauldron{};
        float bestDist = std::numeric_limits<float>::max();
        bool found     = false;

        for (auto& e : cauldrons)
        {
            auto& c  = registry.template GetComponent<CauldronComponent>(e);
            auto& tr = registry.template GetComponent<TransformComponent>(e);

            if (!c.HasFreeSlot())
                continue;

            float dist = glm::length(tr.GetPosition() - playerPos);

            if (dist <= c.interactionRange && dist < bestDist)
            {
                bestDist     = dist;
                bestCauldron = e;
                found        = true;
            }
        }

        if (!found)
            return false;

        auto& cauldron = registry.template GetComponent<CauldronComponent>(bestCauldron);
        cauldron.AddCreature(inventory.captured.front().data);
        inventory.RemoveCreature(0);
        return true;
    }

    // -----------------------------------------------------------------
    // Collect — harvest finished loot from the nearest cauldron
    // -----------------------------------------------------------------

    /**
     * @brief Collects all finished loot from the nearest cauldron and
     *        stores it in the player's LootInventoryComponent.
     *
     * Loot is generated using the creature's @c lootChance: a random
     * roll decides whether the loot is obtained.
     *
     * @return The number of loot items successfully collected.
     */
    template<typename RegistryType>
    static int CollectLoot(RegistryType& registry,
                           typename RegistryType::type playerEntity)
    {
        if (!registry.template HasComponent<LootInventoryComponent>(playerEntity))
            return 0;

        auto& lootInv     = registry.template GetComponent<LootInventoryComponent>(playerEntity);
        auto& playerTrans = registry.template GetComponent<TransformComponent>(playerEntity);
        glm::vec3 playerPos = playerTrans.GetPosition();

        auto cauldrons = registry.template GetAllComponentsView<CauldronComponent, TransformComponent>();

        using EntityType = typename RegistryType::type;
        EntityType bestCauldron{};
        float bestDist = std::numeric_limits<float>::max();
        bool found     = false;

        for (auto& e : cauldrons)
        {
            auto& c  = registry.template GetComponent<CauldronComponent>(e);
            auto& tr = registry.template GetComponent<TransformComponent>(e);

            if (c.FinishedCount() == 0)
                continue;

            float dist = glm::length(tr.GetPosition() - playerPos);

            if (dist <= c.interactionRange && dist < bestDist)
            {
                bestDist     = dist;
                bestCauldron = e;
                found        = true;
            }
        }

        if (!found)
            return 0;

        auto& cauldron = registry.template GetComponent<CauldronComponent>(bestCauldron);

        int collected = 0;

        // Iterate in reverse so erasing doesn't invalidate indices.
        for (int i = static_cast<int>(cauldron.slots.size()) - 1; i >= 0; --i)
        {
            auto& slot = cauldron.slots[static_cast<size_t>(i)];
            if (!slot.finished)
                continue;

            // Roll for loot.
            int roll = std::rand() % 100;
            if (roll < slot.data.lootChance)
            {
                lootInv.AddLoot(slot.data.lootName);
                ++collected;
            }

            cauldron.slots.erase(cauldron.slots.begin() + i);
        }

        return collected;
    }
};
