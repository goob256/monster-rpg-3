#include "Nooskewl_Wedge/pause_sprite.h"

using namespace wedge;

namespace wedge {

Pause_Sprite_Step::Pause_Sprite_Step(gfx::Sprite *sprite, bool paused, Task *task) :
	Step(task),
	sprite(sprite),
	paused(paused)
{
}

Pause_Sprite_Step::~Pause_Sprite_Step()
{
}

void Pause_Sprite_Step::start()
{
	if (paused) {
		sprite->stop();
	}
	else {
		sprite->start();
	}
}

bool Pause_Sprite_Step::run()
{
	send_done_signal();
	return false;
}

}
