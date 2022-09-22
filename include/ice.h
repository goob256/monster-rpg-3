#ifndef ICE_H
#define ICE_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

class Ice_Step : public wedge::Step
{
public:
	Ice_Step(wedge::Task *task);
	virtual ~Ice_Step();
	
	bool run();
	void draw_fore();
	void start();

protected:
	static const int NUM_PARTICLES = 100;
	static const int MAX_DELAY = 3000;
	static const int MIN_LIFETIME = 1000;
	static const int MAX_LIFETIME = 2000;
	static const int MAX_OFFSET = 35;
	static const float MAX_ANGLE_INC;
	static const int MIN_FLIPS = 2;
	static const int MAX_FLIPS = 4;

	void run_ice(Uint32 now);
	bool is_alive(Uint32 now, int index);
	void update(int index);
	void draw_ice(Uint32 now, int index);
	void add_particle();
	float get_angle(Uint32 now, int index);
	float increase_mids(float f, int passes);
	float lessen_mids(float f, int passes);

	struct Ice_Particle {
		// Delay before start
		Uint32 delay;

		// Length of life (after delay) (affects speed)
		Uint32 lifetime;

		float angle;
		float angle_inc;

		util::Point<int> offset_start;
		util::Point<int> offset_end;

		SDL_Colour colour;

		float scale;

		int image_index;

		float angle_offset;

		int flips;
	};

	std::vector<Ice_Particle> ice;

	Uint32 start_time;
	Uint32 end_time;

	util::Point<int> ice_start;
	util::Point<int> ice_end;

	std::vector<gfx::Image *> images;
};

#endif // ICE_H
