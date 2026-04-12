#include "Core/Transform2dComponent.h"


#include "Core/InputManager.h"
#include "ECS/Entities.h"
#include "ECS/Registry.h"

using ecsType = KGR::ECS::Registry<KGR::ECS::Entity::_64, 100>;

class Inventory {
public:
	Inventory() = default;
	~Inventory() = default;
};