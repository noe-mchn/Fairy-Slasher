#pragma once

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <algorithm>

// Enemy type identifiers
#define SOULS    0
#define FAIRY    1
#define CREATURE 2

struct EnemyComponent
{
	enum class Type : uint8_t
	{
		Souls    = SOULS,
		Fairy    = FAIRY,
		Creature = CREATURE
	};

	Type        type            = Type::Souls;
	bool        alive           = true;

	// --- stats ---
	float       health          = 100.0f;
	float       maxHealth       = 100.0f;
	float       speed           = 2.0f;

	// --- timers (seconds) ---
	float       respirationTime = 5.0f;
	float       infusionTime    = 10.0f;

	// --- loot ---
	const char* lootName        = "Souls Essences";
	int         lootChance      = 60; // percent

	// --- simple AI state ---
	enum class State : uint8_t { Idle, Patrol, Chase, Attack, Dead };
	State       state           = State::Patrol;
	float       stateTimer      = 0.0f;
	float       patrolDir       = 1.0f; // 1 = right, -1 = left
	float       patrolRange     = 4.0f;
	float       originX         = 0.0f; // patrol center

	// ---------------------------------------------------------------
	// Factory helpers – keep the same specs as the original classes
	// ---------------------------------------------------------------
	static EnemyComponent CreateSouls(float x = 0.0f, float z = 0.0f)
	{
		EnemyComponent e;
		e.type            = Type::Souls;
		e.health          = 80.0f;
		e.maxHealth       = 80.0f;
		e.speed           = 3.0f;
		e.respirationTime = 5.0f;
		e.infusionTime    = 10.0f;
		e.lootName        = "Souls Essences";
		e.lootChance      = 60;
		e.patrolRange     = 4.0f;
		e.originX         = x;
		return e;
	}

	static EnemyComponent CreateFairy(float x = 0.0f, float z = 0.0f)
	{
		EnemyComponent e;
		e.type            = Type::Fairy;
		e.health          = 120.0f;
		e.maxHealth       = 120.0f;
		e.speed           = 2.0f;
		e.respirationTime = 20.0f;
		e.infusionTime    = 20.0f;
		e.lootName        = "Fairy Wings";
		e.lootChance      = 40;
		e.patrolRange     = 6.0f;
		e.originX         = x;
		return e;
	}

	static EnemyComponent CreateCreature(float x = 0.0f, float z = 0.0f)
	{
		EnemyComponent e;
		e.type            = Type::Creature;
		e.health          = 200.0f;
		e.maxHealth       = 200.0f;
		e.speed           = 1.5f;
		e.respirationTime = 30.0f;
		e.infusionTime    = 30.0f;
		e.lootName        = "Creature Claws";
		e.lootChance      = 20;
		e.patrolRange     = 3.0f;
		e.originX         = x;
		return e;
	}

	// ---------------------------------------------------------------
	// Per-frame update — simple patrol AI
	// ---------------------------------------------------------------
	void Update(float dt, TransformComponent& transform)
	{
		if (!alive)
			return;

		stateTimer += dt;

		switch (state)
		{
		case State::Patrol:
		{
			glm::vec3 pos = transform.GetPosition();
			pos.x += patrolDir * speed * dt;

			// reverse direction at patrol bounds
			if (pos.x > originX + patrolRange)  patrolDir = -1.0f;
			if (pos.x < originX - patrolRange)  patrolDir =  1.0f;

			transform.SetPosition(pos);
			break;
		}
		case State::Idle:
		{
			// breathe / wait for respirationTime then resume patrol
			if (stateTimer >= respirationTime)
			{
				state      = State::Patrol;
				stateTimer = 0.0f;
			}
			break;
		}
		case State::Dead:
			break;
		default:
			break;
		}
	}

	// ---------------------------------------------------------------
	// Take damage – returns true if enemy just died
	// ---------------------------------------------------------------
	bool TakeDamage(float amount)
	{
		if (!alive) return false;
		health -= amount;
		if (health <= 0.0f)
		{
			health = 0.0f;
			alive  = false;
			state  = State::Dead;
			return true;
		}
		return false;
	}

	// ---------------------------------------------------------------
	// Loot roll — returns true if the loot dropped
	// ---------------------------------------------------------------
	bool RollLoot() const
	{
		return (std::rand() % 100) < lootChance;
	}
};
