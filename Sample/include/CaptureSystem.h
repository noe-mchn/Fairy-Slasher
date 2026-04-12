#pragma once

#include "CreatureAIComponent.h"
#include "InventoryComponent.h"
#include "Core/TrasformComponent.h"
#include "ECS/Registry.h"

#include <vector>
#include <glm/glm.hpp>

struct CaptureSystem
{
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
