#ifndef NOOSKEWL_WEDGE_PLAY_ANIMATION_H
#define NOOSKEWL_WEDGE_PLAY_ANIMATION_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Play_Animation_Step : public Step
{
public:
	Play_Animation_Step(gfx::Sprite *sprite, std::string anim_name, Task *task);
	virtual ~Play_Animation_Step();
	
	void start();
	bool run();

	void set_done(bool done);

private:
	gfx::Sprite *sprite;
	std::string anim_name;
	bool done;
};

}

#endif // NOOSKEWL_WEDGE_PLAY_ANIMATION_H
