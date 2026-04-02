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
		// a camera needs a CameraComponent (orthographic or perspective) and a transform
		CameraComponent cam = CameraComponent::Create(glm::radians(45.0f),window->GetSize().x,window->GetSize().y,0.01f,100.0f,CameraComponent::Type::Perspective);
		TransformComponent transform;
		transform.SetPosition({ 0,3,5 });
		transform.LookAt({ 0,0,0 });
		cameraEntity = registry.CreateEntity();

		registry.AddComponents(cameraEntity, std::move(cam), std::move(transform), InventoryComponent{});
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
		transform.LookAtDir({ -0.2f,-1,- 0.3f });
		auto e = registry.CreateEntity();
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
		UiComponent ui({1920,1080},UiComponent::Anchor::LeftTop);
		// here set the position in the virtual resolution
		ui.SetPos({ 0, 0 });
		// here the scale
		ui.SetScale({ 200,200 });
		// create a texture but be aware that only the first texture in the component will be use 
		TextureComponent texture;
		texture.texture =  &TextureLoader::Load("Textures/texture.jpg", window->App());
		
		// same as always 
		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(transform), std::move(ui),std::move(texture), std::move(CollisionComp2d{}));

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

	{
		TransformComponent* playerTransform = nullptr;
		{
			auto es = registry.GetAllComponentsView<MeshComponent, TransformComponent>();
			for (auto& e : es)
			{
				if (!registry.HasComponent<CreatureAIComponent>(e))
				{
					playerTransform = &registry.GetComponent<TransformComponent>(e);
					break;
				}
			}
		}

		auto es = registry.GetAllComponentsView<CreatureAIComponent, TransformComponent>();
		for (auto& e : es)
		{
			auto& ai = registry.GetComponent<CreatureAIComponent>(e);
			auto& tr = registry.GetComponent<TransformComponent>(e);
			ai.Init(&tr, playerTransform, { {5,0,0}, {-5,0,3}, {3,0,-5}, {0,0,0} });
		}
	}

	// FPS camera state
	float yaw   = -90.0f;
	float pitch = -30.0f;
	float mouseSensitivity = 0.1f;
	float moveSpeed = 5.0f;
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
	glm::vec3 cameraRight = glm::vec3(1.0f, 0.0f,  0.0f);
	glm::vec3 worldUp     = glm::vec3(0.0f, 1.0f,  0.0f);

	// Capture state
	float captureTimer    = 0.0f;
	float captureDuration = 1.5f;
	float captureRange    = 2.5f;

	// Compute initial camera vectors from yaw/pitch
	{
		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);
		cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
		cameraUp    = glm::normalize(glm::cross(cameraRight, cameraFront));
	}

	float current = 0.0f;
	KGR::Tools::Chrono<float> chrono;
	while (!window->ShouldClose())
	{
		float actual = chrono.GetElapsedTime().AsSeconds();
		float dt = actual - current;
		current = actual;
		KGR::RenderWindow::PollEvent();
		window->Update();
		// FPS camera: mouse look + ZQSD movement
		{
			auto input = window->GetInputManager();

			// Mouse look
			glm::vec2 mouseDelta = input->GetMouseDelta();
			yaw   += mouseDelta.x * mouseSensitivity;
			pitch += -mouseDelta.y * mouseSensitivity;
			pitch  = std::clamp(pitch, -89.0f, 89.0f);

			// Recompute camera direction vectors
			glm::vec3 front;
			front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			front.y = sin(glm::radians(pitch));
			front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
			cameraFront = glm::normalize(front);
			cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
			cameraUp    = glm::normalize(glm::cross(cameraRight, cameraFront));

			// ZQSD movement on the XZ plane
			auto& camTransform = registry.GetComponent<TransformComponent>(cameraEntity);
			glm::vec3 pos = camTransform.GetPosition();

			glm::vec3 flatFront = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
			glm::vec3 flatRight = glm::normalize(glm::cross(flatFront, worldUp));

			if (input->IsKeyDown(KGR::Key::Z))
				pos += flatFront * moveSpeed * dt;
			if (input->IsKeyDown(KGR::Key::S))
				pos -= flatFront * moveSpeed * dt;
			if (input->IsKeyDown(KGR::Key::Q))
				pos -= flatRight * moveSpeed * dt;
			if (input->IsKeyDown(KGR::Key::D))
				pos += flatRight * moveSpeed * dt;
			if (input->IsKeyDown(KGR::SpecialKey::Space))
				pos.y += moveSpeed * dt;
			if (input->IsKeyDown(KGR::SpecialKey::Shift))
				pos.y -= moveSpeed * dt;

			camTransform.SetPosition(pos);
			camTransform.LookAtDir(cameraFront);

			// Capture: hold R near a creature for captureDuration seconds
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

			// Release last captured creature in front of the camera (press T)
			if (input->IsKeyPressed(KGR::Key::T))
			{
				auto& inv = registry.GetComponent<InventoryComponent>(cameraEntity);
				if (!inv.captured.empty())
				{
					CreatureData releasedData = inv.captured.back().data;
					glm::vec3 releasePos = registry.GetComponent<TransformComponent>(cameraEntity).GetPosition() + cameraFront * 2.0f;
					inv.ReleaseLast();

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
		}

		{

			auto mousePos = window.get()->GetInputManager()->GetMousePosition();
			float aspectRatio = static_cast<float>(window->GetSize().x) / static_cast<float>(window->GetSize().y);
			auto mouseinAR = UiComponent::VrToNdc(mousePos, window->GetSize(), aspectRatio, false);

			auto es = registry.GetAllComponentsView<CollisionComp2d,UiComponent>();
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

		//Test Sound
		if(window->GetInputManager()->IsKeyPressed(KGR::Key::P))
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
			auto es = registry.GetAllComponentsView < TextureComponent, TransformComponent2d,UiComponent > ();
			for (auto& e : es)
				{
					auto transform = registry.GetComponent<TransformComponent2d>(e);
					auto ui = registry.GetComponent<UiComponent>(e);
					auto texture = registry.GetComponent<TextureComponent>(e);
					window->RegisterUi(ui,transform,texture);
				}
		}
		window->Render({ 0.53f, 0.81f, 0.92f, 1.0f });
	}

	window->Destroy();
	KGR::RenderWindow::End();
}