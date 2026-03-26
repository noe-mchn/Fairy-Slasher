#pragma once
#include "Enemi.h"

enum button {
	creature,
	fairy,
	souls,
};

class inventory {
public: 
	inventory() = default;
	~inventory() = default;

	void update() {
		if (true /*check where the click are and what function we select*/)
			return;
	};

	void creatureGes() {
		if (nb_creatures > 0)
			return;
	};

	void fairyGes() {
		if (nb_fairy_two > 0)
			return;
	};

	void soulsGes() {
		if (nb_souls > 0)
			return;
	};
	int nb_creatures = 0;
	int nb_fairy_two = 0;
	int nb_souls = 0;
};