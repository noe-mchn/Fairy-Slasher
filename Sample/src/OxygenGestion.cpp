#include "OxygenGestion.h"
#include <iostream>

OxygenGestion::OxygenGestion()
{
	lifetime = 500.0f;
}

void OxygenGestion::resetOxygen() {
	lifetime = 500.0f;
}

void OxygenGestion::update() 
{
	if (lifetime <= 0.0f) {
		ExitStatus = -1;
		return;
	}
	lifetime -= 1.0f;
	printf("Oxygen lifetime: %f\n", lifetime);
	if (lifetime <= 240.0f) {
		//pass at 80 seconds of respiration change ui print.
	}
	if (lifetime <= 180.0f) {
		//pass at 60 seconds of respiration change ui print.
	}
	if (lifetime <= 120.0f) {
		//pass at 40 seconds of respiration change ui print.
	}
	if (lifetime <= 60.0f) {
		//pass at 20 seconds of respiration change ui print.
	}
}