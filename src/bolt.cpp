#include <Nooskewl_Wedge/general.h>

#include "bolt.h"
#include "globals.h"
#include "vbolt.h"

//#define TESTING_SEEDS
//#define USE_ARRAY
//#define TESTING_ANIMATION
//#define SHOW_TEXT

static int count2;

#ifdef TESTING_SEEDS
const int NUM_SEEDS = 14;
const int seeds[NUM_SEEDS] = { 0, 6, 15, 16, 25, 209, 357, 455, 528, 600, 653, 764, 803, 989 }; // a few that look OK
#else
const int NUM_SEEDS = 1;
const int seeds[NUM_SEEDS] = { 803 }; // these ones look decent
#endif

static std::vector < util::Point<float> > ends;

int Bolt_Step::count;

void Bolt_Step::static_start()
{
	count = 0;
#if defined TESTING_SEEDS && !defined USE_ARRAY
	count2 = 1000; // already seen some
#else
	count2 = 0;
#endif
	ends.clear();
}

Bolt_Step::Bolt_Step(util::Point<float> start, util::Point<float> end, int rough_segments, bool mirror, SDL_Colour c1, SDL_Colour c2, SDL_Colour c3, SDL_Colour c4, wedge::Task *task) :
	wedge::Step(task),
	_start(start),
	end(end),
	rough_segments(rough_segments),
	mirror(mirror),
	c1(c1),
	c2(c2),
	c3(c3),
	c4(c4)
{
	create();
}

Bolt_Step::~Bolt_Step()
{
	destroy();
}

// create() and destroy() functions are separated from constructor/destructor for testing for good seeds
void Bolt_Step::create()
{
	id = count++;

#if defined TESTING_SEEDS && !defined USE_ARRAY
	util::srand(count2);
#else
	util::srand(seeds[count2%NUM_SEEDS]);
#endif
	count2++;

	bolt = calc_bolt(_start, end, 1, rough_segments, 10);

	ends.push_back(bolt[bolt.size()-1]);

	int bolt_size = (int)bolt.size();

	float branchable = bolt_size * 0.75f;
	int nbranches = branchable / 2;
	
	int dir = util::rand(0, 1) == 1 ? 1 : -1;
		
	for (int i = 0; i < nbranches; i++) {
		int segment;
		segment = (bolt_size * 0.2f) + i * (bolt_size * 0.6f / nbranches);
		float base_angle = (end-_start).angle();
		segments.push_back(segment);
		util::Point<float> start = bolt[segment];
		float angle = base_angle + (-(util::rand(0, 1000)/1000.0f * M_PI/16.0f) + M_PI / 4.0f) * dir;
		float len = util::rand(10, 15);
		util::Point<float> end(start.x+cos(angle)*len, start.y+sin(angle)*len);
		util::Point<float> start_ext(start.x+cos(angle), start.y+sin(angle));
		util::Point<float> end_ext(start.x+cos(angle)*100, start.y+sin(angle)*100);
		if (cd::line_line(&start_ext, &end_ext, &bolt[segment+1], &bolt[segment], NULL) ||
			cd::line_line(&start_ext, &end_ext, &bolt[segment+2], &bolt[segment+1], NULL)) {
			continue;
		}
		end = util::Point<float>(start.x+cos(angle)*len, start.y+sin(angle)*len);
		branches.push_back(calc_bolt(start, end, dir, rough_segments/3, 5));
		branch_directions.push_back(dir);
		dir = -dir;
	}

	if (mirror) {
		bolt = flip(end, bolt);
		for (int i = 0; i < nbranches; i++) {
			branches[i] = flip(end, branches[i]);
		}
	}

	util::srand((uint32_t)time(NULL));
}

void Bolt_Step::destroy()
{
	count--;

	bolt.clear();
	branches.clear();
	segments.clear();

	if (id == 0) {
		ends.clear();
	}
}

