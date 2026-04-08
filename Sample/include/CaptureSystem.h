#pragma once

#include "CreatureAIComponent.h"
#include "InventoryComponent.h"
#include "Core/TrasformComponent.h"
#include "ECS/Registry.h"

#include <vector>
#include <glm/glm.hpp>

/**
 * @brief System that handles capturing creatures.
 *
 * When the player is close enough to a creature and holds the capture
 * action long enough, the creature's data is moved into the player's
 * InventoryComponent and the creature entity is destroyed.
 */
struct CaptureSystem
{
    /**
     * @brief Finds the nearest creature within capture range.
     *
     * @tparam RegistryType  Concrete ECS registry type.
     * @param registry       The registry that owns all entities.
     * @param playerEntity   Entity that carries the InventoryComponent.
     * @param captureRange   Maximum distance (XZ) for a capture to succeed.
     * @param outEntity      Receives the closest creature entity if found.
     * @return true if a creature is in range.
     */
    template<typename RegistryType>
    static bool FindNearest(RegistryType& registry,
                            typename RegistryType::type playerEntity,
                            float captureRange,
                            typename RegistryType::type& outEntity)
    {
        if (!registry.template HasComponent<TransformComponent>(playerEntity))
            return false;

        auto& playerTrans  = registry.template GetComponent<TransformComponent>(playerEntity);
        glm::vec3 playerPos = playerTrans.GetPosition();

        using EntityType = typename RegistryType::type;
        float bestDist  = captureRange + 1.0f;
        bool  found     = false;

        auto creatures = registry.template GetAllComponentsView<CreatureAIComponent, TransformComponent>();
        for (auto& e : creatures)
        {
            auto& tr   = registry.template GetComponent<TransformComponent>(e);
            float dist = glm::length(tr.GetPosition() - playerPos);

            if (dist <= captureRange && dist < bestDist)
            {
                bestDist  = dist;
                outEntity = e;
                found     = true;
            }
        }

        return found;
    }

    /**
     * @brief Immediately captures the given creature entity.
     *
     * Moves the creature's data into the player's inventory and
     * destroys the creature entity.
     *
     * @tparam RegistryType  Concrete ECS registry type.
     * @param registry       The registry that owns all entities.
     * @param playerEntity   Entity that carries the InventoryComponent.
     * @param creatureEntity The creature to capture.
     * @return true if the creature was captured.
     */
    template<typename RegistryType>
    static bool Capture(RegistryType& registry,
                        typename RegistryType::type playerEntity,
                        typename RegistryType::type creatureEntity)
    {
        if (!registry.template HasComponent<InventoryComponent>(playerEntity))
            return false;

        auto& inventory = registry.template GetComponent<InventoryComponent>(playerEntity);
        if (inventory.IsFull())
            return false;

        if (!registry.template HasComponent<CreatureAIComponent>(creatureEntity))
            return false;

        auto& ai = registry.template GetComponent<CreatureAIComponent>(creatureEntity);
        inventory.AddCreature(ai.data);
        registry.DestroyEntity(creatureEntity);
        return true;
    }
};
