#ifndef SAILOR_NPC_H
#define SAILOR_NPC_H

#include <Nooskewl_Wedge/npc.h>

class Sailor_NPC : public wedge::NPC
{
public:
	Sailor_NPC(std::string name, std::string tag, std::string sprite_name, std::string dialogue_name);
	Sailor_NPC(util::JSON::Node *json);
	~Sailor_NPC();

	std::string save();

	void draw(util::Point<float> draw_offset);

	void set_pivot(util::Point<float> pivot);
	void set_use_pivot(bool use);

protected:
	util::Point<float> pivot;
	bool use_pivot;
};

#endif // SAILOR_NPC_H
