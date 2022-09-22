#include "Nooskewl_Wedge/map_entity.h"
#include "Nooskewl_Wedge/set_music_volume.h"

using namespace wedge;

namespace wedge {

Set_Music_Volume_Step::Set_Music_Volume_Step(float volume, Task *task) :
	Step(task),
	volume(volume)
{
}

Set_Music_Volume_Step::~Set_Music_Volume_Step()
{
}

void Set_Music_Volume_Step::start()
{
	if (shim::music) {
		shim::music->set_master_volume(volume);
	}
}

bool Set_Music_Volume_Step::run()
{
	send_done_signal();
	return false;
}

}
