#pragma once

#include "Core/StateMachine.h"

#include <glm/glm.hpp>

#include <vector>
#include <PlayerComponent.h>

struct CreatureData;
class TransformComponent;

struct AIContext
{
    CreatureData* data = nullptr;
    TransformComponent* transform = nullptr;
    TransformComponent* playerTransform = nullptr;
    std::vector<glm::vec3> patrolPoints;
    int currentPatrolIndex = 0;
    float waitTimer = 0.0f;
    float respirationTimer = 0.0f;
    bool isAlive = true;
	PlayerComponent* player = nullptr;
};

class IdleState : public IState
{
public:
    explicit IdleState(AIContext* context);
    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Input() override;
    void Render() override;

private:
    AIContext* m_context;
    float m_idleDuration = 0.0f;
    float m_elapsed = 0.0f;
};

class PatrolState : public IState
{
public:
    explicit PatrolState(AIContext* context);
    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Input() override;
    void Render() override;

private:
    AIContext* m_context;
};

class FleeState : public IState
{
public:
    explicit FleeState(AIContext* context);
    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Input() override;
    void Render() override;

private:
    AIContext* m_context;
};
