#pragma once

class OxygenGestion
{
public:
	OxygenGestion();
	~OxygenGestion() = default;
	void update();
	void resetOxygen();

	int ExitStatus = 0; // 0 = alive, 1 = lose
private:
	float lifetime;
};


