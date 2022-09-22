#ifndef NOOSKEWL_WEDGE_PLAY_SOUND_H
#define NOOSKEWL_WEDGE_PLAY_SOUND_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Play_Sound_Step : public Step
{
public:
	Play_Sound_Step(std::string name, Task *task, float volume = 1.0f);
	Play_Sound_Step(audio::Sound *sound, bool wait, bool loop, Task *task, float volume = 1.0f);
	virtual ~Play_Sound_Step();

	void start();
	bool run();
	
private:
	std::string name;
	bool destroy;
	bool wait;
	bool loop;
	audio::Sound *sound;
	float volume;
};

}

#endif // NOOSKEWL_WEDGE_PLAY_SOUND_H
