#ifndef NOOSKEWL_WEDGE_STOP_SOUND_H
#define NOOSKEWL_WEDGE_STOP_SOUND_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Stop_Sound_Step : public Step
{
public:
	Stop_Sound_Step(audio::Sound *sound, Task *task);
	virtual ~Stop_Sound_Step();

	bool run();

private:
	audio::Sound *sound;
};

}

#endif // NOOSKEWL_WEDGE_STOP_SOUND_H
