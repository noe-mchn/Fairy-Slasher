#include "OxygenGestion.h"
#include <iostream>
#include <vector>
#include "Core/UiComponent.h"
OxygenGestion::OxygenGestion()
{
	lifetime = 700.0f;
}

void OxygenGestion::resetOxygen() {
	lifetime = 700.0f;
}

void OxygenGestion::update() 
{
    static int maxLifetime = 700;
    static auto frameTime = (maxLifetime / 17);
	index = lifetime / frameTime;
	lifetime -= 1.0f;
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