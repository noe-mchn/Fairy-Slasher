#pragma once

#include "CreatureData.h"
#include "AIState.h"
#include "Core/StateMachine.h"

#include <memory>
#include <vector>
#include <glm/glm.hpp>

struct CreatureAIComponent
{
    CreatureData data;
    AIContext context;
    std::unique_ptr<StateMachine> stateMachine;
    bool initialized = false;

    void Init(TransformComponent* selfTransform, TransformComponent* playerTransform, std::vector<glm::vec3> patrolPts = {})
    {
        context.data = &data;
        context.transform = selfTransform;
        context.playerTransform = playerTransform;
        context.patrolPoints = std::move(patrolPts);
        context.currentPatrolIndex = 0;
        context.waitTimer = 0.0f;

        auto initialState = std::make_unique<IdleState>(&context);
        stateMachine = std::make_unique<StateMachine>(std::move(initialState), 3);
        initialized = true;
    }
};
