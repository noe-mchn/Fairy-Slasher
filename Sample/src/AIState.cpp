#include "AIState.h"
#include "CreatureData.h"
#include "Core/TrasformComponent.h"

#include <algorithm>

static float DistanceTo(const TransformComponent& a, const TransformComponent& b)
{
    return glm::length(a.GetPosition() - b.GetPosition());
}

static bool PlayerInRange(const AIContext* context, float range)
{
    if (!context->playerTransform)
        return false;

    return DistanceTo(*context->transform, *context->playerTransform) <= range;
}

static void MoveToward(AIContext* context, const glm::vec3& target, float dt)
{
    glm::vec3 pos = context->transform->GetPosition();
    glm::vec3 dir = target - pos;
    float dist = glm::length(dir);

    if (dist < 0.01f)
        return;
    dir = glm::normalize(dir);
    float step = context->data->moveSpeed * dt;

    if (step > dist)
        step = dist;

    context->transform->SetPosition(pos + dir * step);
    context->transform->LookAt(target);
}

// IdleState

IdleState::IdleState(AIContext* context) : m_context(context) {}

void IdleState::OnEnter()
{
    m_elapsed = 0.0f;
    m_idleDuration = 1.5f + static_cast<float>(m_context->data->respirationTime) * 0.1f;
}

void IdleState::OnExit() {}

void IdleState::Update(float dt)
{
    if (m_context->data->flees && PlayerInRange(m_context, m_context->data->detectionRange))
    {
        SetNextState<FleeState>(m_context);
        return;
    }

    m_elapsed += dt;

    if (m_elapsed >= m_idleDuration && !m_context->patrolPoints.empty())
    {
        SetNextState<PatrolState>(m_context);
    }
}

void IdleState::Input() {}
void IdleState::Render() {}

// PatrolState

PatrolState::PatrolState(AIContext* context) : m_context(context) {}

void PatrolState::OnEnter() {}
void PatrolState::OnExit() {}

void PatrolState::Update(float dt)
{
    if (m_context->data->flees && PlayerInRange(m_context, m_context->data->detectionRange))
    {
        SetNextState<FleeState>(m_context);
        return;
    }

    if (m_context->patrolPoints.empty())
    {
        SetNextState<IdleState>(m_context);
        return;
    }

    const glm::vec3& target = m_context->patrolPoints[m_context->currentPatrolIndex];
    MoveToward(m_context, target, dt);

    float dist = glm::length(m_context->transform->GetPosition() - target);

    if (dist < 0.3f)
    {
        m_context->currentPatrolIndex = (m_context->currentPatrolIndex + 1) % static_cast<int>(m_context->patrolPoints.size());
        SetNextState<IdleState>(m_context);
    }
}

void PatrolState::Input() {}
void PatrolState::Render() {}

// FleeState

FleeState::FleeState(AIContext* context) : m_context(context) {}

void FleeState::OnEnter() {}
void FleeState::OnExit() {}

void FleeState::Update(float dt)
{
    if (!m_context->playerTransform)
    {
        SetNextState<IdleState>(m_context);
        return;
    }

    float dist = DistanceTo(*m_context->transform, *m_context->playerTransform);

    if (dist > m_context->data->detectionRange * 1.5f)
    {
        SetNextState<IdleState>(m_context);
        return;
    }

    glm::vec3 pos = m_context->transform->GetPosition();
    glm::vec3 playerPos = m_context->playerTransform->GetPosition();
    glm::vec3 fleeDir = pos - playerPos;

    float fleeDist = glm::length(fleeDir);

    if (fleeDist > 0.01f)
    {
        fleeDir = glm::normalize(fleeDir);
        float step = m_context->data->moveSpeed * dt;
        m_context->transform->SetPosition(pos + fleeDir * step);
    }
}

void FleeState::Input() {}
void FleeState::Render() {}
