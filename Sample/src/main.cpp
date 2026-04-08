#include <iostream>
#include <algorithm>

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
#include "inventory.h"
#include <PlayerComponent.h>

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
	Inventory inventory;


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

		registry.AddComponents(cameraEntity, std::move(cam), std::move(transform), InventoryComponent{});
	}

	// lantern (held by player, attached to camera)
	std::uint64_t lanternEntity = 0;
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

		lanternEntity = registry.CreateEntity();
		registry.AddComponents(lanternEntity, std::move(mesh), std::move(mat), std::move(transform));
	}

	// ground plane
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
		transform.SetPosition({ 0, -0.5f, 0 });
		transform.SetScale({ 50.0f, 0.5f, 50.0f });

		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(mesh), std::move(mat), std::move(transform));
	}

	// light
	{
		// directional light so the whole scene is illuminated
		LightComponent<LightData::Type::Directional> lc = LightComponent<LightData::Type::Directional>::Create({ 1, 1,1 }, { 1,1,1 }, 100.0f);
		TransformComponent transform;
		transform.SetPosition({ 0,5,0 });
		transform.LookAtDir({ -0.2f,-1,-0.3f });
		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(lc), std::move(transform));
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

	//Hand mesh
	{
		MeshComponent mesh;
		mesh.mesh = &MeshLoader::Load("Models/MAIN2.obj", window->App());

		MaterialComponent mat;
		mat.materials.resize(mesh.mesh->GetSubMeshesCount());
		for (int i = 0; i < mesh.mesh->GetSubMeshesCount(); ++i)
		{
			Material m;
			m.baseColor = &TextureLoader::Load("Textures/test_mat_e.png", window->App());
			mat.materials[i] = m;
		}

		TransformComponent transform;
		transform.SetPosition({ 0, -100, 0 });
		transform.SetScale({ 0.3f, 1.0f, 0.25f });

		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(mesh), /*std::move(mat),*/ std::move(transform));
	}

	//Player
	{
		// A player needs : a Mesh, a Cam, a Transform
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
		auto es = registry.GetAllComponentsView<PlayerComponent, TransformComponent>();
		for (auto& e : es) {
			player = &registry.GetComponent<PlayerComponent>(e);
		}
	}

	//this check can be wrong if you have multiple mesh and transform without player component might change later
	TransformComponent* handTransform = nullptr;
	{
		auto es = registry.GetAllComponentsView<MeshComponent, TransformComponent>();
		for (auto& e : es) {
			auto& mesh = registry.GetComponent<MeshComponent>(e);
			if (!registry.HasComponent<CreatureAIComponent>(e) || !registry.HasComponent<PlayerComponent>(e)) {
				handTransform = &registry.GetComponent<TransformComponent>(e);
				break;
			}
		}
	}

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
	auto mainCamera = registry.GetAllComponentsView<CameraComponent, TransformComponent>().begin()[0];
	auto& transformCamera = registry.GetComponent<TransformComponent>(mainCamera);

	assert(player != nullptr && "Player component not found!");

	assert(handTransform != nullptr && "Hand Transform not found!");

	//Base on camera
	handTransform->SetPosition(transformCamera.GetPosition() + glm::vec3{ 5.0, -2.0, 0.5 });
	handTransform->SetScale({ 0.1f, 0.25f, 0.25f });

	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

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

			if (input->IsKeyDown(KGR::Key::Num1)) {
				auto& u = getIdUi(ui, 0, registry);
				u.SetPos({ 870, 890 });
				auto& u1 = getIdUi(ui, 1, registry);
				u1.SetPos({ 915, 900 });
				auto& u2 = getIdUi(ui, 2, registry);
				u2.SetPos({ 960, 900 });
				auto& u3 = getIdUi(ui, 3, registry);
				u3.SetPos({ 1005, 900 });
			}
			if (input->IsKeyDown(KGR::Key::Num2)) {
				auto& u = getIdUi(ui, 1, registry);
				u.SetPos({ 915, 890 });
				auto& u1 = getIdUi(ui, 2, registry);
				u1.SetPos({ 960, 900 });
				auto& u2 = getIdUi(ui, 3, registry);
				u2.SetPos({ 1005, 900 });
				auto& u3 = getIdUi(ui, 0, registry);
				u3.SetPos({ 870, 900 });
			}
			if (input->IsKeyDown(KGR::Key::Num3)) {
				auto& u = getIdUi(ui, 2, registry);
				u.SetPos({ 960, 890 });
				auto& u1 = getIdUi(ui, 1, registry);
				u1.SetPos({ 915, 900 });
				auto& u2 = getIdUi(ui, 3, registry);
				u2.SetPos({ 1005, 900 });
				auto& u3 = getIdUi(ui, 0, registry);
				u3.SetPos({ 870, 900 });
			}
			if (input->IsKeyDown(KGR::Key::Num4)) {
				auto& u = getIdUi(ui, 3, registry);
				u.SetPos({ 1005, 890 });
				auto& u1 = getIdUi(ui, 1, registry);
				u1.SetPos({ 915, 900 });
				auto& u2 = getIdUi(ui, 2, registry);
				u2.SetPos({ 960, 900 });
				auto& u3 = getIdUi(ui, 0, registry);
				u3.SetPos({ 870, 900 });
			}
			if (input->IsKeyDown(KGR::Key::Num5)) {
				auto& u = getIdUi(ui, 3, registry);
				u.SetPos({ 1005, 900 });
				auto& u1 = getIdUi(ui, 1, registry);
				u1.SetPos({ 915, 900 });
				auto& u2 = getIdUi(ui, 2, registry);
				u2.SetPos({ 960, 900 });
				auto& u3 = getIdUi(ui, 0, registry);
				u3.SetPos({ 870, 900 });
			}

			std::uint64_t nearestCreature{};
			bool creatureInRange = CaptureSystem::FindNearest(registry, cameraEntity, captureRange, nearestCreature);

			if (input->IsKeyDown(KGR::Key::R) && creatureInRange)
			{
				captureTimer += dt;
				if (captureTimer >= captureDuration)
				{
					CaptureSystem::Capture(registry, cameraEntity, nearestCreature);
					captureTimer = 0.0f;
				}
			}
			else
			{
				captureTimer = 0.0f;
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

			if (input->IsKeyPressed(KGR::Key::T))
			{
				auto& inv = registry.GetComponent<InventoryComponent>(cameraEntity);
				if (!inv.captured.empty())
				{
					CreatureData releasedData = inv.captured.back().data;
					glm::vec3 releasePos = registry.GetComponent<TransformComponent>(cameraEntity).GetPosition() + cameraFront * 2.0f;
					inv.ReleaseLast();
					if (glm::length(moveDir) > 0.0f)
						moveDir = glm::normalize(moveDir);

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

					if (input->IsKeyDown(KGR::SpecialKey::Ctrl))
					{
						player->startCrouching();
					}
					if (input->IsKeyReleased(KGR::SpecialKey::Ctrl))
					{
						player->stopCrouching();
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
					relAi.Init(&relTr, playerTr, { releasePos, {5,0,0}, {-5,0,3}, {3,0,-5} });
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

		{
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
				//for the hand to follow we will have to get the cams infos
				glm::quat camRot = transformCamera.GetOrientation();
				auto camPos = transformCamera.GetPosition();

				glm::vec3 right = camRot * glm::vec3(1.0f, 0.0f, 0.0f);
				glm::vec3 up = camRot * glm::vec3(0.0f, 1.0f, 0.0f);
				glm::vec3 forward = camRot * glm::vec3(0.0f, 0.0f, -1.0f);

				glm::vec3 localOffset = { 0.35f, 0.15f, 0.1f };

				auto& lanternTransform = registry.GetComponent<TransformComponent>(lanternEntity);

				glm::vec3 lanternPos =
					camPos +
					right * localOffset.x +
					up * localOffset.y +
					forward * localOffset.z;

				lanternTransform->SetPosition(lanternPos);
				lanternTransform->SetOrientation(camRot);
			}

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
		window->Render({ 0.53f, 0.81f, 0.92f, 1.0f });
	}
	window->Destroy();
	KGR::RenderWindow::End();
}