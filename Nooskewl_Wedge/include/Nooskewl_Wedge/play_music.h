#ifndef NOOSKEWL_WEDGE_PLAY_MUSIC_H
#define NOOSKEWL_WEDGE_PLAY_MUSIC_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Play_Music_Step : public Step
{
public:
	Play_Music_Step(std::string name, Task *task);
	virtual ~Play_Music_Step();

	void start();
	bool run();

private:
	std::string name;
};

}

#endif // NOOSKEWL_WEDGE_PLAY_MUSIC_H