void Bolt_Step::handle_event(TGUI_Event *event)
{
#ifdef TESTING_SEEDS
	if (event->type == TGUI_KEY_DOWN && event->keyboard.code == TGUIK_SPACE) {
		destroy();
		create();
	}
#endif
}

bool Bolt_Step::run()
{
	Uint32 now = GET_TICKS();
	Uint32 elapsed = now - start_time;
#ifdef TESTING_SEEDS
	bool done = false;
#else
	bool done = (int)elapsed >= DURATION;
#endif
	
	if (done) {
		send_done_signal();
	}

	return !done;
}

void Bolt_Step::draw_fore()
{
	Uint32 now = GET_TICKS();
#ifdef TESTING_ANIMATION
	Uint32 elapsed = (now - start_time) % DURATION;
#else
	Uint32 elapsed = now - start_time;
#endif
	float p = elapsed / (float)DURATION;
	if (p > 1.0f) {
		p = 1.0f;
	}

#if defined TESTING_SEEDS && !defined TESTING_ANIMATION
	p = 0.5f;
#endif

	bool half = p >= 0.5f;

	if (id == 0) {
		gfx::draw_primitives_start(); // this happens to work across Bolt_Step/vBolt_Step instances...
	}

	float p_bak = p;

	if (half) {
		p = 1.0f - ((p - 0.5f) / 0.5f);
	}
	else {
		p = p / 0.5f;
		p = p * p;
	}

	draw_bolt(p_bak, 3.25f, 3.25f, c1);
	draw_bolt(p_bak, 2.5f, 3.25f, c2);
	draw_bolt(p_bak, 1.75f, 3.25f, c3);
	draw_bolt(p_bak, 1.0f, 3.25f, c4);
	
	if (id == count-1) { // only draw this once if there are multiple bolts
		gfx::set_target_image(GLOBALS->work_image);
		gfx::clear(shim::transparent);
		gfx::set_default_projection(shim::real_screen_size, shim::screen_offset, shim::scale);
		gfx::update_projection();

		gfx::draw_primitives_end();

		gfx::set_target_backbuffer();

		glm::mat4 identity, _mv, _p;
		gfx::get_matrices(_mv, _p);
		gfx::set_matrices(identity, _p);
		gfx::update_projection();

		SDL_Colour colour = shim::white;
		colour.r *= p;
		colour.g *= p;
		colour.b *= p;
		colour.a *= p;

		GLOBALS->work_image->draw_tinted(colour, util::Point<int>(0, 0));

		gfx::set_matrices(_mv, _p);
		gfx::update_projection();

		util::Point<float> middle(0.0f, 0.0f);
		for (size_t i = 0; i < ends.size(); i++) {
			middle += ends[i];
		}
		middle /= ends.size();

#if !defined TESTING_SEEDS || defined TESTING_ANIMATION
		gfx::draw_filled_circle(colour, middle, p*150, p*150);
#endif

#if defined TESTING_SEEDS && defined SHOW_TEXT
		shim::font->enable_shadow(shim::black, gfx::Font::FULL_SHADOW);
#ifdef USE_ARRAY
		int num = (count2-1) % NUM_SEEDS;
#else
		int num = count2 - 1;
#endif
		shim::font->draw(shim::white, util::itos(num), util::Point<float>(2.0f, 2.0f));
		shim::font->disable_shadow();
#endif
	}

}

