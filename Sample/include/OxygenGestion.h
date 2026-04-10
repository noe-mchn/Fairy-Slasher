#pragma once
#include <Core/UiComponent.h>

class OxygenGestion
{
public:
	OxygenGestion();
	~OxygenGestion() = default;
	void update(float dt);
	void resetOxygen();
	int getIndex();
	void getActiveUiComponent(UiComponent& ui);
	int getLifetime() { return lifetime; }
	void ActiveEndScreen(UiComponent& ui) {
		if (ui.getId() == 22)
			ui.ActivePrint();
	}
private:
	int lifetime;
	int index;
};


