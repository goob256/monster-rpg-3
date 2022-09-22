#include <Nooskewl_Wedge/area_game.h>

#include "general.h"
#include "globals.h"
#include "sailor_npc.h"

Sailor_NPC::Sailor_NPC(std::string name, std::string tag, std::string sprite_name, std::string dialogue_name) :
	wedge::NPC(name, tag, sprite_name, dialogue_name),
	use_pivot(false)
{
}

Sailor_NPC::Sailor_NPC(util::JSON::Node *json) :
	wedge::NPC(json),
	use_pivot(false)
{
}

Sailor_NPC::~Sailor_NPC()
{
	// NPC deletes sprite
}

std::string Sailor_NPC::save()
{
	std::string s;

	s += util::string_printf("\"type\": \"sailor_npc\",");

	s += wedge::NPC::save();

	return s;
}

void Sailor_NPC::draw(util::Point<float> draw_offset)
{
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
}

void Sailor_NPC::set_pivot(util::Point<float> pivot)
{
	this->pivot = pivot;
}

void Sailor_NPC::set_use_pivot(bool use)
{
	use_pivot = use;
}