void Bolt_Step::draw_bolt(float p, float scale, float max_scale, SDL_Colour colour)
{
	SDL_Colour vertex_colours[4];
	vertex_colours[0] = colour;
	vertex_colours[1] = colour;
	vertex_colours[2] = colour;
	vertex_colours[3] = colour;

	int last;
	if (p < 0.5f) {
		last = (int)MIN(bolt.size()-1, (p / 0.5f) * bolt.size());
	}
	else {
		last = (int)MIN(bolt.size()-1, (1.0f - ((p - 0.85f) / 0.15f)) * bolt.size());
	}
	const int fading_sections = 4;

	util::Point<float> last_pt = bolt[0];

	for (size_t i = 1; i < bolt.size(); i++) {
		float mul;
		if ((int)i < last-(fading_sections-1)) {
			mul = 1.0f;
		}
		else {
			int ii = (int)i - (last-(fading_sections-1));
			mul = 1.0f - (ii/(float)fading_sections);
		}
		float p2 = get_p(p, (int)i, (int)bolt.size());
		float p_next;
		if ((int)i == last) {
			p_next = p2;
		}
		else {
			float mul2;
			if ((int)i+1 < last-(fading_sections-1)) {
				mul2 = 1.0f;
			}
			else {
				int ii = (int)(i+1) - (last-(fading_sections-1));
				mul2 = 1.0f - (ii/(float)fading_sections);
			}
			p_next = mul2 * p2;
		}
		const float branch_scale = 0.65f;
		for (size_t j = 0; j < branches.size(); j++) {
			if (segments[j] == (int)i-1) {
				int last_b = (int)MIN(branches[j].size()-1, p2 * branches[j].size());
				util::Point<float> last_pt_b = branches[j][0];
				for (size_t k = 1; k < branches[j].size(); k++) {
					const int fading_sections_b = 3;
					float mul2;
					if ((int)k < last_b-(fading_sections_b-1)) {
						mul2 = 1.0f;
					}
					else {
						int kk = (int)k - (last_b-(fading_sections_b-1));
						mul2 = 1.0f - (kk/(float)fading_sections_b);
					}
					float p3;
					if (p >= 0.5f) {
						p3 = p2 * mul2;
					}
					else {
						p3 = get_p(p2/2.0f, (int)k, (int)branches[j].size()) * mul2;
					}
					float p_next_b;
					if ((int)k == last_b) {
						p_next_b = p3;
					}
					else {
						float mul3;
						if ((int)k+1 < last_b-(fading_sections_b-1)) {
							mul3 = 1.0f;
						}
						else {
							int ii = (int)(k+1) - (last_b-(fading_sections_b-1));
							mul3 = 1.0f - (ii/(float)fading_sections_b);
						}
						p_next_b = mul3 * p3;
					}
					if (p3 < 0.1f) {
						continue;
					}
					else {
						util::Point<float> start = branches[j][k-1];
						util::Point<float> diff = branches[j][k]-start;
						float angle = diff.angle();
						util::Point<float> corner1, corner2, corner3, corner4;
						util::Point<float> cornerm1, cornerm2;
						if (k >= 2) {
							float width = p3*branch_scale*max_scale;
							float width2 = p3*branch_scale*scale;
							float angle2 = (start - last_pt_b).angle();
							float mul;
							if (branch_directions[j] == 1) {
								float s = sin(angle);
								float s2 = sin(angle2);
								if (s2 < 0.0f) {
									mul = s2 < s ? -1.0f : 1.0f;
								}
								else {
									mul = s2 > s ? 1.0f : -1.0f;
								}
							}
							else {
								float c = cos(angle);
								float c2 = cos(angle2);
								if (c2 < 0.0f) {
									mul = c2 < c ? -1.0f : 1.0f;
								}
								else {
									mul = c2 > c ? 1.0f : -1.0f;
								}
							}
							util::Point<float> a(start.x+cos(angle2+mul*M_PI/2.0f)*width/2.0f, start.y+sin(angle2+mul*M_PI/2.0f)*width/2.0f);
							corner1 = util::Point<float>(start.x+cos(angle2+mul*M_PI/2.0f)*width2/2.0f, start.y+sin(angle2+mul*M_PI/2.0f)*width2/2.0f);
							corner2 = util::Point<float>(start.x+cos(angle2-mul*M_PI/2.0f)*width2/2.0f, start.y+sin(angle2-mul*M_PI/2.0f)*width2/2.0f);
							start = util::Point<float>(a.x+cos(angle-mul*M_PI/2.0f)*width/2.0f, a.y+sin(angle-mul*M_PI/2.0f)*width/2.0f);
							diff = branches[j][k]-start;
							angle = diff.angle();
							corner3 = util::Point<float>(start.x+cos(angle+mul*M_PI/2.0f)*width2/2.0f, start.y+sin(angle+mul*M_PI/2.0f)*width2/2.0f);
							corner4 = util::Point<float>(start.x+cos(angle-mul*M_PI/2.0f)*width2/2.0f, start.y+sin(angle-mul*M_PI/2.0f)*width2/2.0f);
							if (mul == 1) {
								util::Point<float> tmp;
								tmp = corner1;
								corner1 = corner2;
								corner2 = tmp;
								tmp = corner3;
								corner3 = corner4;
								corner4 = tmp;
							}
							util::Point<float> tmp1, tmp2, tmp3, tmp4;
							tmp1 = corner1 + util::Point<float>(cos(angle2)*50, sin(angle2)*50);
							tmp2 = corner2 + util::Point<float>(cos(angle2)*50, sin(angle2)*50);
							tmp3 = corner3 + util::Point<float>(cos(M_PI+angle)*50, sin(M_PI+angle)*50);
							tmp4 = corner4 + util::Point<float>(cos(M_PI+angle)*50, sin(M_PI+angle)*50);
							if ((corner1-corner3).length() <= 1.0f) {
								cornerm1 = corner1;
							}
							else {
								bool collides = cd::line_line(&tmp1, &corner1, &tmp3, &corner3, &cornerm1);
								if (collides == false) {
									cornerm1 = corner1;
								}
							}
							if ((corner2-corner4).length() < 1.0f) {
								cornerm2 = corner2;
							}
							else {
								bool collides = cd::line_line(&tmp2, &corner2, &tmp4, &corner4, &cornerm2);
								if (collides == false) {
									cornerm2 = corner2;
								}
							}
						}
						float w1 = p3*branch_scale*scale*0.5f;
						float w2 = p_next_b*branch_scale*scale*0.5f;
						float angle2 = (start - branches[j][k]).angle();
						if ((int)k == last_b) {
							float a1 = angle + M_PI/2.0f;
							float a2 = angle - M_PI/2.0f;
							float len = p3 * scale * 0.5f * branch_scale;
							util::Point<float> pt1(start.x+cos(a1)*len, start.y+sin(a1)*len);
							util::Point<float> pt2(start.x+cos(a2)*len, start.y+sin(a2)*len);
							gfx::draw_filled_triangle(colour, pt1, pt2, branches[j][k]);
						}
						else {
							//gfx::draw_line(colour, start, branches[j][k], p2*scale);
							util::Point<float> a = start + util::Point<float>(cos(angle2+M_PI/2.0f)*w1, sin(angle2+M_PI/2.0f)*w1);
							util::Point<float> b = start + util::Point<float>(cos(angle2-M_PI/2.0f)*w1, sin(angle2-M_PI/2.0f)*w1);
							util::Point<float> c = branches[j][k] + util::Point<float>(cos(angle2+M_PI/2.0f)*w2, sin(angle2+M_PI/2.0f)*w2);
							util::Point<float> d = branches[j][k] + util::Point<float>(cos(angle2-M_PI/2.0f)*w2, sin(angle2-M_PI/2.0f)*w2);
							noo::gfx::Vertex_Cache::instance()->cache(vertex_colours, noo::util::Point<float>(0, 0), noo::util::Size<float>(0, 0), a, c, d, b, 0);
						}
						if (k >= 2) {
							noo::gfx::Vertex_Cache::instance()->cache(vertex_colours, noo::util::Point<float>(0, 0), noo::util::Size<float>(0, 0), corner1, cornerm1, cornerm2, corner2, 0);
							noo::gfx::Vertex_Cache::instance()->cache(vertex_colours, noo::util::Point<float>(0, 0), noo::util::Size<float>(0, 0), cornerm1, corner3, corner4, cornerm2, 0);
						}
						last_pt_b = start;
					}
				}
			}
		}
		p2 *= mul;
		if (p2 < 0.1f) {
			continue;
		}
		else {
			util::Point<float> start = bolt[i-1];
			util::Point<float> diff = bolt[i]-start;
			float angle = diff.angle();
			util::Point<float> corner1, corner2, corner3, corner4;
			util::Point<float> cornerm1, cornerm2;
			if (i >= 2) {
				float width = p2*max_scale;
				float width2 = p2*scale;
				float angle2 = (start - last_pt).angle();
				float mul;
				float s = sin(angle);
				float s2 = sin(angle2);
				if (s < 0.0f) {
					mul = s2 < s ? -1.0f : 1.0f;
				}
				else {
					mul = s2 > s ? 1.0f : -1.0f;
				}
				util::Point<float> a(start.x+cos(angle2+mul*M_PI/2.0f)*width/2.0f, start.y+sin(angle2+mul*M_PI/2.0f)*width/2.0f);
				corner1 = util::Point<float>(start.x+cos(angle2+mul*M_PI/2.0f)*width2/2.0f, start.y+sin(angle2+mul*M_PI/2.0f)*width2/2.0f);
				corner2 = util::Point<float>(start.x+cos(angle2-mul*M_PI/2.0f)*width2/2.0f, start.y+sin(angle2-mul*M_PI/2.0f)*width2/2.0f);
				start = util::Point<float>(a.x+cos(angle-mul*M_PI/2.0f)*width/2.0f, a.y+sin(angle-mul*M_PI/2.0f)*width/2.0f);
				diff = bolt[i]-start;
				angle = diff.angle();
				corner3 = util::Point<float>(start.x+cos(angle+mul*M_PI/2.0f)*width2/2.0f, start.y+sin(angle+mul*M_PI/2.0f)*width2/2.0f);
				corner4 = util::Point<float>(start.x+cos(angle-mul*M_PI/2.0f)*width2/2.0f, start.y+sin(angle-mul*M_PI/2.0f)*width2/2.0f);
				if (mul == 1) {
					util::Point<float> tmp;
					tmp = corner1;
					corner1 = corner2;
					corner2 = tmp;
					tmp = corner3;
					corner3 = corner4;
					corner4 = tmp;
				}
				util::Point<float> tmp1, tmp2, tmp3, tmp4;
				tmp1 = corner1 + util::Point<float>(cos(angle2)*50, sin(angle2)*50);
				tmp2 = corner2 + util::Point<float>(cos(angle2)*50, sin(angle2)*50);
				tmp3 = corner3 + util::Point<float>(cos(M_PI+angle)*50, sin(M_PI+angle)*50);
				tmp4 = corner4 + util::Point<float>(cos(M_PI+angle)*50, sin(M_PI+angle)*50);
				if ((corner1-corner3).length() <= 1.0f) {
					cornerm1 = corner1;
				}
				else {
					bool collides = cd::line_line(&tmp1, &corner1, &tmp3, &corner3, &cornerm1);
					if (collides == false) {
						cornerm1 = corner1;
					}
				}
				if ((corner2-corner4).length() < 1.0f) {
					cornerm2 = corner2;
				}
				else {
					bool collides = cd::line_line(&tmp2, &corner2, &tmp4, &corner4, &cornerm2);
					if (collides == false) {
						cornerm2 = corner2;
					}
				}
			}
			float w1 = p2*scale*0.5f;
			float w2 = p_next*scale*0.5f;
			float angle2 = (start - bolt[i]).angle();
			if ((int)i == last) {
				float a1 = angle + M_PI/2.0f;
				float a2 = angle - M_PI/2.0f;
				float len = p2 * scale * 0.5f;
				util::Point<float> pt1(start.x+cos(a1)*len, start.y+sin(a1)*len);
				util::Point<float> pt2(start.x+cos(a2)*len, start.y+sin(a2)*len);
				gfx::draw_filled_triangle(colour, pt1, pt2, bolt[i]);
			}
			else {
				//gfx::draw_line(colour, start, bolt[i], p2*scale);
				util::Point<float> a = start + util::Point<float>(cos(angle2+M_PI/2.0f)*w1, sin(angle2+M_PI/2.0f)*w1);
				util::Point<float> b = start + util::Point<float>(cos(angle2-M_PI/2.0f)*w1, sin(angle2-M_PI/2.0f)*w1);
				util::Point<float> c = bolt[i] + util::Point<float>(cos(angle2+M_PI/2.0f)*w2, sin(angle2+M_PI/2.0f)*w2);
				util::Point<float> d = bolt[i] + util::Point<float>(cos(angle2-M_PI/2.0f)*w2, sin(angle2-M_PI/2.0f)*w2);
				noo::gfx::Vertex_Cache::instance()->cache(vertex_colours, noo::util::Point<float>(0, 0), noo::util::Size<float>(0, 0), a, c, d, b, 0);
			}
			if (i >= 2) {
				noo::gfx::Vertex_Cache::instance()->cache(vertex_colours, noo::util::Point<float>(0, 0), noo::util::Size<float>(0, 0), corner1, cornerm1, cornerm2, corner2, 0);
				noo::gfx::Vertex_Cache::instance()->cache(vertex_colours, noo::util::Point<float>(0, 0), noo::util::Size<float>(0, 0), cornerm1, corner3, corner4, cornerm2, 0);
			}
			last_pt = start;
		}
	}
}

