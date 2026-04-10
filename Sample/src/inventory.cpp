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

#include "../include/inventory.h"
//
//
//void Inventory::inventoryGestion(KGR::InputManager* input, std::vector<uint64_t> ui, KGR::ECS::Registry<KGR::ECS::Entity::_64, 100> registry)
//{
//	if (input->IsKeyDown(KGR::Key::Num1)) {
//		auto& u = getIdUi(ui, 0, registry);
//		printf("num 1 pressed \n");
//		u.SetPos({ 870, 890 });
//	}
//	if (input->IsKeyDown(KGR::Key::Num2)) {
//		auto& u = getIdUi(ui, 1, registry);
//		printf("num 2 pressed \n");
//		u.SetPos({ 915, 890 });
//	}
//	if (input->IsKeyDown(KGR::Key::Num3)) {
//		auto& u = getIdUi(ui, 2, registry);
//		printf("num 3 pressed \n");
//
//		u.SetPos({ 960, 890 });
//	}
//	if (input->IsKeyDown(KGR::Key::Num4)) {
//		auto& u = getIdUi(ui, 3, registry);
//		printf("num 4 pressed \n");
//
//		u.SetPos({ 1005, 890 });
//	}
//}