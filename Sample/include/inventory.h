#include "Core/Transform2dComponent.h"


#include "Core/InputManager.h"
#include "ECS/Entities.h"
#include "ECS/Registry.h"

using ecsType = KGR::ECS::Registry<KGR::ECS::Entity::_64, 100>;

class Inventory {
public:
	Inventory() = default;
	~Inventory() = default;
	//void inventoryGestion(KGR::InputManager* input, std::vector<uint64_t> ui, ecsType registry);
	//UiComponent& getIdUi(std::vector<uint64_t> es, int id, ecsType registry);
};