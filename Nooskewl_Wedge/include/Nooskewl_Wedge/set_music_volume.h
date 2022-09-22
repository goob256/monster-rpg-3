#ifndef NOOSKEWL_WEDGE_SET_MUSIC_VOLUME_H
#define NOOSKEWL_WEDGE_SET_MUSIC_VOLUME_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Set_Music_Volume_Step : public Step
{
public:
	Set_Music_Volume_Step(float volume, Task *task);
	virtual ~Set_Music_Volume_Step();

	void start();
	bool run();

private:
	float volume;
};

}

#endif // NOOSKEWL_WEDGE_SET_MUSIC_VOLUME_H
