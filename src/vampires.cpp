#include "vampires.h"

void get_vampire_cost(std::string name, int &hp, int &mp)
{
	if (name == "vBolt") {
		hp = 50;
		mp = 10;
	}
	else if (name == "vIce") {
		hp = 200;
		mp = 50;
	}
	else if (name == "vFire") {
		hp = 9999;
		mp = 9999;
	}
	else {
		hp = mp = -1;
	}
}
