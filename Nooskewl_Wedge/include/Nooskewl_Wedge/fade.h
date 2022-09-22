#ifndef NOOSKEWL_WEDGE_FADE_H
#define NOOSKEWL_WEDGE_FADE_H

#include "Nooskewl_Wedge/main.h"
#include "Nooskewl_Wedge/systems.h"

namespace wedge {

class NOOSKEWL_WEDGE_EXPORT Fade_Step : public Step
{
public:
	Fade_Step(SDL_Colour colour, bool out, int length/*ms*/, Task *task); // in or out
	virtual ~Fade_Step();
	
	bool run();
	void start();

protected:
	SDL_Colour colour;
	bool out;
	int length;
	Uint32 start_time;
};

}

#endif // NOOSKEWL_WEDGE_FADE_H
