#include <iostream>

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

#include "AI/CreatureData.h"
#include "AI/CreatureAIComponent.h"
#include "AI/AISystem.h"
#include <PlayerComponent.h>

// make you ecs type with entity 8 / 16 / 32 / 64 and the size of allocation between 1 and infinity
using ecsType = KGR::ECS::Registry<KGR::ECS::Entity::_64, 100>;

int main(int argc, char** argv)
{

	// this part is due to the archi of the code to retrieve the folder resources
	std::filesystem::path exePath = argv[0];
	std::filesystem::path projectRoot = exePath.parent_path().parent_path().parent_path().parent_path().parent_path();


	// init the rendering system ( init glfw )
	KGR::RenderWindow::Init();
	// create your window with the size the name and the resources path
	std::unique_ptr<KGR::RenderWindow> window = std::make_unique<KGR::RenderWindow>(glm::vec2{ 1920,800 }, "test", projectRoot / "Ressources");

	// getInputManager retrieve our input system where you can have the mouse pos mouse delta key pressed ... and set the cursor mode 
	window->GetInputManager()->SetMode(GLFW_CURSOR_NORMAL);

	// create your ecs 
	ecsType registry = ecsType{};


	// This is how to use the sounds and music system
	// and place it somewhere in the code where you want to use it

	// TODO when all test ok move this into a proper place 

	//MUSICS
	KGR::Audio::WavStreamComponent::Init(projectRoot / "Ressources");

	KGR::Audio::WavStreamComponent music;
	music.SetWav(KGR::Audio::WavStreamManager::Load("Musics/test.mp3"));
	music.SetVolume(10.0f);

	//SOUNDS
	KGR::Audio::WavComponent::Init(projectRoot / "Ressources");

	KGR::Audio::WavComponent sound;
	sound.SetWav(KGR::Audio::WavManager::Load("Sounds/sound.mp3"));
	sound.SetVolume(10.0f);


	// music test do not mind

	// camera 
	{
		// a camera need a cameraComponent that can be orthographic or perspective and a transform

		// create the camera with the fov , the size of the window (must be updated ) and the far and near rendering and the mode 
		CameraComponent cam = CameraComponent::Create(glm::radians(45.0f), window->GetSize().x, window->GetSize().y, 0.01f, 100.0f, CameraComponent::Type::Perspective);
		TransformComponent transform;
		// create a transform and set pos and dir 
		transform.SetPosition({ 0,3,5 });
		transform.LookAt({ 0,0,0 });
		// now create an entity , an alias here std::uint64_t
		auto e = registry.CreateEntity();

		// now move the component into the ecs
		registry.AddComponents(e, std::move(cam), std::move(transform));
	}

	// mesh
	{
		// a mesh need a meshComponent a transform and a texture 

		// create a mesh and load it with the cash loader
		MeshComponent mesh;
		mesh.mesh = &MeshLoader::Load("Models/cube.obj", window->App());

		// create a texture 
		MaterialComponent text;
		// allocate the size of the texture must be the same as the number of submeshes 
		text.materials.resize(mesh.mesh->GetSubMeshesCount());
		// then fill the texture ( this system need to be refact but for now you need to do it like that
		for (int i = 0; i < mesh.mesh->GetSubMeshesCount(); ++i)
		{
			Material mat;
			mat.baseColor = &TextureLoader::Load("Textures/test_mat_bc.png", window->App());

			text.materials[i] = mat;
		}

		// create the transform and set all the data
		TransformComponent transform;
		transform.SetPosition({ 0,0,0 });
		transform.SetScale({ 2.0f, 1.0f,3.0f });
		// same create an entity / id
		auto e = registry.CreateEntity();
		// fill the component
		registry.AddComponents(e, std::move(mesh), std::move(text), std::move(transform));
	}

	// light
	{
		// the light need transform component and light component
		// all lights type have their own system to create them go in the file to understand
		LightComponent<LightData::Type::Spot> lc = LightComponent<LightData::Type::Spot>::Create({ 1, 1,1 }, { 1,1,1 }, 10.0f, 100.0f, glm::radians(15.0f), 0.15f);
		// set the transform but certain light need dir some position or both so just use what necessary 
		TransformComponent transform;
		transform.SetPosition({ 0,5,0 });
		transform.LookAtDir({ 0,-1,0 });
		// same 
		auto e = registry.CreateEntity();
		// same
		registry.AddComponents(e, std::move(lc), std::move(transform));
	}

	// ui ( not fully operational)
	{
		// you need texture transform and ui component
		// for the transform it only use for the rotation 
		TransformComponent2d transform;
		// here you can set a rotation ( ROTATION FROM THE CENTER OF THE MESH )
		//transform.SetRotation(glm::radians(-45.0f));
		// create your ui with a virtual resolution and an anchor default center
		UiComponent ui({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		// here set the position in the virtual resolution
		ui.SetPos({ 0, 0 });
		// here the scale
		ui.SetScale({ 200,200 });
		// create a texture but be aware that only the first texture in the component will be use 
		TextureComponent texture;
		texture.texture = &TextureLoader::Load("Textures/texture.jpg", window->App());

		// same as always 
		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(transform), std::move(ui), std::move(texture), std::move(CollisionComp2d{}));

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

	auto es = registry.GetAllComponentsView<CreatureAIComponent, TransformComponent>();
	for (auto& e : es)
	{
		auto& ai = registry.GetComponent<CreatureAIComponent>(e);
		auto& tr = registry.GetComponent<TransformComponent>(e);
		ai.Init(&tr, player, { {5,0,0}, {-5,0,3}, {3,0,-5}, {0,0,0} });
	}


	float current = 0.0f;
	KGR::Tools::Chrono<float> chrono;
	float timer = 0.0f;


	static float mouseSensitivity = 0.0025f;
	float yaw = 0.0f;
	float pitch = glm::radians(-10.0f);

	//get camera
	auto mainCamera = registry.GetAllComponentsView<CameraComponent, TransformComponent>().begin()[0];
	auto& transformCamera = registry.GetComponent<TransformComponent>(mainCamera);

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

			//this comment is to check camera rotation when there's no visual element
			/*if (mouseX != 0.0f || mouseY != 0.0f)
			{
				std::cout << "CamRotX = " << transformCamera.GetRotation().x
					<< " CamRotY = " << transformCamera.GetRotation().y
					<< " CamRotZ = " << transformCamera.GetRotation().z << "\n";
			}*/

			if (input->IsKeyDown(KGR::SpecialKey::Shift))
			{
				player->startRunning();
			}
			if (input->IsKeyReleased(KGR::SpecialKey::Shift))
			{
				player->stopRunning();
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

			if (input->IsKeyDown(KGR::SpecialKey::Ctrl))
			{
				player->startCrouching();
			}
			if (input->IsKeyReleased(KGR::SpecialKey::Ctrl))
			{
				player->stopCrouching();
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
			}
			{
				auto es = registry.GetAllComponentsView<CameraComponent, TransformComponent>();
				if (es.Size() != 1)
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
	}

	window->Destroy();
	KGR::RenderWindow::End();

}