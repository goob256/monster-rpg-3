#ifndef NOOSKEWL_WEDGE_PAUSE_SPRITE_H
#define NOOSKEWL_WEDGE_PAUSE_SPRITE_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Pause_Sprite_Step : public Step
{
public:
	Pause_Sprite_Step(gfx::Sprite *sprite, bool paused, Task *task);
	virtual ~Pause_Sprite_Step();

	void start();
	bool run();

private:
	gfx::Sprite *sprite;
	bool paused;
};

}

#endif // NOOSKEWL_WEDGE_PAUSE_SPRITE_H
