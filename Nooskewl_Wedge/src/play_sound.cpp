#include "Nooskewl_Wedge/play_sound.h"

using namespace wedge;

namespace wedge {

Play_Sound_Step::Play_Sound_Step(std::string name, Task *task, float volume) :
	Step(task),
	name(name),
	destroy(true),
	wait(true),
	loop(false),
	sound(NULL),
	volume(volume)
{
}

Play_Sound_Step::Play_Sound_Step(audio::Sound *sound, bool wait, bool loop, Task *task, float volume) :
	Step(task),
	destroy(false),
	wait(wait),
	loop(loop),
	sound(sound),
	volume(volume)
{
}

Play_Sound_Step::~Play_Sound_Step()
{
	if (destroy) {
		delete sound;
	}
}

void Play_Sound_Step::start()
{
	if (sound == NULL && name != "") {
		if (name.find(".mml") != std::string::npos) {
			sound = new audio::MML(name);
		}
		else {
			sound = new audio::Sample(name);
		}
	}
	sound->play(volume*shim::sfx_volume, loop);
}

bool Play_Sound_Step::run()
{
	if (wait == false || sound == NULL || sound->is_done()) {
		send_done_signal();
		return false;
	}
	return true;
}

}