void Bolt_Step::start()
{
	start_time = GET_TICKS();
}

std::vector< util::Point<float> > Bolt_Step::calc_bolt(util::Point<float> start, util::Point<float> end, int direction, int rough_segments, int snap)
{
	std::vector< util::Point<float> > bolt;

	bolt.push_back(start);

	while (true) {
		util::Point<float> last = bolt.back();
		util::Point<float> diff = end-last;
		float dist_from_goal = diff.length();
		if (dist_from_goal <= snap) {
			bolt.push_back(end);
			break;
		}
		float angle = diff.angle();
		float total_dist = (end-start).length();
		const float max_deviance = float(M_PI / 4.0f);
		float p = dist_from_goal/total_dist;
		float deviance;
		if (p > 0.15f) {
			deviance = max_deviance;
		}
		else {
			deviance = p * max_deviance;
		}
		float dist_to_line = cd::dist_point_line(last, start, end);
		float next_len = total_dist / util::rand(rough_segments, rough_segments * 1.5f);
		float min_angle;
		float max_angle;
		if (dist_to_line <= next_len) {
			min_angle = (end-start).angle();
			max_angle = min_angle + deviance * direction;
		}
		else {
			min_angle = angle - deviance;
			max_angle = angle + deviance;
		}
		if (min_angle > max_angle) {
			float tmp = min_angle;
			min_angle = max_angle;
			max_angle = tmp;
		}
		float angle_diff = max_angle - min_angle;
		float next_angle = min_angle + util::rand(0, 1000)/1000.0f * angle_diff;
		bolt.push_back(util::Point<float>(last.x+cos(next_angle)*next_len, last.y+sin(next_angle)*next_len));
	}

	return bolt;
}

float Bolt_Step::get_p(float p, int segment, int nsegments)
{
	if (p >= 0.5f) {
		return (1.0f - ((p - 0.5f) / 0.5f));
	}
	p = p / 0.5f;
	float p2 = (float)segment / nsegments;
	if (p < p2) {
		return 0.0f;
	}
	float p3 = 1.0f / nsegments;
	float s = p3 * (segment+1);
	if (p > s) {
		return 1.0f;
	}
	float p4 = p - (segment * p3);
	return MIN(1.0f, p4 / p3);
}

std::vector< util::Point<float> > Bolt_Step::flip(util::Point<float> end, std::vector< util::Point<float> > v)
{
	std::vector< util::Point<float> > ret;
	for (size_t i = 0; i < v.size(); i++) {
		util::Point<float> p = v[i];
		float f = end.x - p.x;
		ret.push_back(util::Point<float>(end.x+f, p.y));
	}

	return ret;
}
