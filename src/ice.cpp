#include <Nooskewl_Wedge/general.h>

#include "ice.h"

const float Ice_Step::MAX_ANGLE_INC = 0.2f;

Ice_Step::Ice_Step(wedge::Task *task) :
	wedge::Step(task)
{
}

Ice_Step::~Ice_Step()
{
	for (size_t i = 0; i < images.size(); i++) {
		delete images[i];
	}
}

void Ice_Step::run_ice(Uint32 now)
{
	for (size_t i = 0; i < ice.size(); i++) {
		if (is_alive(now, (int)i)) {
			update((int)i);
		}
	}
}

bool Ice_Step::run()
{
	Uint32 now = GET_TICKS();

	run_ice(now);

	bool done = now >= end_time;
	
	if (done) {
		send_done_signal();
	}

	return !done;
}

void Ice_Step::draw_fore()
{
	Uint32 now = GET_TICKS();

	for (size_t i = 0; i < images.size(); i++) {
		images[i]->start_batch();
		for (size_t j = 0; j < ice.size(); j++) {
			if (ice[j].image_index == (int)i && is_alive(now, (int)j)) {
				draw_ice(now, (int)j);
			}
		}
		images[i]->end_batch();
	}
}

void Ice_Step::start()
{
	for (int i = 1; i < 256; i++) {
		std::string fn = "battle/snowflake" + util::itos(i) + ".tga";
		gfx::Image *image;
		try {
			image = new gfx::Image(fn);
		}
		catch (util::Error e) {
			image = NULL;
		}
		if (image == NULL) {
			break;
		}
		images.push_back(image);
	}

	int max = 0;

	for (size_t i = 0; i < NUM_PARTICLES; i++) {
		add_particle();
		max = MAX(max, int(ice[i].delay+ice[i].lifetime));
	}

	start_time = GET_TICKS();
	end_time = start_time + max;
	
	ice_start = util::Point<int>(shim::screen_size.w, -5);
	ice_end = util::Point<int>(-5, shim::screen_size.h*2/3);
}

void Ice_Step::add_particle()
{
	Ice_Particle i;

	float r = util::rand(0, 1000) / 1000.0f;
	r = increase_mids(r, 1);
	i.delay = r * MAX_DELAY;

	i.lifetime = util::rand(MIN_LIFETIME, MAX_LIFETIME);

	float r1 = util::rand(0, 1000) / 1000.0f;
	float r2 = util::rand(0, 1000) / 1000.0f;

	i.offset_start = util::Point<int>((r1*MAX_OFFSET*2)-MAX_OFFSET, 0);
	i.offset_end = util::Point<int>(0, (r2*MAX_OFFSET*2)-MAX_OFFSET);

	i.angle = util::rand(0, 1000) / 1000.0f * M_PI * 2.0f;
	i.angle_inc = util::rand(0, 1000) / 1000.0f * MAX_ANGLE_INC;

	float alpha = 0.8f + util::rand(0, 1000) / 1000.0f * 0.2f;
	i.colour = shim::palette[20];
	i.colour.r *= alpha;
	i.colour.g *= alpha;
	i.colour.b *= alpha;
	i.colour.a *= alpha;

	i.image_index = util::rand(0, (int)images.size()-1);

	const float min_pix_size = 3.0f;
	const float max_pix_size = 10.0f;
	const float min_scale = min_pix_size / images[i.image_index]->size.w;
	const float max_scale = max_pix_size / images[i.image_index]->size.w;

	i.scale = min_scale + util::rand(0, 1000) / 1000.0f * (max_scale-min_scale);

	i.angle_offset = util::rand(0, 1000) / 1000.0f;

	i.flips = util::rand(MIN_FLIPS, MAX_FLIPS);

	ice.push_back(i);
}
	
bool Ice_Step::is_alive(Uint32 now, int index)
{
	if  (now >= start_time+ice[index].delay && now < start_time+ice[index].delay+ice[index].lifetime) {
		return true;
	}
	return false;
}

void Ice_Step::update(int index)
{
	Ice_Particle &i = ice[index];

	i.angle += i.angle_inc;
}

void Ice_Step::draw_ice(Uint32 now, int index)
{
	Ice_Particle &i = ice[index];

	util::Point<float> start = ice_start + i.offset_start;
	util::Point<float> end = ice_end + i.offset_end;

	Uint32 elapsed = now - start_time - i.delay;
	float p = elapsed / (float)i.lifetime;

	Uint32 elapsed_total = now - start_time;
	float pp = elapsed_total / (float)(end_time - start_time);

	// a little speed up to match sfx
	const float end_p = 0.7f;
	const float inv = 1.0f - end_p;
	const float end_p2 = 0.85f;
	const float inv2 = 1.0f - end_p2;
	if (pp >= end_p) {
		float pp2 = ((pp - end_p) / inv) * 0.5f;
		if (pp >= end_p2) {
			pp2 += ((pp - end_p2) / inv2) * 0.5f;
		}
		p *= (pp2 + 1.0f);
	}

	if (p > 1.0f) {
		return;
	}

	float x = end.x + (start.x-end.x) * (1.0f - p);
	float angle = (1.0f - p) * M_PI + M_PI * 1.5f; // 270 to 90 (wrapping)
	int half_h = (end.y - start.y) / 2;
	int mid_y = start.y + half_h;

	util::Point<float> pos(x, mid_y-sin(angle)*half_h);

	gfx::Image *image = images[i.image_index];

	float angle_x, angle_y;
	if (index % 2 == 0) {
		angle_x = get_angle(now, index);
		angle_y = 0;
	}
	else {
		angle_x = 0;
		angle_y = get_angle(now, index);
	}

	float scale_x, scale_y;
	if (angle_x < M_PI) {
		scale_x = 1.0f - angle_x / M_PI;
	}
	else {
		scale_x = (angle_x-M_PI) / M_PI;
	}
	if (angle_y < M_PI) {
		scale_y = 1.0f - angle_y / M_PI;
	}
	else {
		scale_y = (angle_y-M_PI) / M_PI;
	}

	image->draw_tinted_rotated_scaledxy(i.colour, util::Point<float>(image->size.w/2.0f, image->size.h/2.0f), pos, i.angle, i.scale*scale_x, i.scale*scale_y);
}

float Ice_Step::get_angle(Uint32 now, int index)
{
	Ice_Particle &i = ice[index];
	
	Uint32 elapsed = now - start_time - i.delay;
	float p = elapsed / (float)i.lifetime;
	p = p + i.angle_offset;
	p = fmodf(p, 1.0f);
	float inv = 1.0f / i.flips;
	p = fmodf(p, inv);
	p = p / inv;
	p = lessen_mids(p, 2);
	return p * M_PI * 2;
}

// this pushes the mid range out towards ends
float Ice_Step::lessen_mids(float p, int passes)
{
	for (int i = 0; i < passes; i++) {
		p = 1.0f - ((cos(p * M_PI) + 1.0f) / 2.0f);
	}
	return p;
}

// this pushes the ends towards the middle
float Ice_Step::increase_mids(float p, int passes)
{
	for (int i = 0; i < passes; i++) {
		float d = 0.5f - p;
		float r = sin(p * M_PI) * d;
		p += r / passes;
	}
	return p;
}
