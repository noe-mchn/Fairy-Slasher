#include <iostream>
#include <algorithm>
#include <cstdlib>

#include "Core/Transform2dComponent.h"
#include "Core/UiComponent.h"
#include "Core/InputManager.h"
#include "Core/CameraComponent.h"
#include "Core/Mesh.h"
#include "Core/Texture.h"
#include "Core/TrasformComponent.h"
#include "Core/Window.h"
#include "ECS/Component.h"
#include "ECS/Entities.h"
#include "ECS/Registry.h"
#include "Tools/Chrono.h"
#include "Audio/SoundComponent.h"
#include "Math/Collision2d.h"

#include "CreatureData.h"
#include "CreatureAIComponent.h"
#include "AISystem.h"
#include "CaptureSystem.h"
#include "CauldronComponent.h"
#include "CauldronSystem.h"
#include "LootInventoryComponent.h"
#include "PlayerUpgradeComponent.h"
#include <PlayerComponent.h>
#include "ObjMaterialHelper.h"

 //make you ecs type with entity 8 / 16 / 32 / 64 and the size of allocation between 1 and infinity
using ecsType = KGR::ECS::Registry<KGR::ECS::Entity::_64, 100>;

UiComponent& getIdUi(std::vector<uint64_t> es, int id, ecsType &registry) {
	for (const auto& e : es) {
		auto& u = registry.GetComponent<UiComponent>(e);
		if (u.getId() == id)
			return u;
	}
}

