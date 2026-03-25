#pragma once

#include "AI/CreatureAIComponent.h"
#include "Core/TrasformComponent.h"
#include "ECS/Registry.h"

struct AISystem
{
    template<typename RegistryType>
    static void Update(RegistryType& registry, float dt)
    {
        auto es = registry.template GetAllComponentsView<CreatureAIComponent, TransformComponent>();
        for (auto& e : es)
        {
            auto& ai = registry.template GetComponent<CreatureAIComponent>(e);

            if (!ai.initialized || !ai.stateMachine || !ai.context.isAlive)
                continue;

            ai.context.data      = &ai.data;
            ai.context.transform = &registry.template GetComponent<TransformComponent>(e);

            ai.stateMachine->Update(dt);
            ai.stateMachine->ChangeState();
        }
    }
};
