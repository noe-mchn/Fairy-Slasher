#pragma once
#include <Core/UiComponent.h>

class OxygenGestion
{
public:
	OxygenGestion();
	~OxygenGestion() = default;
	void update();
	void resetOxygen();
	int getIndex();
	void getActiveUiComponent(UiComponent& ui);
	int getLifetime() { return lifetime; }

private:
	int lifetime;
	int index;
};


