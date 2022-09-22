#include <Nooskewl_Wedge/general.h>

#include "fire.h"

#define SPEED 2.0f
#define STRAIGHTEN1 125
#define STRAIGHTEN2 100
#define SAVE_ANGLES 25

struct Draw_Info
{
	int colour;
	util::Point<float> pos;
	float size;
};

static bool compare_draw_info(const Draw_Info &a, const Draw_Info &b)
{
	return (b.pos.y+b.size/2.0f) < (a.pos.y+a.size/2.0f); // reverse sort
}

Fire_Step::Fire_Step(util::Point<int> draw_pos_bottom_middle, wedge::Task *task) :
	wedge::Step(task),
	draw_pos(draw_pos_bottom_middle),
	total_fire_particles(1000),
	changed(false)
{
	start_angle = (float)M_PI/2.0f;
	curr_angle = start_angle;
	for (int i = 0; i < SAVE_ANGLES; i++) {
		last_angles.push_back(curr_angle);
	}
	end_pos = draw_pos;
		
	curr_pos = draw_pos;
	move_end = GET_TICKS();
}

Fire_Step::~Fire_Step()
{
}

bool Fire_Step::run()
{
	Uint32 now = GET_TICKS();

	if (changed) {
		if (curr_pos != end_pos) {
			util::Point<float> diff = (end_pos-curr_pos);
			float len = diff.length();
			if (len <= SPEED) {
				curr_pos = end_pos;
				move_end = GET_TICKS();
			}
			else {
				float angle = diff.angle();
				curr_pos.x += cos(angle) * SPEED;
				curr_pos.y += sin(angle) * SPEED;
			}
		}
		else if (now-move_end < STRAIGHTEN1+STRAIGHTEN2) {
			Uint32 e = now-move_end;
			if (e < STRAIGHTEN1) {
				float diff = (start_angle+(M_PI*5/6*(end_pos.x<draw_pos.x?-1:1))) - start_angle;
				float p = e / (float)STRAIGHTEN1;
				p += util::rand(0, 100) / 750.0f;
				curr_angle = start_angle + sin(p*M_PI/2) * diff;
			}
			else {
				e -= STRAIGHTEN1;
				float diff = ((float)M_PI/2.0f) - (start_angle+(M_PI*5/6*(end_pos.x<draw_pos.x?-1:1)));
				float p = e / (float)STRAIGHTEN2;
				p += util::rand(0, 100) / 500.0f;
				curr_angle = (start_angle+(M_PI*5/6*(end_pos.x<draw_pos.x?-1:1))) + sin(p*M_PI/2) * diff;
			}
		}
		else {
			curr_angle = (float)M_PI/2.0f;
		}
	}

	last_angles.erase(last_angles.begin());
	last_angles.push_back(curr_angle);

	for (std::list<Fire_Particle>::iterator it = fire.begin(); it != fire.end();) {
		Fire_Particle &f = *it;
		f.p += f.speed;
		f.start_x += f.veeriness;
		if (f.p >= 1.0f) {
			it = fire.erase(it);
		}
		else {
			it++;
		}
	}

	int add = MIN(25, fire_to_add);

	if (add > 0) {
		for (int i = 0; i < add; i++) {
			add_fire_particle();
		}

		fire_to_add -= add;
	}
	else {
		int to_add = num_fire_particles - (int)fire.size();

		for (int i = 0; i < to_add; i++) {
			add_fire_particle();
		}
	}

	bool done = added_fire_particles >= total_fire_particles && fire.size() == 0;
	
	if (done) {
		send_done_signal();
	}

	return !done;
}

void Fire_Step::draw_fore()
{
	gfx::draw_primitives_start();

	std::vector<Draw_Info> draw_info;

	for (std::list<Fire_Particle>::iterator it = fire.begin(); it != fire.end(); it++) {
		Fire_Particle &f = *it;
		Draw_Info i;
		get_draw_info(f, i.colour, i.pos, i.size);
		draw_info.push_back(i);
	}
    
	std::sort(draw_info.begin(), draw_info.end(), compare_draw_info);

	for (size_t i = 0; i < draw_info.size(); i++) {
		Draw_Info &d = draw_info[i];
		gfx::draw_filled_rectangle(fire_colours[d.colour], d.pos-util::Point<float>(d.size/2, d.size/2), util::Size<float>(d.size, d.size));
	}
	
	gfx::draw_primitives_end();
}

