#include "Nooskewl_Wedge/play_animation.h"

using namespace wedge;

static void callback(void *data)
{
	Play_Animation_Step *step = static_cast<Play_Animation_Step *>(data);
	step->set_done(true);
}

namespace wedge {

Play_Animation_Step::Play_Animation_Step(gfx::Sprite *sprite, std::string anim_name, Task *task) :
	Step(task),
	sprite(sprite),
	anim_name(anim_name),
	done(false)
{
}

Play_Animation_Step::~Play_Animation_Step()
{
}

void Play_Animation_Step::start()
{
	sprite->set_animation(anim_name, callback, this);
	sprite->reset();
	sprite->start();
}

bool Play_Animation_Step::run()
{
	return !done;
}

void Play_Animation_Step::set_done(bool done)
{
	this->done = done;
	if (done) {
		send_done_signal();
	}
}

}
