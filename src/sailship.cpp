#include "general.h"
#include "globals.h"
#include "sailship.h"

Sail_Ship::Sail_Ship(std::string name) :
	wedge::Map_Entity(name)
{
}

Sail_Ship::Sail_Ship(util::JSON::Node *json) :
	wedge::Map_Entity(json)
{
}

Sail_Ship::~Sail_Ship()
{
	// Map_Entity deletes sprite
}

std::string Sail_Ship::save()
{
	std::string s;

	s += util::string_printf("\"type\": \"sailship\",");

	s += Map_Entity::save();

	return s;
}

void Sail_Ship::draw(util::Point<float> draw_offset)
{
	util::Point<float> p = (offset + pos) * shim::tile_size;
	gfx::Image *image = sprite->get_current_image();

	p.y -= (image->size.h - shim::tile_size);

	float angle = get_ship_angle();

	util::Point<float> pivot(image->size.w/2.0f, image->size.h * 4.0f / 5.0f);

	image->draw_rotated(pivot, draw_offset+p+pivot, angle);
}