void Fire_Step::start()
{
	fire_size = util::Size<int>(5, 30);
	num_fire_particles = fire_size.w * fire_size.h * 2.0f;
	added_fire_particles = 0;
	veers = util::rand(0, 5);
	fire_to_add = num_fire_particles;

	fire_colours.push_back(shim::palette[11]);
	fire_colours.push_back(shim::palette[11]);
	fire_colours.push_back(shim::palette[11]);
	fire_colours.push_back(shim::palette[10]);
	fire_colours.push_back(shim::palette[10]);
	fire_colours.push_back(shim::palette[10]);
	fire_colours.push_back(shim::palette[8]);
	fire_colours.push_back(shim::palette[27]);
}

void Fire_Step::add_fire_particle()
{
	if (added_fire_particles >= total_fire_particles) {
		return;
	}

	if (veers > 0) {
		if (added_fire_particles % (total_fire_particles / veers) == 0) {
			veer_dir = util::rand(0, 2) - 1;
		}
	}
	else {
		veer_dir = 0;
	}

	added_fire_particles++;

	float total_p = added_fire_particles / (float)total_fire_particles;

	Fire_Particle f;

	f.max_h = util::rand(fire_size.h/2, fire_size.h);
	f.p = 0 + 0.5f * util::rand(0, 1000) / 1000.0f * (1.0f - total_p);
	f.start_heat = (util::rand(0, 1000) / 1000.0f) * 0.5f + 0.5f;
	f.sin_offset = (util::rand(0, 1000) / 1000.0f) * 2 * M_PI;
	f.waviness = util::rand(0, 1000) / 1000.0f;
	f.waves = (1.0f - f.waviness) * MAX_WAVES;
	const float max_speed_pps = 40;
	const float min_speed_pps = 15;
	float speed_pps = min_speed_pps + (util::rand(0, 1000) / 1000.0f) * (max_speed_pps - min_speed_pps);
	float speed_ppf = speed_pps / shim::logic_rate;
	f.speed = speed_ppf / f.max_h;
	float x_p = util::rand(0, 1000) / 1000.0f;
	x_p *= x_p;
	x_p = (0.5f * x_p) + util::rand(0, 1000) / 1000.0f * 0.5f;
	f.start_x = fire_size.w * x_p;
	f.start_heat = util::rand(0, 1000) / 1000.0f * 0.5f;
	const float max_size = 3.0f;
	const float min_size = 0.5f;
	f.start_size = min_size + util::rand(0, 1000)/1000.0f * (max_size-min_size);
	f.end_size = util::rand(0, 1000) / 1000.0f * min_size * 2;
	f.veeriness = veer_dir * (util::rand(0, 1000) / 1000.0f) * 0.1f;

	fire.push_back(f);
}

void Fire_Step::get_draw_info(Fire_Particle &f, int &colour, util::Point<float> &pos, float &size)
{
	float p = f.p * f.p;
	int index = last_angles.size() - (p * last_angles.size());
	if (index < 0) {
		index = 0;
	}
	if (index >= (int)last_angles.size()) {
		index = (int)last_angles.size() - 1;
	}
	float ca = last_angles[index];
	float a2 = ca + (float)M_PI/2.0f;
	pos = util::Point<float>(curr_pos.x-cos(a2)*fire_size.w/2, curr_pos.y-sin(a2)*fire_size.w/2) + util::Point<float>(cos(a2) * f.start_x, sin(a2) * f.start_x);
	pos.x -= cos(ca) * p * f.max_h;
	pos.y -= sin(ca) * p * f.max_h;
	float waviness = f.waviness * (1.0f - p);
	pos.x += cos(ca) * sin(p * f.waves * M_PI * 2 + f.sin_offset) * waviness * MAX_WAVINESS;
	pos.x += sin(ca) * sin(p * f.waves * M_PI * 2 + f.sin_offset) * waviness * MAX_WAVINESS;
	float start_colour = fire_colours.size() * f.start_heat;
	float diff = fire_colours.size() - start_colour;
	colour = start_colour + p * diff;
	if (colour > (int)fire_colours.size()-1) {
		colour = (int)fire_colours.size()-1;
	}
	size = f.start_size - (p * (f.start_size - f.end_size));
}
	
void Fire_Step::set_start_angle(float start_angle)
{
	this->start_angle = start_angle;
	curr_angle = start_angle;
	last_angles.clear();
	for (int i = 0; i < SAVE_ANGLES; i++) {
		last_angles.push_back(curr_angle);
	}
	changed = true;
}

void Fire_Step::set_start_pos(util::Point<float> start_pos)
{
	draw_pos = start_pos;
	curr_pos = draw_pos;
	changed = true;
}

void Fire_Step::set_end_pos(util::Point<float> end_pos)
{
	this->end_pos = end_pos;
	changed = true;
}

void Fire_Step::double_particles()
{
	total_fire_particles *= 2;
}
