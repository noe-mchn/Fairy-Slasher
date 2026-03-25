#pragma once

#include "Core/StateMachine.h"

#include <glm/glm.hpp>

#include <vector>

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

class ChaseState : public IState
{
public:
    explicit ChaseState(AIContext* context);
    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Input() override;
    void Render() override;

private:
    AIContext* m_context;
};

class AttackState : public IState
{
public:
    explicit AttackState(AIContext* context);
    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Input() override;
    void Render() override;

private:
    AIContext* m_context;
    float m_attackCooldown = 0.0f;
    float m_elapsed = 0.0f;
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

class DeadState : public IState
{
public:
    explicit DeadState(AIContext* context);
    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Input() override;
    void Render() override;

private:
    AIContext* m_context;
};
