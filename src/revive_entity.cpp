#include "globals.h"
#include "inventory.h"
#include "revive_entity.h"

Revive_Entity::Revive_Entity() :
	wedge::Map_Entity("revive")
{
}

Revive_Entity::Revive_Entity(util::JSON::Node *json) :
	wedge::Map_Entity(json)
{
}

Revive_Entity::~Revive_Entity()
{
}

void Revive_Entity::activate(Map_Entity *activator)
{
	audio::MML *mml = dynamic_cast<audio::MML *>(GLOBALS->item_sfx[ITEM_HOLY_WATER]);
	if (mml == NULL || mml->is_playing() == false) {
		GLOBALS->item_sfx[ITEM_HOLY_WATER]->play(false);
	}
	for (size_t i = 0; i < INSTANCE->stats.size(); i++) {
		INSTANCE->stats[i].base.status = wedge::STATUS_OK;
		INSTANCE->stats[i].base.hp = INSTANCE->stats[i].base.fixed.max_hp;
		INSTANCE->stats[i].base.mp = INSTANCE->stats[i].base.fixed.max_mp;
	}
}

std::string Revive_Entity::save()
{
	std::string s;

	s += util::string_printf("\"type\": \"revive\",");

	s += Map_Entity::save();

	return s;
}
