#pragma once

#include "Enemies/EnemyComponent.h"
#include "AI/AIComponent.h"
#include "Core/Mesh.h"
#include "Core/Texture.h"
#include "Core/TrasformComponent.h"

/// Utility to spawn enemy entities into the ECS registry.
/// Each enemy gets: EnemyComponent + AIComponent + MeshComponent + TextureComponent + TransformComponent
struct EnemySpawner
{
	/// Spawn a Souls-type enemy at the given world position.
	template<typename Registry>
	static void SpawnSouls(Registry& reg, KGR::RenderWindow& window,
		float x, float y, float z)
	{
		auto enemy = EnemyComponent::CreateSouls(x, z);
		auto ai    = AIComponent::CreateForSouls(x, y, z);
		SpawnEnemy(reg, window, std::move(enemy), std::move(ai), { x, y, z }, { 1.0f, 1.0f, 1.0f });
	}

	/// Spawn a Fairy-type enemy at the given world position.
	template<typename Registry>
	static void SpawnFairy(Registry& reg, KGR::RenderWindow& window,
		float x, float y, float z)
	{
		auto enemy = EnemyComponent::CreateFairy(x, z);
		auto ai    = AIComponent::CreateForFairy(x, y, z);
		SpawnEnemy(reg, window, std::move(enemy), std::move(ai), { x, y, z }, { 0.8f, 0.8f, 0.8f });
	}

	/// Spawn a Creature-type enemy at the given world position.
	template<typename Registry>
	static void SpawnCreature(Registry& reg, KGR::RenderWindow& window,
		float x, float y, float z)
	{
		auto enemy = EnemyComponent::CreateCreature(x, z);
		auto ai    = AIComponent::CreateForCreature(x, y, z);
		SpawnEnemy(reg, window, std::move(enemy), std::move(ai), { x, y, z }, { 1.5f, 1.5f, 1.5f });
	}

private:
	template<typename Registry>
	static void SpawnEnemy(Registry& reg, KGR::RenderWindow& window,
		EnemyComponent&& enemy, AIComponent&& ai,
		glm::vec3 position, glm::vec3 scale)
	{
		MeshComponent mesh;
		mesh.mesh = &MeshLoader::Load("Models/cube.obj", window.App());

		TextureComponent tex;
		tex.SetSize(mesh.mesh->GetSubMeshesCount());
		for (int i = 0; i < mesh.mesh->GetSubMeshesCount(); ++i)
			tex.AddTexture(i, &TextureLoader::Load("Textures/viking_room.png", window.App()));

		TransformComponent transform;
		transform.SetPosition(position);
		transform.SetScale(scale);

		auto e = reg.CreateEntity();
		reg.AddComponents(e,
			std::move(enemy),
			std::move(ai),
			std::move(mesh),
			std::move(tex),
			std::move(transform));
	}
};
