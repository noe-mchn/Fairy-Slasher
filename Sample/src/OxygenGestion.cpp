#include "OxygenGestion.h"
#include <iostream>
#include <vector>
#include "Core/UiComponent.h"
OxygenGestion::OxygenGestion()
{
	lifetime = 5000000.0f;
}

void OxygenGestion::resetOxygen() {
	lifetime = 5000000.0f;
}

void OxygenGestion::update(float dt) 
{
    static int maxLifetime = 5000000;
    static auto frameTime = (maxLifetime / 17);
	index = lifetime / frameTime;
	lifetime -= 1.0f * dt;
	printf(" oxygen :%d\n", index);
}

int OxygenGestion::getIndex()
{
    return index;
}

void OxygenGestion::getActiveUiComponent(UiComponent& ui)
{
	auto currentIndex = (17 - index) + 4;
    if (ui.getId() == currentIndex)
		ui.ActivePrint();
}