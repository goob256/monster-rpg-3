#ifndef REVIVE_ENTITY_H
#define REVIVE_ENTITY_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/map_entity.h>

class Revive_Entity : public wedge::Map_Entity
{
public:
	Revive_Entity();
	Revive_Entity(util::JSON::Node *json);
	~Revive_Entity();

	void activate(wedge::Map_Entity *activator);

	std::string save();
};

#endif // REVIVE_ENTITY_H
