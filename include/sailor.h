#ifndef SAILOR_H
#define SAILOR_H

#include <Nooskewl_Wedge/map_entity.h>

class Sailor : public wedge::Map_Entity
{
public:
	Sailor(std::string name);
	Sailor(util::JSON::Node *json);
	~Sailor();

	std::string save();

	void draw(util::Point<float> draw_offset);

	void set_pivot(util::Point<float> pivot);
	void set_use_pivot(bool use);

protected:
	util::Point<float> pivot;
	bool use_pivot;
};

#endif // SAILOR_H