int main(int argc, char** argv)
{

	// this part is due to the archi of the code to retrieve the folder resources
	std::filesystem::path exePath = argv[0];
	std::filesystem::path projectRoot = exePath.parent_path().parent_path().parent_path().parent_path().parent_path();


	// init the rendering system ( init glfw )
	KGR::RenderWindow::Init();
	// create your window with the size the name and the resources path

	//adapt x, y to your screen size
	std::unique_ptr<KGR::RenderWindow> window = std::make_unique<KGR::RenderWindow>(glm::vec2{ 1920,1080 }, "test", projectRoot / "Ressources");

	// getInputManager retrieve our input system where you can have the mouse pos mouse delta key pressed ... and set the cursor mode 
	//Disabled is to make the cam work smoothly to debug bring back to NORMAL
	window->GetInputManager()->SetMode(GLFW_CURSOR_DISABLED);

	// create your ecs 
	ecsType registry = ecsType{};


	// This is how to use the sounds and music system
	// and place it somewhere in the code where you want to use it

	// TODO when all test ok move this into a proper place 

	//MUSICS
	KGR::Audio::WavStreamComponent::Init();

	KGR::Audio::WavStreamComponent music;
	music.SetWav(KGR::Audio::WavStreamManager::Load("Musics/test.mp3"));
	music.SetVolume(10.0f);

	//SOUNDS
	KGR::Audio::WavComponent::Init();

	KGR::Audio::WavComponent sound;
	sound.SetWav(KGR::Audio::WavManager::Load("Sounds/sound.mp3"));
	sound.SetVolume(10.0f);


	// music test do not mind

	// camera 
	std::uint64_t cameraEntity = 0;
	{
		// a camera need a cameraComponent that can be orthographic or perspective and a transform

		// create the camera with the fov , the size of the window (must be updated ) and the far and near rendering and the mode 
		CameraComponent cam = CameraComponent::Create(glm::radians(45.0f), window->GetSize().x, window->GetSize().y, 0.01f, 100.0f, CameraComponent::Type::Perspective);
		TransformComponent transform;
		transform.SetPosition({ 0,3,5 });
		transform.LookAt({ 0,0,0 });
		cameraEntity = registry.CreateEntity();

		registry.AddComponents(cameraEntity, std::move(cam), std::move(transform), InventoryComponent{}, LootInventoryComponent{}, PlayerUpgradeComponent{});
	}

	// hand
	std::uint64_t handEntity = 0;
	{
	MeshComponent mesh;
	mesh.mesh = &MeshLoader::Load("Models/lanterne.obj", window->App());

	MaterialComponent mat;
	 
	mat.materials.resize(mesh.mesh->GetSubMeshesCount());
	for (int i = 0; i < mesh.mesh->GetSubMeshesCount(); ++i)
	{
	Material m;
	m.baseColor = &TextureLoader::Load("Textures/test_mat_bc.png", window->App());
	mat.materials[i] = m;
	}

	TransformComponent transform;
	transform.SetScale({ 0.15f, 0.15f, 0.15f });

	handEntity = registry.CreateEntity();
	registry.AddComponents(handEntity, std::move(mesh), std::move(mat), std::move(transform));
	}

	// map
	{
		MeshComponent mesh;
		mesh.mesh = &MeshLoader::Load("Models/map.obj", window->App());

		// Parse the .mtl referenced by the .obj to get the correct texture per sub-mesh
		auto matInfos = GetObjMaterials("Models/map.obj", projectRoot / "Ressources");

		// Override textures for shapes that have no texture in the .mtl
		for (auto& info : matInfos)
		{
			if (info.diffuseTexturePath.empty())
			{
				if (info.shapeName.find("lvl_floor") != std::string::npos ||
					info.shapeName.find("player_height") != std::string::npos ||
					info.shapeName.find("Plane") != std::string::npos)
					info.diffuseTexturePath = "Textures/herbe.png";
				else if (info.shapeName.find("limite") != std::string::npos)
					info.diffuseTexturePath = "Textures/mursombre.png";
				else if (info.shapeName.find("transition_rock") != std::string::npos ||
						 info.shapeName.find("Sphere") != std::string::npos)
					info.diffuseTexturePath = "Textures/cailloux.png";
				else if (info.shapeName.find("dryad_placeholder") != std::string::npos ||
						 info.shapeName.find("fairy_placeholder") != std::string::npos)
					info.diffuseTexturePath = "Textures/mousse.png";
			}
		}

		// Fix: background material Spheres/Cube/Cylinder have foret.png from .mtl but aren't background planes
		for (auto& info : matInfos)
		{
			if (info.shapeName.find("Sphere") != std::string::npos &&
				info.diffuseTexturePath.find("foret") != std::string::npos)
				info.diffuseTexturePath = "Textures/cailloux.png";
			if (info.shapeName == "Cube" &&
				info.diffuseTexturePath.find("foret") != std::string::npos)
				info.diffuseTexturePath = "Textures/cailloux.png";
			if (info.shapeName == "Cylinder" &&
				info.diffuseTexturePath.find("foret") != std::string::npos)
				info.diffuseTexturePath = "Textures/cailloux.png";
		}

		MaterialComponent mat;
		mat.materials.resize(mesh.mesh->GetSubMeshesCount());
		for (int i = 0; i < mesh.mesh->GetSubMeshesCount(); ++i)
		{
			Material m;
			if (i < static_cast<int>(matInfos.size()) && !matInfos[i].diffuseTexturePath.empty())
				m.baseColor = &TextureLoader::Load(matInfos[i].diffuseTexturePath, window->App());
			mat.materials[i] = m;
		}

		TransformComponent transform;
		transform.SetPosition({ 0, 0, 0 });
		transform.SetScale({ 1.0f, 1.0f, 1.0f });

		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(mesh), std::move(mat), std::move(transform));
	}

	// light
	{
		// directional sun – subtle purple moonlight (from Blender, toned down)
		glm::vec3 sunColor = glm::vec3(0.3f, 0.1f, 0.55f);
		LightComponent<LightData::Type::Directional> lc = LightComponent<LightData::Type::Directional>::Create(sunColor, sunColor, 100.0f);
		TransformComponent transform;
		transform.SetPosition({ 0,5,0 });
		transform.LookAtDir({ -0.2f,-1,- 0.3f });
		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(lc), std::move(transform));
	}

	// cauldron
	{
		MeshComponent mesh;
		mesh.mesh = &MeshLoader::Load("Models/cube.obj", window->App());

		MaterialComponent mat;
		mat.materials.resize(mesh.mesh->GetSubMeshesCount());
		for (int i = 0; i < mesh.mesh->GetSubMeshesCount(); ++i)
		{
			Material m;
			m.baseColor = &TextureLoader::Load("Textures/test_mat_bc.png", window->App());
			mat.materials[i] = m;
		}

		TransformComponent transform;
		transform.SetPosition({ 0, 0, -3 });
		transform.SetScale({ 0.8f, 0.8f, 0.8f });

		CauldronComponent cauldron;
		cauldron.maxSlots = 1;
		cauldron.interactionRange = 5.0f;

		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(mesh), std::move(mat), std::move(transform), std::move(cauldron));
	}

	// ui ( not fully operational)
	{
		// you need texture transform and ui component
		// for the transform it only use for the rotation 
		TransformComponent2d transform1;
		TransformComponent2d transform2;
		TransformComponent2d transform3;
		TransformComponent2d transform4;
		// here you can set a rotation ( ROTATION FROM THE CENTER OF THE MESH )
		//transform.SetRotation(glm::radians(-45.0f));
		// create your ui with a virtual resolution and an anchor default center
		UiComponent ui1({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent ui2({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent ui3({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent ui4({ 1920,1080 }, UiComponent::Anchor::LeftTop);

		// here set the position in the virtual resolution
		ui1.SetPos({ 870, 900 });
		ui2.SetPos({ 915, 900 });
		ui3.SetPos({ 960, 900 });
		ui4.SetPos({ 1005, 900 });

		// here the scale
		ui1.SetScale({ 40,40 });
		ui2.SetScale({ 40,40 });
		ui3.SetScale({ 40,40 });
		ui4.SetScale({ 40,40 });

		ui1.setId(0);
		ui2.setId(1);
		ui3.setId(2);
		ui4.setId(3);

		// create a texture but be aware that only the first texture in the component will be use 
		TextureComponent texture1;
		TextureComponent texture2;
		TextureComponent texture3;
		TextureComponent texture4;

		texture1.texture = &TextureLoader::Load("Textures/InventorySlot.jpg", window->App());
		texture2.texture = &TextureLoader::Load("Textures/InventorySlot.jpg", window->App());
		texture3.texture = &TextureLoader::Load("Textures/InventorySlot.jpg", window->App());
		texture4.texture = &TextureLoader::Load("Textures/InventorySlot.jpg", window->App());


		// same as always 
		auto e1 = registry.CreateEntity();
		auto e2 = registry.CreateEntity();
		auto e3 = registry.CreateEntity();
		auto e4 = registry.CreateEntity();

		registry.AddComponents(e1, std::move(transform1), std::move(ui1), std::move(texture1), std::move(CollisionComp2d{}));
		registry.AddComponents(e2, std::move(transform2), std::move(ui2), std::move(texture2), std::move(CollisionComp2d{}));
		registry.AddComponents(e3, std::move(transform3), std::move(ui3), std::move(texture3), std::move(CollisionComp2d{}));
		registry.AddComponents(e4, std::move(transform4), std::move(ui4), std::move(texture4), std::move(CollisionComp2d{}));


	}

	// creatures
	{
		// Souls creature
		MeshComponent mesh;
		mesh.mesh = &MeshLoader::Load("Models/cube.obj", window->App());

		MaterialComponent mat;
		mat.materials.resize(mesh.mesh->GetSubMeshesCount());
		for (int i = 0; i < mesh.mesh->GetSubMeshesCount(); ++i)
		{
			Material m;
			m.baseColor = &TextureLoader::Load("Textures/test_mat_bc.png", window->App());
			mat.materials[i] = m;
		}

		TransformComponent transform;
		transform.SetPosition({ 5, 0, 0 });
		transform.SetScale({ 0.5f, 0.5f, 0.5f });

		CreatureAIComponent ai;
		ai.data = CreatureData::MakeSouls();

		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(mesh), std::move(mat), std::move(transform), std::move(ai));
	}

	{
		// Fairy creature
		MeshComponent mesh;
		mesh.mesh = &MeshLoader::Load("Models/cube.obj", window->App());

		MaterialComponent mat;
		mat.materials.resize(mesh.mesh->GetSubMeshesCount());
		for (int i = 0; i < mesh.mesh->GetSubMeshesCount(); ++i)
		{
			Material m;
			m.baseColor = &TextureLoader::Load("Textures/test_mat_bc.png", window->App());
			mat.materials[i] = m;
		}

		TransformComponent transform;
		transform.SetPosition({ -5, 0, 3 });
		transform.SetScale({ 0.4f, 0.4f, 0.4f });

		CreatureAIComponent ai;
		ai.data = CreatureData::MakeFairy();

		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(mesh), std::move(mat), std::move(transform), std::move(ai));
	}

	{
		// Rare Soul
		MeshComponent mesh;
		mesh.mesh = &MeshLoader::Load("Models/cube.obj", window->App());

		MaterialComponent mat;
		mat.materials.resize(mesh.mesh->GetSubMeshesCount());
		for (int i = 0; i < mesh.mesh->GetSubMeshesCount(); ++i)
		{
			Material m;
			m.baseColor = &TextureLoader::Load("Textures/test_mat_bc.png", window->App());
			mat.materials[i] = m;
		}

		TransformComponent transform;
		transform.SetPosition({ 3, 0, -5 });
		transform.SetScale({ 0.6f, 0.6f, 0.6f });

		CreatureAIComponent ai;
		ai.data = CreatureData::MakeRareSoul();

		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(mesh), std::move(mat), std::move(transform), std::move(ai));
	}

	{
		// Rare Fairy
		MeshComponent mesh;
		mesh.mesh = &MeshLoader::Load("Models/cube.obj", window->App());

		MaterialComponent mat;
		mat.materials.resize(mesh.mesh->GetSubMeshesCount());
		for (int i = 0; i < mesh.mesh->GetSubMeshesCount(); ++i)
		{
			Material m;
			m.baseColor = &TextureLoader::Load("Textures/test_mat_bc.png", window->App());
			mat.materials[i] = m;
		}

		TransformComponent transform;
		transform.SetPosition({ -3, 0, -4 });
		transform.SetScale({ 0.35f, 0.35f, 0.35f });

		CreatureAIComponent ai;
		ai.data = CreatureData::MakeRareFairy();

		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(mesh), std::move(mat), std::move(transform), std::move(ai));
	}

	//Player
	{
		// A player needs : a Mesh, a mat, a Transform
		MeshComponent mesh;
		mesh.mesh = &MeshLoader::Load("Models/cube.obj", window->App());

		MaterialComponent mat;
		mat.materials.resize(mesh.mesh->GetSubMeshesCount());
		for (int i = 0; i < mesh.mesh->GetSubMeshesCount(); ++i)
		{
			Material m;
			m.baseColor = &TextureLoader::Load("Textures/test_mat_e.png", window->App());
			mat.materials[i] = m;
		}

		TransformComponent transform;
		transform.SetPosition({ 0, 0, 0 });
		transform.SetScale({ 1.0f, 1.0f, 1.0f });

		PlayerComponent player;
		player.playerTransform = &transform;

		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(mesh), std::move(mat), std::move(transform), std::move(player));
	}

	PlayerComponent* player = nullptr;
	{
		auto es = registry.GetAllComponentsView<PlayerComponent, TransformComponent, MeshComponent, MaterialComponent>();
		for (auto& e : es) {
			player = &registry.GetComponent<PlayerComponent>(e);
		}
	}

	auto& handTransform = registry.GetComponent<TransformComponent>(handEntity);

	auto es = registry.GetAllComponentsView<CreatureAIComponent, TransformComponent>();
	for (auto& e : es)
	{
		auto& ai = registry.GetComponent<CreatureAIComponent>(e);
		auto& tr = registry.GetComponent<TransformComponent>(e);
		ai.Init(&tr, player, { {5,0,0}, {-5,0,3}, {3,0,-5}, {0,0,0} });
	}

	float captureTimer = 0.0f;
	float captureDuration = 1.5f;
	float captureRange = 2.5f;

	auto ui = registry.GetAllComponentsView<UiComponent>();
	float current = 0.0f;
	KGR::Tools::Chrono<float> chrono;
	float timer = 0.0f;

	static float mouseSensitivity = 0.0025f;
	float yaw = 0.0f;
	float pitch = glm::radians(-10.0f);

	//get camera
	auto& mainCamera = registry.GetComponent<CameraComponent>(cameraEntity);
	auto& transformCamera = registry.GetComponent<TransformComponent>(cameraEntity);

	assert(player != nullptr && "Player component not found!");

	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

	// creature respawn
	const glm::vec3 spawnPoints[] = { {5,0,0}, {-5,0,3}, {3,0,-5}, {-3,0,-4}, {7,0,2}, {-7,0,-2} };
	const int spawnPointCount = sizeof(spawnPoints) / sizeof(spawnPoints[0]);
	const size_t maxCreaturesInWorld = 4;
	float respawnTimer = 0.0f;
	const float respawnInterval = 15.0f;

	{
		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);
		cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
		cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
	}

	while (!window->ShouldClose())
	{
		float actual = chrono.GetElapsedTime().AsSeconds();
		float dt = actual - current;
		current = actual;
		KGR::RenderWindow::PollEvent();
		window->Update();
		{
			auto input = window->GetInputManager();

			auto mousemove = input->GetMouseDelta();
			float mouseX = mousemove.x;
			float mouseY = mousemove.y;

			yaw -= mouseX * mouseSensitivity;
			pitch -= mouseY * mouseSensitivity;

			pitch = std::clamp(pitch, glm::radians(-89.0f), glm::radians(89.0f));

			glm::vec3 front;

			player->SetRotation({ 0.0f, yaw, 0.0f });
			transformCamera.SetRotation({ pitch, yaw, 0.0f });

			if (input->IsKeyDown(KGR::SpecialKey::Shift))
			{
				player->startRunning();
			}
			if (input->IsKeyReleased(KGR::SpecialKey::Shift))
			{
				player->stopRunning();
			}

			if (input->IsKeyDown(KGR::SpecialKey::Ctrl))
			{
				player->startCrouching();
			}
			if (input->IsKeyReleased(KGR::SpecialKey::Ctrl))
			{
				player->stopCrouching();
			}

			glm::vec3 moveDir{ 0.0f };

			if (input->IsKeyDown(KGR::Key::Z))
				moveDir += player->getForward();
			if (input->IsKeyDown(KGR::Key::S))
				moveDir -= player->getForward();
			if (input->IsKeyDown(KGR::Key::D))
				moveDir += player->getRight();
			if (input->IsKeyDown(KGR::Key::Q))
				moveDir -= player->getRight();

			if (glm::length(moveDir) > 0.0f)
				moveDir = glm::normalize(moveDir);

			player->move(moveDir, dt);

			// slot selection (1-4)
			auto& inv = registry.GetComponent<InventoryComponent>(cameraEntity);
			if (input->IsKeyPressed(KGR::Key::Num1)) { inv.selectedSlot = 0; std::cout << "[INV] Slot 1 selected" << std::endl; }
			if (input->IsKeyPressed(KGR::Key::Num2)) { inv.selectedSlot = 1; std::cout << "[INV] Slot 2 selected" << std::endl; }
			if (input->IsKeyPressed(KGR::Key::Num3)) { inv.selectedSlot = 2; std::cout << "[INV] Slot 3 selected" << std::endl; }
			if (input->IsKeyPressed(KGR::Key::Num4)) { inv.selectedSlot = 3; std::cout << "[INV] Slot 4 selected" << std::endl; }

			// update UI to highlight selected slot
			{
				const glm::vec2 slotPositions[4] = { {870,900}, {915,900}, {960,900}, {1005,900} };
				for (int s = 0; s < 4; ++s)
				{
					auto& u = getIdUi(ui, s, registry);
					glm::vec2 p = slotPositions[s];
					if (s == inv.selectedSlot)
						p.y = 890;
					u.SetPos(p);
				}
			}

			// E : deposit selected creature into nearest cauldron
			if (input->IsKeyPressed(KGR::Key::E))
			{
				std::cout << "[CAULDRON] Trying to deposit from slot " << inv.selectedSlot + 1
						  << (inv.IsSlotEmpty(inv.selectedSlot) ? " (EMPTY)" : " (has creature)") << std::endl;
				bool deposited = CauldronSystem::DepositCreature(registry, cameraEntity, inv.selectedSlot);
				if (deposited)
					std::cout << "[CAULDRON] >> Creature deposited! Infusion started." << std::endl;
				else
					std::cout << "[CAULDRON] >> Failed (empty slot, no cauldron nearby, or cauldron full)" << std::endl;
			}

			// F : collect finished loot from nearest cauldron
			if (input->IsKeyPressed(KGR::Key::F))
			{
				// debug: show distance to every cauldron
				size_t slotsBefore = 0;
				{
					glm::vec3 pPos = registry.GetComponent<TransformComponent>(cameraEntity).GetPosition();
					auto cds = registry.GetAllComponentsView<CauldronComponent, TransformComponent>();
					for (auto& ce : cds)
					{
						auto& ct = registry.GetComponent<TransformComponent>(ce);
						auto& cc = registry.GetComponent<CauldronComponent>(ce);
						slotsBefore += cc.slots.size();
						float d = glm::length(ct.GetPosition() - pPos);
						std::cout << "[CAULDRON] Distance: " << d << " (range=" << cc.interactionRange << ", finished=" << cc.FinishedCount() << ")" << std::endl;
					}
				}
				int collected = CauldronSystem::CollectLoot(registry, cameraEntity);
				size_t slotsAfter = 0;
				{
					auto cds = registry.GetAllComponentsView<CauldronComponent>();
					for (auto& ce : cds)
						slotsAfter += registry.GetComponent<CauldronComponent>(ce).slots.size();
				}
				if (collected > 0)
				{
					std::cout << "[CAULDRON] >> Collected " << collected << " loot(s)!" << std::endl;
					auto& lootInv = registry.GetComponent<LootInventoryComponent>(cameraEntity);
					for (auto& item : lootInv.items)
						std::cout << "  - " << item.name << " x" << item.quantity << std::endl;
				}
				else if (slotsBefore > slotsAfter)
					std::cout << "[CAULDRON] >> Infusion processed but loot roll FAILED (bad luck! creature lost)" << std::endl;
				else
					std::cout << "[CAULDRON] >> No loot ready (not near cauldron, or infusion not finished)" << std::endl;
			}

			// V : unlock night vision (requires 1 "Rare Souls Essences")
			if (input->IsKeyPressed(KGR::Key::V))
			{
				if (registry.HasComponent<PlayerUpgradeComponent>(cameraEntity) &&
					registry.HasComponent<LootInventoryComponent>(cameraEntity))
				{
					auto& upgrades = registry.GetComponent<PlayerUpgradeComponent>(cameraEntity);
					auto& lootInv  = registry.GetComponent<LootInventoryComponent>(cameraEntity);
					if (upgrades.nightVision)
						std::cout << "[UPGRADE] Night Vision already unlocked" << std::endl;
					else if (PlayerUpgradeComponent::TryUnlockNightVision(upgrades, lootInv))
						std::cout << "[UPGRADE] >> Night Vision UNLOCKED!" << std::endl;
					else
						std::cout << "[UPGRADE] >> Not enough loot (need 1x Rare Souls Essences, have " << lootInv.GetQuantity("Rare Souls Essences") << ")" << std::endl;
				}
			}

			// J : unlock double jump (requires 1 "Fairy Wings")
			if (input->IsKeyPressed(KGR::Key::J))
			{
				if (registry.HasComponent<PlayerUpgradeComponent>(cameraEntity) &&
					registry.HasComponent<LootInventoryComponent>(cameraEntity))
				{
					auto& upgrades = registry.GetComponent<PlayerUpgradeComponent>(cameraEntity);
					auto& lootInv  = registry.GetComponent<LootInventoryComponent>(cameraEntity);
					if (upgrades.doubleJump)
						std::cout << "[UPGRADE] Double Jump already unlocked" << std::endl;
					else if (PlayerUpgradeComponent::TryUnlockDoubleJump(upgrades, lootInv))
						std::cout << "[UPGRADE] >> Double Jump UNLOCKED!" << std::endl;
					else
						std::cout << "[UPGRADE] >> Not enough loot (need 1x Fairy Wings, have " << lootInv.GetQuantity("Fairy Wings") << ")" << std::endl;
				}
			}

			std::uint64_t nearestCreature{};
			bool creatureInRange = CaptureSystem::FindNearest(registry, cameraEntity, captureRange, nearestCreature);

			if (input->IsKeyDown(KGR::Key::R) && creatureInRange)
			{
				captureTimer += dt;
				std::cout << "\r[CAPTURE] Capturing... " << static_cast<int>(captureTimer / captureDuration * 100) << "%" << std::flush;
				if (captureTimer >= captureDuration)
				{
					bool ok = CaptureSystem::Capture(registry, cameraEntity, nearestCreature);
					std::cout << std::endl;
					if (ok)
					{
						std::cout << "[CAPTURE] >> Creature captured! Inventory:";
						for (int s = 0; s < 4; ++s)
							std::cout << " [" << s+1 << "]=" << (inv.IsSlotEmpty(s) ? "vide" : "plein");
						std::cout << std::endl;
					}
					else
						std::cout << "[CAPTURE] >> Failed (inventory full?)" << std::endl;
					captureTimer = 0.0f;
				}
			}
			else
			{
				if (captureTimer > 0.0f)
					std::cout << std::endl << "[CAPTURE] Cancelled" << std::endl;
				captureTimer = 0.0f;
			}

			if (input->IsKeyPressed(KGR::Key::T))
			{
				auto removed = inv.RemoveFromSlot(inv.selectedSlot);
				if (removed.has_value())
				{
					std::cout << "[INV] Released creature from slot " << inv.selectedSlot + 1 << std::endl;
					CreatureData releasedData = removed->data;
					glm::vec3 releasePos = registry.GetComponent<TransformComponent>(cameraEntity).GetPosition() + cameraFront * 2.0f;

					MeshComponent mesh;
					mesh.mesh = &MeshLoader::Load("Models/cube.obj", window->App());
					player->move(moveDir, dt);

					MaterialComponent mat;
					mat.materials.resize(mesh.mesh->GetSubMeshesCount());
					for (int i = 0; i < mesh.mesh->GetSubMeshesCount(); ++i)
					{
						Material m;
						m.baseColor = &TextureLoader::Load("Textures/test_mat_bc.png", window->App());
						mat.materials[i] = m;
					}

					TransformComponent transform;
					transform.SetPosition(releasePos);
					transform.SetScale({ 0.5f, 0.5f, 0.5f });

					CreatureAIComponent ai;
					ai.data = releasedData;

					auto released = registry.CreateEntity();
					registry.AddComponents(released, std::move(mesh), std::move(mat), std::move(transform), std::move(ai));

					auto& relAi = registry.GetComponent<CreatureAIComponent>(released);
					auto& relTr = registry.GetComponent<TransformComponent>(released);
					auto* playerTr = &registry.GetComponent<TransformComponent>(cameraEntity);
					relAi.Init(&relTr, player, { releasePos, {5,0,0}, {-5,0,3}, {3,0,-5} });
				}
			}
			if (input->IsKeyPressed(KGR::SpecialKey::Space))
			{
				auto positionY = player->playerTransform->GetPosition().y;
				if (player->jumpCharge > 0) {
					player->verticalVelocity += 8.0f;
					player->jumpCharge--;
				}
				positionY += player->verticalVelocity * dt;
				player->playerTransform->Translate({ 0.0f, player->verticalVelocity * dt, 0.0f });
				if (positionY > 0.0f) {
					player->isGrounded = false;
				}
			}

			if (!player->isGrounded)
			{
				player->verticalVelocity -= 3.0f * dt;
				player->playerTransform->Translate({ 0.0f, player->verticalVelocity * dt, 0.0f });
				auto positionY = player->playerTransform->GetPosition().y;
				if (positionY <= 0.0f)
				{
					auto pos = player->playerTransform->GetPosition();
					pos.y = 0.0f;
					player->playerTransform->SetPosition(pos);
					player->isGrounded = true;
					player->verticalVelocity = 0.0f;
					player->jumpCharge = 2;
				}
			}
		}
		auto mousePos = window.get()->GetInputManager()->GetMousePosition();
		float aspectRatio = static_cast<float>(window->GetSize().x) / static_cast<float>(window->GetSize().y);
		auto mouseinAR = UiComponent::VrToNdc(mousePos, window->GetSize(), aspectRatio, false);

		auto es = registry.GetAllComponentsView<CollisionComp2d, UiComponent>();
		for (auto e : es)
		{
			auto& t = registry.GetComponent<CollisionComp2d>(e);
			auto& u = registry.GetComponent<UiComponent>(e);
			t.Update(u.GetPosNdc(aspectRatio), u.GetScaleNdc(aspectRatio));

			if (t.aabb.IsColliding(mouseinAR))
				u.SetColor({ 1,0,0,1 });
			else
				u.SetColor({ 0,1,0,1 });
		}
		transformCamera.SetPosition(player->playerTransform->GetPosition() + glm::vec3{ 0.0f, 0.0f, 0.0f });

		{
			glm::quat camRot = transformCamera.GetRotation();
			auto camPos = transformCamera.GetPosition();

			glm::vec3 right = camRot * glm::vec3(1.0f, 0.0f, 0.0f);
			glm::vec3 up = camRot * glm::vec3(0.0f, 1.0f, 0.0f);
			glm::vec3 forward = camRot * glm::vec3(0.0f, 0.0f, -1.0f);

			glm::vec3 localOffset = { 0.25f, -0.05f, 0.5f };

			glm::vec3 lanternPos = camPos + right * localOffset.x + up * localOffset.y + forward * localOffset.z;

			handTransform.SetPosition(lanternPos);
			handTransform.SetOrientation(camRot);
			
		}

		{
			auto es = registry.GetAllComponentsView<CameraComponent, TransformComponent>();
			if (es.size() != 1)
				throw std::runtime_error("need one and one cam");
			for (auto& e : es)
			{
				registry.GetComponent<CameraComponent>(e).UpdateCamera(registry.GetComponent<TransformComponent>(e).GetFullTransform());
				registry.GetComponent<CameraComponent>(e).SetAspect(window->GetSize().x, window->GetSize().y);
				window->RegisterCam(registry.GetComponent<CameraComponent>(e), registry.GetComponent<TransformComponent>(e));
			}
		}

		{
			auto es = registry.GetAllComponentsView<MeshComponent, TransformComponent, MaterialComponent>();
			for (auto& e : es)
			{
				window->RegisterRender(
					registry.GetComponent<MeshComponent>(e),
					registry.GetComponent<TransformComponent>(e),
					registry.GetComponent<MaterialComponent>(e));
			}

		}

		AISystem::Update(registry, dt);
		CauldronSystem::Update(registry, dt);

		// creature respawn
		{
			auto creatures = registry.GetAllComponentsView<CreatureAIComponent, TransformComponent>();
			if (creatures.size() < maxCreaturesInWorld)
			{
				respawnTimer += dt;
				if (respawnTimer >= respawnInterval)
				{
					respawnTimer = 0.0f;

					// pick a random creature type
					CreatureData newData;
					float scale = 0.5f;
					int typeRoll = std::rand() % 4;
					switch (typeRoll)
					{
					case 0: newData = CreatureData::MakeSouls();     scale = 0.5f;  break;
					case 1: newData = CreatureData::MakeFairy();     scale = 0.4f;  break;
					case 2: newData = CreatureData::MakeRareSoul();  scale = 0.6f;  break;
					case 3: newData = CreatureData::MakeRareFairy(); scale = 0.35f; break;
					}

					glm::vec3 spawnPos = spawnPoints[std::rand() % spawnPointCount];

					MeshComponent mesh;
					mesh.mesh = &MeshLoader::Load("Models/cube.obj", window->App());

					MaterialComponent mat;
					mat.materials.resize(mesh.mesh->GetSubMeshesCount());
					for (int i = 0; i < mesh.mesh->GetSubMeshesCount(); ++i)
					{
						Material m;
						m.baseColor = &TextureLoader::Load("Textures/test_mat_bc.png", window->App());
						mat.materials[i] = m;
					}

					TransformComponent transform;
					transform.SetPosition(spawnPos);
					transform.SetScale({ scale, scale, scale });

					CreatureAIComponent ai;
					ai.data = newData;

					auto spawned = registry.CreateEntity();
					registry.AddComponents(spawned, std::move(mesh), std::move(mat), std::move(transform), std::move(ai));

					auto& spAi = registry.GetComponent<CreatureAIComponent>(spawned);
					auto& spTr = registry.GetComponent<TransformComponent>(spawned);
					auto* playerTr = &registry.GetComponent<TransformComponent>(cameraEntity);
					spAi.Init(&spTr, player, { spawnPos, {5,0,0}, {-5,0,3}, {3,0,-5} });

					std::cout << "[SPAWN] New creature spawned: " << newData.lootName
							  << " at (" << spawnPos.x << ", " << spawnPos.y << ", " << spawnPos.z << ")" << std::endl;
				}
			}
			else
			{
				respawnTimer = 0.0f;
			}
		}

		// debug: print cauldron infusion progress once per second
		{
			static float cauldronLogTimer = 0.0f;
			cauldronLogTimer += dt;
			if (cauldronLogTimer >= 1.0f)
			{
				cauldronLogTimer = 0.0f;
				auto cauldrons = registry.GetAllComponentsView<CauldronComponent>();
				for (auto& ce : cauldrons)
				{
					auto& c = registry.GetComponent<CauldronComponent>(ce);
					for (size_t i = 0; i < c.slots.size(); ++i)
					{
						auto& slot = c.slots[i];
						if (!slot.finished)
							std::cout << "[CAULDRON] Infusing " << slot.data.lootName << " : " << static_cast<int>(slot.elapsed) << "s / " << static_cast<int>(slot.data.infusionTime) << "s" << std::endl;
						else
							std::cout << "[CAULDRON] " << slot.data.lootName << " READY! Press F near cauldron to collect." << std::endl;
					}
				}
			}
		}

		timer += dt;
		if (timer >= 1.0f) {
			timer -= 1.0f;

			player->OxygenDepletion();
		}
		if (player->isOutOFOxygen()) {
			std::cout << "Player is out of oxygen!\n";
		}
		if (player->isDead()) {
			std::cout << "Player is dead!\n";
		}


		//Test Sound
		if (window->GetInputManager()->IsKeyPressed(KGR::Key::P))
			sound.Play();
		{
			auto es = registry.GetAllComponentsView<LightComponent<LightData::Type::Point>, TransformComponent>();
			for (auto& e : es)
				window->RegisterLight(registry.GetComponent<LightComponent<LightData::Type::Point>>(e), registry.GetComponent<TransformComponent>(e));
		}
		{
			auto es = registry.GetAllComponentsView<LightComponent<LightData::Type::Spot>, TransformComponent>();
			for (auto& e : es)
				window->RegisterLight(registry.GetComponent<LightComponent<LightData::Type::Spot>>(e), registry.GetComponent<TransformComponent>(e));
		}
		{
			auto es = registry.GetAllComponentsView<LightComponent<LightData::Type::Directional>, TransformComponent>();
			for (auto& e : es)
				window->RegisterLight(registry.GetComponent<LightComponent<LightData::Type::Directional>>(e), registry.GetComponent<TransformComponent>(e));
		}
		{
			auto es = registry.GetAllComponentsView < TextureComponent, TransformComponent2d, UiComponent >();
			for (auto& e : es)
			{
				auto transform = registry.GetComponent<TransformComponent2d>(e);
				auto ui = registry.GetComponent<UiComponent>(e);
				auto texture = registry.GetComponent<TextureComponent>(e);
				window->RegisterUi(ui, transform, texture);
			}
		}
		// night vision changes the ambient clear color
		glm::vec4 clearColor = { 0.53f, 0.81f, 0.92f, 1.0f };
		if (registry.HasComponent<PlayerUpgradeComponent>(cameraEntity))
		{
			auto& upgrades = registry.GetComponent<PlayerUpgradeComponent>(cameraEntity);
			if (upgrades.nightVision)
				clearColor = { 0.1f, 0.4f, 0.15f, 1.0f };
		}
		window->Render(clearColor);
	}
	window->Destroy();
	KGR::RenderWindow::End();
}