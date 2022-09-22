#include <Nooskewl_Wedge/globals.h>

#include "cure.h"
#include "general.h"
#include "globals.h"

int Cure_Step::count;
gfx::Image *Cure_Step::work1;
gfx::Image *Cure_Step::work2;
gfx::Image *Cure_Step::work_src;
gfx::Image *Cure_Step::work_dest;
gfx::Image *Cure_Step::image;

void Cure_Step::static_start()
{
	count = 0;
	work1 = NULL;
	work2 = NULL;
}

Cure_Step::Cure_Step(util::Point<int> draw_pos, util::Size<int> size, wedge::Task *task) :
	wedge::Step(task),
	draw_pos(draw_pos),
	size(size)
{
}

Cure_Step::~Cure_Step()
{
	if (count == 1) {
		delete image;
	}
	lost_device();
}

void Cure_Step::lost_device()
{
	if (count == 1 && work1 != NULL) {
		delete work1;
		work1 = NULL;
		delete work2;
		work2 = NULL;
	}
	if (count > 0) {
		count--;
	}
}

void Cure_Step::found_device()
{
	if (count == 0) {
		util::Size<float> sz = image->size + util::Size<int>(3, 3);
		work1 = new gfx::Image(sz*shim::scale);
		work2 = new gfx::Image(sz*shim::scale);
	}
	count++;
}

bool Cure_Step::run()
{
	Uint32 elapsed = GET_TICKS() - start_time;
	if ((int)elapsed < NUM*ADD+FALL_TIME) {
		/*
		float pix_per_ms = (float)size.h / FALL_TIME;
		float speed = pix_per_ms * (1000.0f/shim::logic_rate);
		*/
		float max_y = draw_pos.y+size.h-2;
		for (size_t i = 0; i < positions.size(); i++) {
			float r = (float)(elapsed-(i+1)*ADD) / FALL_TIME;
			util::Point<float> &p = positions[i];
			float f = (float)(i+1) / positions.size();
			//float s = speed * (f * 0.5f + 0.5f);
			//p.y += s;
			float max = size.h * (f * 0.5f + 0.5f);
			p.y = draw_pos.y + r * max;
			if (p.y > max_y) {
				p.y = max_y;
			}
		}
	}
	else {
		/*
		float pix_per_ms = (float)size.h / (RISE_DURATION/2);
		float speed = pix_per_ms * (1000.0f/shim::logic_rate);
		*/
		Uint32 e = elapsed - (NUM*ADD+FALL_TIME);
		float r = (float)e / (RISE_DURATION/2);
		for (size_t i = 0; i < positions.size(); i++) {
			util::Point<float> &p = positions[i];
			float f = (float)(i+1) / positions.size();
			f = f * f;
			f = 1.0f - f;
			float max = size.h * (f * 0.5f + 0.5f);
			p.y = (draw_pos.y+size.h-2) - r * max;
			//float s = speed * (f * 0.5f + 0.5f);
			//p.y -= s;
			if (p.y < draw_pos.y) {
				p.y = draw_pos.y;
			}
		}
	}
	
	bool ret = (int)elapsed < DURATION;
	
	if (ret == false) {
		send_done_signal();
	}

	return ret;
}

