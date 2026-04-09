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
#include "OxygenGestion.h"
#include "CauldronComponent.h"
#include "CauldronSystem.h"
#include "LootInventoryComponent.h"
#include "PlayerUpgradeComponent.h"
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

		registry.AddComponents(cameraEntity, std::move(cam), std::move(transform), InventoryComponent{}, LootInventoryComponent{}, PlayerUpgradeComponent{});
	}

	// lantern (held by player, attached to camera)
	//std::uint64_t lanternEntity = 0;
	//{
	//	MeshComponent mesh;
	//	mesh.mesh = &MeshLoader::Load("Models/lanterne.obj", window->App());

	//	MaterialComponent mat;
	//	mat.materials.resize(mesh.mesh->GetSubMeshesCount());
	//	for (int i = 0; i < mesh.mesh->GetSubMeshesCount(); ++i)
	//	{
	//		Material m;
	//		m.baseColor = &TextureLoader::Load("Textures/test_mat_bc.png", window->App());
	//		mat.materials[i] = m;
	//	}

	//	TransformComponent transform;
	//	transform.SetScale({ 0.15f, 0.15f, 0.15f });

	//	lanternEntity = registry.CreateEntity();
	//	registry.AddComponents(lanternEntity, std::move(mesh), std::move(mat), std::move(transform));
	//}

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

		//instanciation for the life bar
		TransformComponent2d LifeBar0;
		TransformComponent2d LifeBar1;
		TransformComponent2d LifeBar2;
		TransformComponent2d LifeBar3;
		TransformComponent2d LifeBar4;
		TransformComponent2d LifeBar5;
		TransformComponent2d LifeBar6;
		TransformComponent2d LifeBar7;
		TransformComponent2d LifeBar8;
		TransformComponent2d LifeBar9;
		TransformComponent2d LifeBar10;
		TransformComponent2d LifeBar11;
		TransformComponent2d LifeBar12;
		TransformComponent2d LifeBar13;
		TransformComponent2d LifeBar14;
		TransformComponent2d LifeBar15;
		TransformComponent2d LifeBar16;

		//TransformComponent2d LifeFullHeart;
		//TransformComponent2d LifeEmptyHeart;
		// here you can set a rotation ( ROTATION FROM THE CENTER OF THE MESH )
		//transform.SetRotation(glm::radians(-45.0f));
		// create your ui with a virtual resolution and an anchor default center
		UiComponent ui1({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent ui2({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent ui3({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent ui4({ 1920,1080 }, UiComponent::Anchor::LeftTop);

		//life bar components
		UiComponent LifeBarComp0({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent LifeBarComp1({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent LifeBarComp2({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent LifeBarComp3({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent LifeBarComp4({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent LifeBarComp5({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent LifeBarComp6({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent LifeBarComp7({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent LifeBarComp8({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent LifeBarComp9({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent LifeBarComp10({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent LifeBarComp11({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent LifeBarComp12({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent LifeBarComp13({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent LifeBarComp14({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent LifeBarComp15({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		UiComponent LifeBarComp16({ 1920,1080 }, UiComponent::Anchor::LeftTop);

		//UiComponent FullHeartComp({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		//UiComponent EmptyHeartComp({ 1920,1080 }, UiComponent::Anchor::LeftTop);

		// here set the position in the virtual resolution
		ui1.SetPos({ 870, 900 });
		ui2.SetPos({ 960, 900 });
		ui3.SetPos({ 1040, 900 });
		ui4.SetPos({ 1115, 900 });

		//life bar components position
		LifeBarComp0.SetPos({ 10, 10 });
		LifeBarComp1.SetPos({ 10, 10 });
		LifeBarComp2.SetPos({ 10, 10 });
		LifeBarComp3.SetPos({ 10, 10 });
		LifeBarComp4.SetPos({ 10, 10 });
		LifeBarComp5.SetPos({ 10, 10 });
		LifeBarComp6.SetPos({ 10, 10 });
		LifeBarComp7.SetPos({ 10, 10 });
		LifeBarComp8.SetPos({ 10, 10 });
		LifeBarComp9.SetPos({ 10, 10 });
		LifeBarComp10.SetPos({ 10, 10 });
		LifeBarComp11.SetPos({ 10, 10 });
		LifeBarComp12.SetPos({ 10, 10 });
		LifeBarComp13.SetPos({ 10, 10 });
		LifeBarComp14.SetPos({ 10, 10 });
		LifeBarComp15.SetPos({ 10, 10 });
		LifeBarComp16.SetPos({ 10, 10 });

		
		//FullHeartComp.SetPos({ 0, 0});
		//EmptyHeartComp.SetPos({ 20, 0 });

		// here the scale
		ui1.SetScale({ 80,80 });
		ui2.SetScale({ 80,80 });
		ui3.SetScale({ 80,80 });
		ui4.SetScale({ 80,80 });

		//life bar components scale
		LifeBarComp0.SetScale({ 400, 100 });
		LifeBarComp1.SetScale({ 400, 100 });
		LifeBarComp2.SetScale({ 400, 100 });
		LifeBarComp3.SetScale({ 400, 100 });
		LifeBarComp4.SetScale({ 400, 100 });
		LifeBarComp5.SetScale({ 400, 100 });
		LifeBarComp6.SetScale({ 400, 100 });
		LifeBarComp7.SetScale({ 400, 100 });
		LifeBarComp8.SetScale({ 400, 100 });
		LifeBarComp9.SetScale({ 400, 100 });
		LifeBarComp10.SetScale({ 400, 100 });
		LifeBarComp11.SetScale({ 400, 100 });
		LifeBarComp12.SetScale({ 400, 100 });
		LifeBarComp13.SetScale({ 400, 100 });
		LifeBarComp14.SetScale({ 400, 100 });
		LifeBarComp15.SetScale({ 400, 100 });
		LifeBarComp16.SetScale({ 400, 100 });

		//FullHeartComp.SetScale({ 20, 20 });
		//EmptyHeartComp.SetScale({ 20, 20 });

		ui1.setId(0);
		ui2.setId(1);
		ui3.setId(2);
		ui4.setId(3);
		LifeBarComp0.setId(5);
		LifeBarComp1.setId(6);
		LifeBarComp2.setId(7);
		LifeBarComp3.setId(8);
		LifeBarComp4.setId(9);
		LifeBarComp5.setId(10);
		LifeBarComp6.setId(11);
		LifeBarComp7.setId(12);
		LifeBarComp8.setId(13);
		LifeBarComp9.setId(14);
		LifeBarComp10.setId(15);
		LifeBarComp11.setId(16);
		LifeBarComp12.setId(17);
		LifeBarComp13.setId(18);
		LifeBarComp14.setId(19);
		LifeBarComp15.setId(20);
		LifeBarComp16.setId(21);

		//FullHeartComp.setId(4);
		//EmptyHeartComp.setId(5);
		ui1.ActivePrint();
		ui2.ActivePrint();
		ui3.ActivePrint();
		ui4.ActivePrint();
		// create a texture but be aware that only the first texture in the component will be use 
		TextureComponent texture1;
		TextureComponent texture2;
		TextureComponent texture3;
		TextureComponent texture4;
		TextureComponent LifeBarTexture0;
		TextureComponent LifeBarTexture1;
		TextureComponent LifeBarTexture2;
		TextureComponent LifeBarTexture3;
		TextureComponent LifeBarTexture4;
		TextureComponent LifeBarTexture5;
		TextureComponent LifeBarTexture6;
		TextureComponent LifeBarTexture7;
		TextureComponent LifeBarTexture8;
		TextureComponent LifeBarTexture9;
		TextureComponent LifeBarTexture10;
		TextureComponent LifeBarTexture11;
		TextureComponent LifeBarTexture12;
		TextureComponent LifeBarTexture13;
		TextureComponent LifeBarTexture14;
		TextureComponent LifeBarTexture15;
		TextureComponent LifeBarTexture16;
		//TextureComponent LifeEmpHeartText;
		//TextureComponent LifefullHeartText;

		texture1.texture =  &TextureLoader::Load("Textures/inventory/inventoryslot.png", window->App());
		texture2.texture = &TextureLoader::Load("Textures/inventory/inventoryslot.png", window->App());
		texture3.texture = &TextureLoader::Load("Textures/inventory/inventoryslot.png", window->App());
		texture4.texture = &TextureLoader::Load("Textures/inventory/inventoryslot.png", window->App());
		LifeBarTexture0.texture = &TextureLoader::Load("Textures/ui/health_bar1.png", window->App());
		LifeBarTexture1.texture = &TextureLoader::Load("Textures/ui/health_bar2.png", window->App());
		LifeBarTexture2.texture = &TextureLoader::Load("Textures/ui/health_bar3.png", window->App());
		LifeBarTexture3.texture = &TextureLoader::Load("Textures/ui/health_bar4.png", window->App());
		LifeBarTexture4.texture = &TextureLoader::Load("Textures/ui/health_bar5.png", window->App());
		LifeBarTexture5.texture = &TextureLoader::Load("Textures/ui/health_bar6.png", window->App());
		LifeBarTexture6.texture = &TextureLoader::Load("Textures/ui/health_bar7.png", window->App());
		LifeBarTexture7.texture = &TextureLoader::Load("Textures/ui/health_bar8.png", window->App());
		LifeBarTexture8.texture = &TextureLoader::Load("Textures/ui/health_bar9.png", window->App());
		LifeBarTexture9.texture = &TextureLoader::Load("Textures/ui/health_bar10.png", window->App());
		LifeBarTexture10.texture = &TextureLoader::Load("Textures/ui/health_bar11.png", window->App());
		LifeBarTexture11.texture = &TextureLoader::Load("Textures/ui/health_bar12.png", window->App());
		LifeBarTexture12.texture = &TextureLoader::Load("Textures/ui/health_bar13.png", window->App());
		LifeBarTexture13.texture = &TextureLoader::Load("Textures/ui/health_bar14.png", window->App());
		LifeBarTexture14.texture = &TextureLoader::Load("Textures/ui/health_bar15.png", window->App());
		LifeBarTexture15.texture = &TextureLoader::Load("Textures/ui/health_bar16.png", window->App());
		LifeBarTexture16.texture = &TextureLoader::Load("Textures/ui/health_bar17.png", window->App());
		//LifeEmpHeartText.texture = &TextureLoader::Load("Textures/empty_heart", window->App());
		//LifefullHeartText.texture = &TextureLoader::Load("Textures/full_heart", window->App());

		
		auto e1 = registry.CreateEntity();
		auto e2 = registry.CreateEntity();
		auto e3 = registry.CreateEntity();
		auto e4 = registry.CreateEntity();
		auto eLifeBar0 = registry.CreateEntity();
		auto eLifeBar1 = registry.CreateEntity();
		auto eLifeBar2 = registry.CreateEntity();
		auto eLifeBar3 = registry.CreateEntity();
		auto eLifeBar4 = registry.CreateEntity();
		auto eLifeBar5 = registry.CreateEntity();
		auto eLifeBar6 = registry.CreateEntity();
		auto eLifeBar7 = registry.CreateEntity();
		auto eLifeBar8 = registry.CreateEntity();
		auto eLifeBar9 = registry.CreateEntity();
		auto eLifeBar10 = registry.CreateEntity();
		auto eLifeBar11 = registry.CreateEntity();
		auto eLifeBar12 = registry.CreateEntity();
		auto eLifeBar13 = registry.CreateEntity();
		auto eLifeBar14 = registry.CreateEntity();
		auto eLifeBar15 = registry.CreateEntity();
		auto eLifeBar16 = registry.CreateEntity();
		//auto emptyHeart = registry.CreateEntity();
		//auto fullHeart = registry.CreateEntity();

		registry.AddComponents(e1, std::move(transform1), std::move(ui1),std::move(texture1), std::move(CollisionComp2d{}));
		registry.AddComponents(e2, std::move(transform2), std::move(ui2), std::move(texture2), std::move(CollisionComp2d{}));
		registry.AddComponents(e3, std::move(transform3), std::move(ui3), std::move(texture3), std::move(CollisionComp2d{}));
		registry.AddComponents(e4, std::move(transform4), std::move(ui4), std::move(texture4), std::move(CollisionComp2d{}));
		registry.AddComponents(eLifeBar0, std::move(LifeBar0), std::move(LifeBarComp0), std::move(LifeBarTexture0), std::move(CollisionComp2d{}));
		registry.AddComponents(eLifeBar1, std::move(LifeBar1), std::move(LifeBarComp1), std::move(LifeBarTexture1), std::move(CollisionComp2d{}));
		registry.AddComponents(eLifeBar2, std::move(LifeBar2), std::move(LifeBarComp2), std::move(LifeBarTexture2), std::move(CollisionComp2d{}));
		registry.AddComponents(eLifeBar3, std::move(LifeBar3), std::move(LifeBarComp3), std::move(LifeBarTexture3), std::move(CollisionComp2d{}));
		registry.AddComponents(eLifeBar5, std::move(LifeBar5), std::move(LifeBarComp5), std::move(LifeBarTexture5), std::move(CollisionComp2d{}));
		registry.AddComponents(eLifeBar6, std::move(LifeBar6), std::move(LifeBarComp6), std::move(LifeBarTexture6), std::move(CollisionComp2d{}));
		registry.AddComponents(eLifeBar7, std::move(LifeBar7), std::move(LifeBarComp7), std::move(LifeBarTexture7), std::move(CollisionComp2d{}));
		registry.AddComponents(eLifeBar8, std::move(LifeBar8), std::move(LifeBarComp8), std::move(LifeBarTexture8), std::move(CollisionComp2d{}));
		registry.AddComponents(eLifeBar9, std::move(LifeBar9), std::move(LifeBarComp9), std::move(LifeBarTexture9), std::move(CollisionComp2d{}));
		registry.AddComponents(eLifeBar10, std::move(LifeBar10), std::move(LifeBarComp10), std::move(LifeBarTexture10), std::move(CollisionComp2d{}));
		registry.AddComponents(eLifeBar11, std::move(LifeBar11), std::move(LifeBarComp11), std::move(LifeBarTexture11), std::move(CollisionComp2d{}));
		registry.AddComponents(eLifeBar12, std::move(LifeBar12), std::move(LifeBarComp12), std::move(LifeBarTexture12), std::move(CollisionComp2d{}));
		registry.AddComponents(eLifeBar13, std::move(LifeBar13), std::move(LifeBarComp13), std::move(LifeBarTexture13), std::move(CollisionComp2d{}));
		registry.AddComponents(eLifeBar14, std::move(LifeBar14), std::move(LifeBarComp14), std::move(LifeBarTexture14), std::move(CollisionComp2d{}));
		registry.AddComponents(eLifeBar15, std::move(LifeBar15), std::move(LifeBarComp15), std::move(LifeBarTexture15), std::move(CollisionComp2d{}));
		registry.AddComponents(eLifeBar16, std::move(LifeBar16), std::move(LifeBarComp16), std::move(LifeBarTexture16), std::move(CollisionComp2d{}));

		//registry.AddComponents(emptyHeart, std::move(LifeFullHeart), std::move(FullHeartComp), std::move(LifefullHeartText), std::move(CollisionComp2d{}));
		//registry.AddComponents(fullHeart, std::move(LifeEmptyHeart), std::move(EmptyHeartComp), std::move(LifeEmpHeartText), std::move(CollisionComp2d{}));

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

	float yaw   = -90.0f;
	float pitch = -30.0f;
	float mouseSensitivity = 0.1f;
	float moveSpeed = 5.0f;
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
	glm::vec3 cameraRight = glm::vec3(1.0f, 0.0f,  0.0f);
	glm::vec3 worldUp     = glm::vec3(0.0f, 1.0f,  0.0f);

	float captureTimer    = 0.0f;
	float captureDuration = 1.5f;
	float captureRange    = 2.5f;

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
		cameraUp    = glm::normalize(glm::cross(cameraRight, cameraFront));
	}



	auto ui = registry.GetAllComponentsView<UiComponent>();
	float current = 0.0f;
	KGR::Tools::Chrono<float> chrono;
	OxygenGestion oxygenGestion;
	
	while (!window->ShouldClose())
	{
		float actual = chrono.GetElapsedTime().AsSeconds();
		float dt = actual - current;
		current = actual;
		KGR::RenderWindow::PollEvent();
		window->Update();
		oxygenGestion.update(dt);
		if (oxygenGestion.getLifetime() <= 0)
			break;
		for (auto& uiComp : ui) {
			auto& oui = registry.GetComponent<UiComponent>(uiComp);

			if (registry.GetComponent<UiComponent>(uiComp).GetStatePrint() != true)
				oxygenGestion.getActiveUiComponent(oui);
		}
		{
			auto input = window->GetInputManager();

			glm::vec2 mouseDelta = input->GetMouseDelta();
			yaw   += mouseDelta.x * mouseSensitivity;
			pitch += -mouseDelta.y * mouseSensitivity;
			pitch  = std::clamp(pitch, -89.0f, 89.0f);

			glm::vec3 front;
			front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			front.y = sin(glm::radians(pitch));
			front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
			cameraFront = glm::normalize(front);
			cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
			cameraUp    = glm::normalize(glm::cross(cameraRight, cameraFront));

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

			// slot selection (1-4)
			auto& inv = registry.GetComponent<InventoryComponent>(cameraEntity);
			if (input->IsKeyPressed(KGR::Key::Num1)) { inv.selectedSlot = 0; std::cout << "[INV] Slot 1 selected" << std::endl; }
			if (input->IsKeyPressed(KGR::Key::Num2)) { inv.selectedSlot = 1; std::cout << "[INV] Slot 2 selected" << std::endl; }
			if (input->IsKeyPressed(KGR::Key::Num3)) { inv.selectedSlot = 2; std::cout << "[INV] Slot 3 selected" << std::endl; }
			if (input->IsKeyPressed(KGR::Key::Num4)) { inv.selectedSlot = 3; std::cout << "[INV] Slot 4 selected" << std::endl; }

			// update UI to highlight selected slot
			{
				const glm::vec2 slotPositions[4] = { {800,900}, {885,900}, {965,900}, {1045,900} };
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

			camTransform.SetPosition(pos);
			camTransform.LookAtDir(cameraFront);

			//// update lantern to follow camera (FPS arm)
			//{
			//	auto& lanternTransform = registry.GetComponent<TransformComponent>(lanternEntity);
			//	glm::vec3 lanternPos = pos + cameraFront * 0.35f + cameraRight * 0.15f - cameraUp * 0.1f;
			//	lanternTransform.SetPosition(lanternPos);
			//	lanternTransform.LookAtDir(cameraFront);
			//}

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
				
				//if (t.aabb.IsColliding(mouseinAR))
				//	u.SetColor({ 1,0,0,1 });
				//else
				//	u.SetColor({ 0,1,0,1 });
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
					spAi.Init(&spTr, playerTr, { spawnPos, {5,0,0}, {-5,0,3}, {3,0,-5} });

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