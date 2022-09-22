#include <Nooskewl_Wedge/globals.h>

#include "globals.h"
#include "vbolt.h"

static gfx::Image *cloud;

const float vBolt_Step::CLOUD_W = 4.0f;
const float vBolt_Step::CLOUD_H = 1.5f;

static void zeus_callback(void *data)
{
	gfx::Model *z = static_cast<gfx::Model *>(data);
	z->stop();
	z->reset();
}

vBolt_Step::vBolt_Step(util::Point<float> start, util::Point<float> end, int rough_segments, bool mirror, SDL_Colour c1, SDL_Colour c2, SDL_Colour c3, SDL_Colour c4, wedge::Task *task) :
	Bolt_Step(start, end, rough_segments, mirror, c1, c2, c3, c4, task),
	started(false),
	zeus_started(false),
	sample_played(false)
{
	if (id == 0) {
		M3_GLOBALS->zeus->set_animation("ArmatureAction", zeus_callback, M3_GLOBALS->zeus);
		cloud = new gfx::Image("battle/cloud.tga");
	}
}

vBolt_Step::~vBolt_Step()
{
	if (id == 0) {
		delete cloud;
	}
}

bool vBolt_Step::run()
{
	Uint32 now = GET_TICKS();
	Uint32 elapsed = now - total_start_time;

	if (started == false && M3_GLOBALS->zeus->get_current_frame() == 75) {
		Bolt_Step::start();
		if (id == 0) {
			GLOBALS->spell_sfx["Bolt"]->play(false);
		}
		started = true;
	}

	if (id == 0 && sample_played == false && M3_GLOBALS->zeus->get_current_frame() == 60) {
		M3_GLOBALS->vbolt_sample->play(5.0f, false);
		sample_played = true;
	}

	if (zeus_started == false && elapsed >= POOF_TIME) {
		M3_GLOBALS->zeus->start();
		zeus_started = true;
	}

	if (elapsed < POOF_TIME) {
		int t = GET_TICKS() % 500;
		float f = (sin(t / 500.0f * M_PI * 2) + 1) / 8;
		for (size_t i = 0; i < poofs.size(); i++) {
			Poof &p = poofs[i];
			p.position += p.speed * f;
			p.angle += p.angle_speed;
			p.scale += p.scale_speed;
		}
	}

	for (size_t i = 0; i < clouds.size(); i++) {
		clouds[i].angle += clouds[i].angle_speed;
	}

	if (elapsed >= POOF_TIME+ZEUS_TIME) {
		send_done_signal();
		return false;
	}
	else {
		return true;
	}
}

void vBolt_Step::get_zeus_mvp(glm::mat4 &mv, glm::mat4 &p)
{
	// scales adjust for screen_offset
	float scale_x = (shim::screen_size.w*shim::scale)/shim::real_screen_size.w;
	float scale_y = (shim::screen_size.h*shim::scale)/shim::real_screen_size.h;
	float eye_x = 0, eye_y = -2.0f*scale_y, eye_z = 25;
	float centre_x = -4.5f*scale_x, centre_z = 0;

	float aspect = shim::screen_size.w / (float)shim::screen_size.h;
	p = glm::perspective(float(M_PI/4.0f), aspect, 1.0f, 1000.0f);
	mv = glm::lookAt(glm::vec3(eye_x, eye_y, eye_z), glm::vec3(centre_x, eye_y, centre_z), glm::vec3(0.0f, 1.0f, 0.0f));
	if (shim::screen_offset.x > 0.0f && shim::screen_offset.y > 0.0f) {
		mv = glm::scale(mv, glm::vec3(scale_x, scale_y, 1.0f));
	}
	else if (shim::screen_offset.x > 0.0f) {
		mv = glm::scale(mv, glm::vec3(scale_x, 1.0f, 1.0f));
	}
	else if (shim::screen_offset.y > 0.0f) {
		mv = glm::scale(mv, glm::vec3(1.0f, scale_y, 1.0f));
	}
	if (shim::allow_dpad_below && shim::screen_offset.y != 0 && shim::dpad_below) {
		float v;
		//v = (shim::real_screen_size.h - (shim::screen_size.h*shim::scale)) / 2.0f;
		//v /= shim::screen_offset.y;
		
		//v = (((shim::real_screen_size.h - (shim::screen_size.h*shim::scale)) / 2.0f) - shim::screen_offset.y) / (shim::real_screen_size.h/2.0f);
		//v = (shim::real_screen_size.h/2.0f) / shim::screen_offset.y;
		v = (shim::real_screen_size.h/2.0f) / (((shim::real_screen_size.h - (shim::screen_size.h*shim::scale)) / 2.0f) - shim::screen_offset.y);
		//v = (((shim::real_screen_size.h - (shim::screen_size.h*shim::scale)) / 2.0f) / shim::screen_offset.y);
		
		mv = glm::translate(mv, glm::vec3(0.0f, (float)v, 0.0f));
	}
}