void Cure_Step::draw_fore()
{
	Uint32 now = GET_TICKS();
	Uint32 elapsed = now - start_time;

	float add = elapsed / (float)ADD;
	float p = add / NUM;
	if (p > 1.0f) {
		p = 1.0f;
	}
	p = p * p;
	add = p * NUM;

	int to_add = MIN(NUM, add) - positions.size();

	for (int i = 0; i < to_add; i++) {
		util::Point<float> p;
		int index = (int)positions.size() + 1;
		p.x = draw_pos.x + size.w - (index * (size.w/NUM));
		p.y = draw_pos.y;
		positions.push_back(p);
	}

	int spin;

	if ((int)elapsed > NUM*ADD+FALL_TIME) {
		spin = SPIN2;
	}
	else {
		spin = SPIN1;
	}

	Uint32 ticks = now % spin;
	float angle = ticks/(float)spin * M_PI*2.0f;

	const int cycles = 1;
	int offset = (float)elapsed / DURATION * cycles * NUM;

	const int NLOOPS = (const int)MAX(4.0f, MIN(MAX(8.0f, shim::scale/3.0f), shim::scale));
	gfx::Shader *tmp_shader = NULL;
	if (id == 0) {
		work_src = work1;
		work_dest = work2;
	}

	for (int loops = 0; loops < NLOOPS; loops++) {
		if (loops == 0 && id == 0) {
			gfx::set_target_image(work_src);
			gfx::clear(shim::transparent);
			gfx::set_target_image(work_dest);
			gfx::clear(shim::transparent);

			glm::mat4 modelview, proj;
			modelview = glm::scale(modelview, glm::vec3(shim::scale, shim::scale, 1.0f));
			proj = glm::ortho(0.0f, (float)work1->size.w, (float)work1->size.h, 0.0f);
			gfx::set_matrices(modelview, proj);
			gfx::update_projection();
			image->draw(util::Point<float>(1.5f, 1.5f)); // work is image->size+3 (scaled, but transformed by shim::scale)
		}

		if (loops == NLOOPS-1) {
			Uint32 prev_cache = gfx::Vertex_Cache::instance()->get_current_cache();
			gfx::Vertex_Cache::instance()->select_cache(4);
			if (id == 0) {
				image->start_batch();
			}
			for (size_t i = 0; i < positions.size(); i++) {
				if ((int)elapsed > NUM*ADD+FALL_TIME && positions[i].y == draw_pos.y) {
					continue;
				}
				float p = MAX(0.0f, 1.0f - ((abs((offset%NUM)-(int)i)+1.0f) / (NUM/2.0f)));
				SDL_Colour start_colour = shim::palette[20];
				SDL_Colour colour = brighten(start_colour, 1.5f * p);
				image->draw_tinted_rotated(colour, util::Point<float>(image->size.w/2.0f, image->size.h/2.0f), positions[i], angle);
			}
			if (id == count-1) {
				image->end_batch();
			}
			gfx::Vertex_Cache::instance()->select_cache(prev_cache);
		}

		if (loops < NLOOPS-1) {
			if (id == 0) {
				if (loops == 0) {
					tmp_shader = shim::current_shader;
					shim::current_shader = M3_GLOBALS->blur_shader;
					shim::current_shader->use();
				}

				gfx::Image *tmp = work_dest;
				work_dest = work_src;
				work_src = tmp;

				gfx::set_target_image(work_dest);

				const float OFFSET_INC_X = (shim::scale / NLOOPS) / work1->size.w;
				const float OFFSET_INC_Y = (shim::scale / NLOOPS) / work1->size.h;
				float offset_x = OFFSET_INC_X * (loops+1);
				float offset_y = OFFSET_INC_Y * (loops+1);
				shim::current_shader->set_float("offset_x", offset_x);
				shim::current_shader->set_float("offset_y", offset_y);

				work_src->draw(util::Point<int>(0, 0));

			}

			if (loops == NLOOPS-2) {
				gfx::set_target_backbuffer();

				if (id == 0) {
					shim::current_shader = tmp_shader;
					shim::current_shader->use();
				}

				Uint32 prev_cache = gfx::Vertex_Cache::instance()->get_current_cache();
				gfx::Vertex_Cache::instance()->select_cache(3);

				if (id == 0) {
					work_dest->start_batch();
				}

				for (size_t i = 0; i < positions.size(); i++) {
					if ((int)elapsed > NUM*ADD+FALL_TIME && positions[i].y == draw_pos.y) {
						continue;
					}
					float p = MAX(0.0f, 1.0f - ((abs((offset%NUM)-(int)i)+1.0f) / (NUM/2.0f)));
					SDL_Colour start_colour = shim::palette[30];
					SDL_Colour colour = brighten(start_colour, 1.5f * p);
					work_dest->draw_tinted_rotated(colour, util::Point<float>(work_dest->size.w/2.0f, work_dest->size.h/2.0f), util::Point<float>(positions[i].x*shim::scale, positions[i].y*shim::scale), angle);
				}

				if (id == count-1) {
					gfx::set_default_projection(shim::real_screen_size, shim::screen_offset, 1.0f);
					gfx::update_projection();

					work_dest->end_batch();
				}

				gfx::Vertex_Cache::instance()->select_cache(prev_cache);
				
				gfx::set_default_projection(shim::real_screen_size, shim::screen_offset, shim::scale);
				gfx::update_projection();
			}
		}
	}
}

void Cure_Step::start()
{
	id = count;

	start_time = GET_TICKS();
	if (id == 0) {
		image = new gfx::Image("battle/cure.tga");
	}
	NUM = size.w / image->size.w + 1;
	RISE_DURATION = NUM*ADD+FALL_TIME;
	DURATION = NUM * ADD + FALL_TIME + RISE_DURATION;
	
	found_device(); // create work textures
}
