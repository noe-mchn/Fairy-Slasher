#include "OxygenGestion.h"
#include <iostream>
#include <vector>
#include "Core/UiComponent.h"
OxygenGestion::OxygenGestion()
{
	lifetime = 50000;
}

void OxygenGestion::resetOxygen() {
	lifetime = 50000;
}

void OxygenGestion::update(float dt) 
{
	static int maxLifetime = 50000;
	static auto frameTime = (maxLifetime / 17);
	index = lifetime / frameTime;
	lifetime -= 1.0f * dt;
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