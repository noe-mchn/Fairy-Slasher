#pragma once

#include <cstdint>
#include <glm/glm.hpp>

struct AIComponent
{
	// --- AI state machine ---
	enum class State : uint8_t { Idle, Patrol, Flee, Chase, Attack, Dead };
	State state = State::Patrol;
	float stateTimer = 0.0f;

	// --- reaction to player ---
	enum class Reaction : uint8_t
	{
		Passive,    // Creature: ignore player, patrol only
		Aggressive, // Souls: chase and attack
		Fearful     // Fairy: flee from player
	};
	Reaction reaction = Reaction::Passive;

	// --- detection ---
	float detectionRadius = 8.0f;
	float loseRadius      = 14.0f; // stop chasing/fleeing when player is this far

	// --- attack (Aggressive only) ---
	float attackRange    = 2.0f;
	float attackCooldown = 1.5f;
	float attackTimer    = 0.0f;
	float attackDamage   = 10.0f;

	// --- patrol ---
	float patrolDir   = 1.0f;  // 1 = right, -1 = left
	float patrolRange = 4.0f;
	glm::vec3 origin  = { 0.0f, 0.0f, 0.0f }; // patrol center

	// --- flee (Fearful only) ---
	float fleeSpeedMultiplier = 1.5f; // speed boost when fleeing

	// ---------------------------------------------------------------
	// Factory helpers
	// ---------------------------------------------------------------
	static AIComponent CreateForSouls(float x, float y, float z)
	{
		AIComponent ai;
		ai.reaction        = Reaction::Aggressive;
		ai.detectionRadius = 10.0f;
		ai.loseRadius      = 15.0f;
		ai.attackRange     = 2.5f;
		ai.attackCooldown  = 1.0f;
		ai.attackDamage    = 15.0f;
		ai.patrolRange     = 4.0f;
		ai.origin          = { x, y, z };
		return ai;
	}

	static AIComponent CreateForFairy(float x, float y, float z)
	{
		AIComponent ai;
		ai.reaction            = Reaction::Fearful;
		ai.detectionRadius     = 12.0f;
		ai.loseRadius          = 18.0f;
		ai.fleeSpeedMultiplier = 1.8f;
		ai.patrolRange         = 6.0f;
		ai.origin              = { x, y, z };
		return ai;
	}

	static AIComponent CreateForCreature(float x, float y, float z)
	{
		AIComponent ai;
		ai.reaction        = Reaction::Passive;
		ai.detectionRadius = 5.0f;
		ai.loseRadius      = 8.0f;
		ai.patrolRange     = 3.0f;
		ai.origin          = { x, y, z };
		return ai;
	}
};
