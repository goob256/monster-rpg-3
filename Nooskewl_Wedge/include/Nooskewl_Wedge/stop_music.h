#ifndef NOOSKEWL_WEDGE_STOP_MUSIC_H
#define NOOSKEWL_WEDGE_STOP_MUSIC_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Stop_Music_Step : public Step
{
public:
	Stop_Music_Step(Task *task);
	virtual ~Stop_Music_Step();

	void start();
	bool run();
};

}

#endif // NOOSKEWL_WEDGE_STOP_MUSIC_H
