#include "AI/AIState.h"
#include "AI/CreatureData.h"
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
    if (!m_context->isAlive)
    {
        SetNextState<DeadState>(m_context);
        return;
    }

    m_context->respirationTimer += dt;

    if (m_context->respirationTimer >= m_context->data->respirationTime)
    {
        m_context->respirationTimer = 0.0f;
        m_context->data->health = std::min(m_context->data->health + 5.0f, m_context->data->maxHealth);
    }

    if (PlayerInRange(m_context, m_context->data->detectionRange))
    {
        if (m_context->data->flees)
            SetNextState<FleeState>(m_context);

        else
            SetNextState<ChaseState>(m_context);

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
    if (!m_context->isAlive)
    {
        SetNextState<DeadState>(m_context);
        return;
    }

    if (PlayerInRange(m_context, m_context->data->detectionRange))
    {
        if (m_context->data->flees)
            SetNextState<FleeState>(m_context);

        else
            SetNextState<ChaseState>(m_context);

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

// ChaseState

ChaseState::ChaseState(AIContext* context) : m_context(context) {}

void ChaseState::OnEnter() {}
void ChaseState::OnExit() {}

void ChaseState::Update(float dt)
{
    if (!m_context->isAlive)
    {
        SetNextState<DeadState>(m_context);
        return;
    }

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

    if (dist <= m_context->data->attackRange)
    {
        SetNextState<AttackState>(m_context);
        return;
    }

    MoveToward(m_context, m_context->playerTransform->GetPosition(), dt);
}

void ChaseState::Input() {}
void ChaseState::Render() {}

// AttackState

AttackState::AttackState(AIContext* context) : m_context(context) {}

void AttackState::OnEnter()
{
    m_elapsed = 0.0f;
    m_attackCooldown = m_context->data->infusionTime * 0.1f;

    if (m_attackCooldown < 0.5f)
        m_attackCooldown = 0.5f;
}

void AttackState::OnExit() {}

void AttackState::Update(float dt)
{
    if (!m_context->isAlive)
    {
        SetNextState<DeadState>(m_context);
        return;
    }

    if (!m_context->playerTransform)
    {
        SetNextState<IdleState>(m_context);
        return;
    }

    float dist = DistanceTo(*m_context->transform, *m_context->playerTransform);

    if (dist > m_context->data->attackRange * 1.2f)
    {
        SetNextState<ChaseState>(m_context);
        return;
    }

    m_context->transform->LookAt(m_context->playerTransform->GetPosition());

    m_elapsed += dt;

    if (m_elapsed >= m_attackCooldown)
    {
        //Inflict damage to player Health
		m_context->player->TakeDamage(15);
        m_elapsed = 0.0f;
    }

}

void AttackState::Input() {}
void AttackState::Render() {}

// FleeState

FleeState::FleeState(AIContext* context) : m_context(context) {}

void FleeState::OnEnter() {}
void FleeState::OnExit() {}

void FleeState::Update(float dt)
{
    if (!m_context->isAlive)
    {
        SetNextState<DeadState>(m_context);
        return;
    }

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

// DeadState

DeadState::DeadState(AIContext* context) : m_context(context) {}

void DeadState::OnEnter()
{
    m_context->isAlive = false;
}

void DeadState::OnExit() {}
void DeadState::Update(float) {}
void DeadState::Input() {}
void DeadState::Render() {}
