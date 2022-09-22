#include "Nooskewl_Wedge/map_entity.h"
#include "Nooskewl_Wedge/play_music.h"

using namespace wedge;

namespace wedge {

Play_Music_Step::Play_Music_Step(std::string name, Task *task) :
	Step(task),
	name(name)
{
}

Play_Music_Step::~Play_Music_Step()
{
}

void Play_Music_Step::start()
{
	audio::play_music(name);
}

bool Play_Music_Step::run()
{
	send_done_signal();
	return false;
}

}
