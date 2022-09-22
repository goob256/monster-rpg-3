#ifndef SAILSHIP_H
#define SAILSHIP_H

#include <Nooskewl_Wedge/map_entity.h>

class Sail_Ship : public wedge::Map_Entity
{
public:
	Sail_Ship(std::string name);
	Sail_Ship(util::JSON::Node *json);
	~Sail_Ship();

	std::string save();

	void draw(util::Point<float> draw_offset);
};

#endif // SAILSHIP_H
