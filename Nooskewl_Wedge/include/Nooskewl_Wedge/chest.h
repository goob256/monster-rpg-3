#ifndef NOOSKEWL_WEDGE_CHEST_H
#define NOOSKEWL_WEDGE_CHEST_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/inventory.h"
#include "Nooskewl_Wedge/map_entity.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Chest : public Map_Entity
{
public:
	Chest(std::string entity_name, std::string sprite_name, Object contents, int milestone = -1, Dialogue_Position dialogue_position = DIALOGUE_AUTO);
	Chest(std::string entity_name, std::string sprite_name, int gold, int milestone = -1, Dialogue_Position dialogue_position = DIALOGUE_AUTO);
	Chest(util::JSON::Node *json);
	virtual ~Chest();

	void activate(Map_Entity *activator);

	std::string save();

protected:
	bool open;
	Object contents;
	int gold;
	int milestone;
	Dialogue_Position dialogue_position;
};

}

#endif // NOOSKEWL_WEDGE_CHEST_H
