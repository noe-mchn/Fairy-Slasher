#pragma once

#define SOULS
#define FAIRY
#define CREATURE

class souls {
public:
	souls() = default;
	~souls() = default;
	void update() {

	}

private:
	int respirationTime = 5;
	int infusionTime = 10;
	const char *lootname = "souls Essences";
	int lootChance = 60;
};

class fairy {
public:
	fairy() = default;
	~fairy() = default;
	void update() {
	}
private:
	int respirationTime = 20;
	int infusionTime = 20;
	const char* lootname = "Fairy Wings";
	int lootChance = 40;
};

class creature { //a définir qu'elle créature c'est, ca peut etre un 3eme type d'ennemi ou une version plus rare d'un des deux autres
public:
	creature() = default;
	~creature() = default;
	void update() {
	}

private:
	int respirationTime = 30;
	int infusionTime = 30;
	const char* lootname = "Creature Claws";
	int lootChance = 20;
};