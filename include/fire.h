#ifndef FIRE_H
#define FIRE_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

class Fire_Step : public wedge::Step
{
public:
	Fire_Step(util::Point<int> draw_pos_bottom_middle, wedge::Task *task);
	virtual ~Fire_Step();
	
	bool run();
	void draw_fore();
	void start();

	void set_start_angle(float start_angle);
	void set_start_pos(util::Point<float> start_pos);
	void set_end_pos(util::Point<float> end_pos);
	void double_particles();

private:
	struct Fire_Particle {
		float max_h;
		float p;
		float start_heat;
		float sin_offset;
		float waviness;
		float speed;
		float waves;
		float start_x;
		float start_size;
		float end_size;
		float veeriness;
	};

	void add_fire_particle();
	void get_draw_info(Fire_Particle &f, int &colour, util::Point<float> &pos, float &size);

	util::Point<int> draw_pos;

	static const int MAX_WAVINESS = 7;
	static const int MAX_WAVES = 7;

	util::Size<int> fire_size;

	int num_fire_particles;
	int fire_to_add;
	int total_fire_particles;
	int added_fire_particles;

	std::vector<SDL_Colour> fire_colours;

	int veers;
	int veer_dir;
	
	std::list<Fire_Particle> fire;

	float start_angle;
	float curr_angle;
	util::Point<float> end_pos;
	util::Point<float> curr_pos;
	Uint32 move_end;

	std::vector<float> last_angles;

	bool changed;
};

#endif // FIRE_H
