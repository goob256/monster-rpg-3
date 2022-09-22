#include <Nooskewl_Wedge/area_game.h>

#include "general.h"
#include "globals.h"
#include "sailor.h"

Sailor::Sailor(std::string name) :
	wedge::Map_Entity(name),
	use_pivot(false)
{
}

Sailor::Sailor(util::JSON::Node *json) :
	wedge::Map_Entity(json),
	use_pivot(false)
{
}

Sailor::~Sailor()
{
	// Map_Entity deletes sprite
}

std::string Sailor::save()
{
	std::string s;

	s += util::string_printf("\"type\": \"sailor\",");

	s += Map_Entity::save();

	return s;
}

void Sailor::draw(util::Point<float> draw_offset)
{
	if (visible == false) {
		return;
	}

	util::Point<float> p = (offset + pos) * shim::tile_size;
	gfx::Image *image = sprite->get_current_image();

	p.y -= (image->size.h - shim::tile_size);

	if (use_pivot) {
		float angle = get_ship_angle();

		util::Point<float> centre(pivot.x - p.x, pivot.y - p.y);

		image->draw_rotated(centre, draw_offset+pivot, angle);
	}
	else {
		image->draw(draw_offset+p);
	}

	int index = -1;
	std::vector<wedge::Map_Entity *> players = AREA->get_players();
	for (size_t i = 0; i < players.size(); i++) {
		if (players[i] == this) {
			index = (int)i;
			break;
		}
	}
	if (index >= 0 && INSTANCE->stats[index].base.status != wedge::STATUS_OK) {
		if (INSTANCE->stats[index].base.status == wedge::STATUS_POISONED) {
			gfx::Image *poison_image = GLOBALS->poison_sprite->get_current_image();
			util::Point<int> topleft, bottomright;
			sprite->get_bounds(topleft, bottomright);
			util::Point<int> sz = bottomright - topleft;
			poison_image->draw(draw_offset+p+topleft+sz/2-poison_image->size/2);
		}
		else {
			GLOBALS->draw_custom_status(this, INSTANCE->stats[index].base.status, draw_offset+p);
		}
	}
}

void Sailor::set_pivot(util::Point<float> pivot)
{
	this->pivot = pivot;
}

void Sailor::set_use_pivot(bool use)
{
	use_pivot = use;
}