void vBolt_Step::draw_zeus()
{
	glm::mat4 old_mv, old_p;
	gfx::get_matrices(old_mv, old_p);

	std::vector<gfx::Model::Node *> nodes = M3_GLOBALS->zeus->get_nodes();
	
	glm::mat4 mv, _p;

	get_zeus_mvp(mv, _p);

	gfx::set_matrices(mv, _p);
	gfx::update_projection();

	gfx::enable_depth_write(true);
	gfx::clear_depth_buffer(1.0f);
	gfx::enable_depth_test(true);

	M3_GLOBALS->zeus->draw_textured();
	
	gfx::enable_depth_write(false);
	gfx::set_cull_mode(gfx::FRONT_FACE); // sprites below normally are drawn correctly but with perspective y is inversed and therefore winding is also

	cloud->start_batch();
	for (size_t i = 0; i < clouds.size(); i++) {
		Cloud &c = clouds[i];
		const int phase = 1000;
		int t = GET_TICKS() % phase;
		float f = (float)t / phase * M_PI * 2 + c.spin_offset;
		float len = 0.1f;
		util::Point<float> add(cos(f) * len, sin(f) * len);
		add.y += -1.25f; // move it down a bit
		cloud->draw_rotated_scaled_z(
			util::Point<float>(cloud->size.w/2, cloud->size.h/2),
			c.position + add,
			c.angle,
			c.scale,
			c.z,
			c.flags
		);
	}
	cloud->end_batch();
	
	gfx::set_cull_mode(gfx::BACK_FACE);
	
	gfx::set_matrices(old_mv, old_p);
	gfx::update_projection();
	
	gfx::enable_depth_test(false);
}

void vBolt_Step::draw_fore()
{
	if (started) {
		Bolt_Step::draw_fore();
	}
	
	Uint32 now = GET_TICKS();
	Uint32 elapsed = now - total_start_time;

	if (id == count-1) {
		if (elapsed >= POOF_TIME*2/3) {
			Uint32 fade_start = POOF_TIME+(ZEUS_TIME*2/3);
			float alpha;
			if (elapsed >= fade_start) {
				alpha = (float)(elapsed-fade_start) / (ZEUS_TIME/3);
				if (alpha > 1) {
					alpha = 1;
				}
				alpha = 1 - alpha;
			}
			else {
				alpha = (float)(elapsed-POOF_TIME*2/3)/(POOF_TIME/3);
				if (alpha > 1.0f) {
					alpha = 1.0f;
				}
			}
			gfx::set_target_image(GLOBALS->work_image);
			gfx::clear(shim::transparent);
			draw_zeus();
			gfx::set_target_backbuffer();
			SDL_Colour tint = shim::white;
			tint.r *= alpha;
			tint.g *= alpha;
			tint.b *= alpha;
			tint.a *= alpha;

			// <0, 0, 0> is the bottom middle of the model, this gets that position after projection/transformations
			glm::mat4 mv, _p;
			get_zeus_mvp(mv, _p);
			glm::vec3 zero(0.0f, 0.0f, 0.0f);
			zero = glm::project(zero, mv, _p, glm::vec4(0.0f, 0.0f, (float)shim::real_screen_size.w, (float)shim::real_screen_size.h));
			// don't just use shim::screen_size above as it has to account for black bars
			zero.x -= shim::screen_offset.x;
			zero.y -= shim::screen_offset.y;
			zero.x /= shim::scale;
			zero.y /= shim::scale;
			zero.y = shim::screen_size.h - zero.y;

			util::Point<float> topleft(zero.x - 40, zero.y - 50);
			util::Size<float> size(70, 80);
			if (topleft.y < 0) {
				size.h -= topleft.y;
				topleft.y = 0;
			}
			if (topleft.x < 0) {
				size.w -= topleft.x;
				topleft.x = 0;
			}

			GLOBALS->work_image->stretch_region_tinted(tint, shim::screen_offset+topleft*shim::scale, size*shim::scale, topleft, size);
		}
	
		if (elapsed < POOF_TIME) {
			cloud->start_batch();
			for (size_t i = 0; i < poofs.size(); i++) {
				Poof &p = poofs[i];
				SDL_Colour tint = shim::white;
				tint.r *= p.alpha;
				tint.g *= p.alpha;
				tint.b *= p.alpha;
				tint.a *= p.alpha;
				float f = (float)elapsed / POOF_TIME;
				if (f > 0.75f) {
					f = (f - 0.75f) / 0.25f;
					f = 1 - f;
					if (f < 0) {
						f = 0;
					}
				}
				else if (f < 0.25f) {
					f = f / 0.25f;
				}
				else {
					f = 1.0f;
				}
				tint.r *= f;
				tint.g *= f;
				tint.b *= f;
				tint.a *= f;
				cloud->draw_tinted_rotated_scaled(
					tint,
					util::Point<float>(cloud->size.w/2, cloud->size.h/2),
					p.position,
					p.angle,
					p.scale,
					p.flags
				);
			}
			cloud->end_batch();
		}
	}
}

void vBolt_Step::start()
{
	if (id == count-1) {
		float dist = shim::screen_size.w/2;
		int ticks = (POOF_TIME * 2 / 3) / (1000 / shim::logic_rate);
		float max_speed = dist / ticks;
		float max_scale_speed = MAX_POOF_SCALE / (float)ticks;
		
		// <0, 0, 0> is the bottom middle of the model, this gets that position after projection/transformations
		glm::mat4 mv, _p;
		get_zeus_mvp(mv, _p);
		glm::vec3 zero(0.0f, 0.0f, 0.0f);
		zero = glm::project(zero, mv, _p, glm::vec4(0.0f, 0.0f, (float)shim::real_screen_size.w, (float)shim::real_screen_size.h));
		// don't just use shim::screen_size above as it has to account for black bars
		zero.x -= shim::screen_offset.x;
		zero.y -= shim::real_screen_size.h - shim::screen_size.h*shim::scale - shim::screen_offset.y;
		zero.x /= shim::scale;
		zero.y /= shim::scale;
		zero.y = shim::screen_size.h - zero.y;

		for (int i = 0; i < NPOOFS; i++) {
			Poof p;
			p.position = util::Point<float>(zero.x, zero.y);
			int r = util::rand(0, 1000);
			int groups = 2;
			int per_group = NPOOFS / groups;
			int my_i = i % per_group;
			int my_group = i / per_group;
			float f = my_group / (float)groups;
			float speed = f * max_speed * 2 + (max_speed/8.0f);
			float angle_i = (M_PI * 2.0f) / per_group;
			float extra_i = angle_i / groups;
			float angle = angle_i * my_i + extra_i * my_group;
			p.speed = util::Point<float>(cos(angle) * speed * ((float)shim::screen_size.w/shim::screen_size.h), sin(angle) * speed);
			p.scale = 0.01f;
			float stride = max_scale_speed / (groups-1);
			p.scale_speed = my_group * stride * 0.5f + 0.5f * max_scale_speed;
			p.angle = util::rand(0, 1000)/1000.0f * M_PI * 2.0f;
			p.angle_speed = ((util::rand(0, 1000) / 500.0f) - 1.0f) * 0.025f;
			// flipping the cloud image randomly might give a little variety
			r = i % 4;
			switch (r) {
				case 0:
					p.flags = 0;
					break;
				case 1:
					p.flags = gfx::Image::FLIP_V;
					break;
				case 2:
					p.flags = gfx::Image::FLIP_H;
					break;
				default:
					p.flags = gfx::Image::FLIP_V | gfx::Image::FLIP_H;
					break;
			}
			p.alpha = util::rand(0, 1000)/1000.0f * 0.5f + 0.5f;
			poofs.push_back(p);
		}

		for (int i = 0; i < NUM_CLOUDS; i++) {
			Cloud c;
			int groups = 4;
			int group_size = NUM_CLOUDS / groups;
			int group = i / group_size;
			int g = i % group_size;
			float o = (1.0f - sin(M_PI/2 + ((float)group/groups) * M_PI / 2)) * CLOUD_W / 4;
			c.position.x = (-CLOUD_W/2) + o + (CLOUD_W-o*2) * (float)g / group_size;
			c.position.y = (CLOUD_H/2) - (float)group/groups * CLOUD_H;
			c.z = -1.5f + 3 * (float)g/group_size;
			c.scale = (util::rand(0, 1000) / 1000.0f * 0.025f + 0.025f);
			c.angle = util::rand(0, 1000) / 1000.0f * M_PI * 2;
			c.angle_speed = (util::rand(0, 1) == 0 ? 1 : -1) * (util::rand(0, 1000) / 1000.0f * 0.025f + 0.05f);
			c.spin_offset = util::rand(0, 1000) / 1000.0f * M_PI * 2;
			// flipping the cloud image randomly might give a little variety
			int r = util::rand(0, 3);
			switch (r) {
				case 0:
					c.flags = 0;
					break;
				case 1:
					c.flags = gfx::Image::FLIP_V;
					break;
				case 2:
					c.flags = gfx::Image::FLIP_H;
					break;
				default:
					c.flags = gfx::Image::FLIP_V | gfx::Image::FLIP_H;
					break;
			}
			clouds.push_back(c);
		}
	}

	total_start_time = GET_TICKS();
}
