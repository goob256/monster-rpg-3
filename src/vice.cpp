#include <Nooskewl_Wedge/globals.h>

#include "battle_game.h"
#include "globals.h"
#include "vice.h"

int vIce_Step::count;
int vIce_Step::destroyed_count;

void vIce_Step::static_start()
{
	count = 0;
	destroyed_count = 0;
}

vIce_Step::vIce_Step(util::Point<float> target, wedge::Task *task) :
	Ice_Step(task),
	target(target),
	nframes(0),
	started(false),
	played_sound(false),
	last_ticks(0)
{
	id = count;
	count++;
}

vIce_Step::~vIce_Step()
{
	destroyed_count++;
	if (destroyed_count == count) {
		count = 0;
		destroyed_count = 0;
	}
}

bool vIce_Step::run()
{
	Uint32 now = GET_TICKS();

	Uint32 elapsed = now - start_time;

	if (started == false && elapsed > (MAX_DELAY+MAX_LIFETIME-VICE_TIME-500)) {
		started = true;
		icicle_start = now;
	}

	if (id == count-1) {
		run_ice(now);
	}

	bool done = now >= start_time + MAX_DELAY + MAX_LIFETIME;
	if (done) {
		send_done_signal();
	}
	return !done;
}

void vIce_Step::draw_fore()
{
	if (started == false) {
		if (id == count-1) {
			Ice_Step::draw_fore();
		}
		return;
	}

	draw_icicle(target, icicle_particles);

	if (id == count-1) {
		Ice_Step::draw_fore();
	}
}

void vIce_Step::start()
{
	if (id == count-1) {
		Ice_Step::start();
		M3_GLOBALS->spell_sfx["Ice"]->play(false);
	}
	else {
		start_time = GET_TICKS();
	}

	icicle_particles = gen_icicle_particles();
}

std::vector<vIce_Step::Icicle_Particle> vIce_Step::gen_icicle_particles()
{
	std::vector<Icicle_Particle> icicle_particles;

	gfx::Model::Node *node = M3_GLOBALS->icicle->find("Model");
	int ntris = node->num_vertices / 3;
	for (int i = 0; i < ntris; i++) {
		Icicle_Particle p;
		const float min_vel = 0.015f;
		const float max_vel = 0.025f;
		p.velocity.x = min_vel + util::rand(0, 1000) / 1000.0f * (max_vel - min_vel) * (util::rand(0, 1) == 0 ? -1.0f : 1.0f);
		p.velocity.y = min_vel + util::rand(0, 1000) / 1000.0f * (max_vel - min_vel);
		p.velocity.z = min_vel + util::rand(0, 1000) / 1000.0f * (max_vel - min_vel) * (util::rand(0, 1) == 0 ? -1.0f : 1.0f);
		icicle_particles.push_back(p);
	}

	return icicle_particles;
}

void vIce_Step::draw_icicle(util::Point<float> target, std::vector<Icicle_Particle> icicle_particles)
{
	gfx::Model::Node *node = M3_GLOBALS->icicle->find("Model");

	float main_length = sqrtf(75.0f*75.0f + 75.0f*75.0f) * shim::scale;

	target = target * shim::scale;

	glm::mat4 save_mv, save_p;
	gfx::get_matrices(save_mv, save_p);

	// scales adjust for screen_offset
	float scale_x = (shim::screen_size.w*shim::scale)/shim::real_screen_size.w;
	float scale_y = (shim::screen_size.h*shim::scale)/shim::real_screen_size.h;

	float aspect = shim::screen_size.w / (float)shim::screen_size.h;
	glm::mat4 _proj = glm::perspective(float(M_PI/4.0f), aspect, 1.0f, 1000.0f);

	glm::mat4 _mv;

	if (shim::screen_offset.x > 0.0f) {
		_mv = glm::scale(_mv, glm::vec3(scale_x, 1.0f, 1.0f));
	}
	if (shim::screen_offset.y > 0.0f) {
		_mv = glm::scale(_mv, glm::vec3(1.0f, scale_y, 1.0f));
	}

	_mv = glm::translate(_mv, glm::vec3(0.0f, 0.0f, -2.0f));
	
	gfx::enable_depth_write(true);
	gfx::clear_depth_buffer(1.0f);
	gfx::enable_depth_test(true);

	// Get the length of the icicle in screen pixels
	glm::vec3 top(0.0f, 0.0f, 0.0f);
	glm::vec3 bottom(0.0f, 1.0f, 0.0f);
	top = glm::project(top, _mv, _proj, glm::vec4(0.0f, 0.0f, (float)shim::real_screen_size.w, (float)shim::real_screen_size.h));
	bottom = glm::project(bottom, _mv, _proj, glm::vec4(0.0f, 0.0f, (float)shim::real_screen_size.w, (float)shim::real_screen_size.h));
	float length = fabsf(bottom.y - top.y);
	float extra_length = length * 1.5f; // explosion lasts 0.5f of length (so the end (1.0) will be finished exactly at the end)

	glm::vec3 start_unproj = glm::unProject(glm::vec3(target.x+cos(M_PI/4.0f)*main_length+shim::screen_offset.x, target.y-sin(M_PI/4.0f)*main_length+shim::screen_offset.y, 0.0f), _mv, _proj, glm::vec4(0, 0, shim::real_screen_size.w, shim::real_screen_size.h));

	start_unproj.x *= 2.0f;
	start_unproj.y *= -2.0f;
	start_unproj.z = 0.0f;

	glm::vec3 target_unproj = glm::unProject(glm::vec3(target.x-cos(M_PI/4.0f)*extra_length+shim::screen_offset.x, target.y+sin(M_PI/4.0f)*extra_length+shim::screen_offset.y, 0.0f), _mv, _proj, glm::vec4(0, 0, shim::real_screen_size.w, shim::real_screen_size.h));

	target_unproj.x *= 2.0f;
	target_unproj.y *= -2.0f;
	target_unproj.z = 0.0f;

	const int FALL = 2500;
	Uint32 ticks2 = GET_TICKS() - icicle_start;
	float p2 = ticks2 / (float)FALL;

	glm::vec3 interp = start_unproj + (target_unproj - start_unproj) * p2;
	_mv = glm::translate(_mv, glm::vec3(interp.x, interp.y+0.5f/*0.5f to align the tip instead of center (icicle is 1.0f tall)*/, interp.z));


	// Rotate it 45 degrees
	_mv = glm::translate(_mv, glm::vec3(0.0f, -0.5f, 0.0f));
	_mv = glm::rotate(_mv, float(-M_PI/4.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	_mv = glm::translate(_mv, glm::vec3(0.0f, 0.5f, 0.0f));

	// spin
	glm::mat4 rotation_matrix = glm::rotate(glm::mat4(), float(M_PI*2.0f*p2*2.5f), glm::vec3(0.0f, 1.0f, 0.0f));
	_mv *= rotation_matrix;

	gfx::Shader *bak_shader = shim::current_shader;
	shim::current_shader = M3_GLOBALS->lit_3d_shader;
	shim::current_shader->use();

	float light_pos[3] = { -1000000.0f, 0.0f, 500000.0f };
	shim::current_shader->set_float_vector("light_pos", 3, light_pos, 1);
	shim::current_shader->set_colour("ground_colour", static_cast<Monster_RPG_3_Battle_Game *>(BATTLE)->get_tint());
	glm::mat4 it_rotation_matrix = glm::inverseTranspose(rotation_matrix);
	shim::current_shader->set_matrix("normal_matrix", it_rotation_matrix);

	glm::mat4 identity;
	gfx::set_matrices(identity, _proj);
	gfx::update_projection();

	// explosion
	float full_length = main_length + extra_length;
	float explosion_start = main_length / full_length;

	float p_per_frame = p2 / nframes;

	gfx::Model::Node *root = node;
	while (root && root->parent != 0) {
		root = root->parent;
	}
	glm::mat4 root_transform = root->transform;
	root->transform = glm::mat4();

	int ntris = node->num_vertices / 3;
	float elapsed = p2 - explosion_start;
	float inv = 1.0f - explosion_start; // explosion length percentage
	int explosion_frames = inv / p_per_frame;
	int explosion_frames_per_tri = explosion_frames / 3;
	float p3 = elapsed / inv; // percent of explosion done
	int num_tris_in_explosion = MAX(0, MIN(ntris, p3 / 0.667f * ntris));

	if (id == count-1 && played_sound == false && num_tris_in_explosion > 0) {
		played_sound = true;
		M3_GLOBALS->vice_sample->play(false);
	}

	if (last_ticks != ticks2) {
		for (int i = 0; i < num_tris_in_explosion; i++) {
			float f = (float)i / ntris;
			float this_start = f / 1.5f;
			float this_p = (p3 - this_start) / 0.333f;
			float alpha = MAX(0.0f, 1.0f - this_p);

			float *v1 = &node->vertices[(i * 3) * 12 + 0];
			float *v2 = &node->vertices[(i * 3) * 12 + 12];
			float *v3 = &node->vertices[(i * 3) * 12 + 24];
			float *v2_1 = &node->animated_vertices[(i * 3) * 12 + 0];
			float *v2_2 = &node->animated_vertices[(i * 3) * 12 + 12];
			float *v2_3 = &node->animated_vertices[(i * 3) * 12 + 24];
			
			v2_1[8] = v1[8];
			v2_1[9] = v1[9];
			v2_1[10] = v1[10];
			v2_1[11] = v1[11];
			v2_2[8] = v2[8];
			v2_2[9] = v2[9];
			v2_2[10] = v2[10];
			v2_2[11] = v2[11];
			v2_3[8] = v3[8];
			v2_3[9] = v3[9];
			v2_3[10] = v3[10];
			v2_3[11] = v3[11];

			// skip computations if invisible
			if (alpha > 0.0f) {
				int frame = this_p * explosion_frames_per_tri;
				glm::vec4 glmv1(v1[0], v1[1], v1[2], 1.0f);
				glm::vec4 glmv2(v2[0], v2[1], v2[2], 1.0f);
				glm::vec4 glmv3(v3[0], v3[1], v3[2], 1.0f);
				glm::mat4 partial_mv;
				if (shim::screen_offset.x > 0.0f) {
					partial_mv = glm::scale(partial_mv, glm::vec3(scale_x, 1.0f, 1.0f));
				}
				if (shim::screen_offset.y > 0.0f) {
					partial_mv = glm::scale(partial_mv, glm::vec3(1.0f, scale_y, 1.0f));
				}
				partial_mv = glm::translate(partial_mv, glm::vec3(0.0f, 0.0f, -2.0f));
				float p4 = inv * this_start + explosion_start;
				glm::vec3 interp = start_unproj + (target_unproj - start_unproj) * p4;
				partial_mv = glm::translate(partial_mv, glm::vec3(interp.x, interp.y+0.5f/*0.5f to align the tip instead of center (icicle is 1.0f tall)*/, interp.z));
				partial_mv = glm::translate(partial_mv, glm::vec3(0.0f, -0.5f, 0.0f));
				partial_mv = glm::rotate(partial_mv, float(-M_PI/4.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				partial_mv = glm::translate(partial_mv, glm::vec3(0.0f, 0.5f, 0.0f));
				glm::mat4 rotation_matrix = glm::rotate(glm::mat4(), float(M_PI*2.0f*p4*2.5f), glm::vec3(0.0f, 1.0f, 0.0f));
				partial_mv *= rotation_matrix;
				glmv1 = partial_mv * root_transform * glmv1;
				glmv2 = partial_mv * root_transform * glmv2;
				glmv3 = partial_mv * root_transform * glmv3;

				v2_1[0] = glmv1.x;
				v2_1[1] = glmv1.y;
				v2_1[2] = glmv1.z;
				v2_2[0] = glmv2.x;
				v2_2[1] = glmv2.y;
				v2_2[2] = glmv2.z;
				v2_3[0] = glmv3.x;
				v2_3[1] = glmv3.y;
				v2_3[2] = glmv3.z;
				
				v2_1[0] += icicle_particles[i].velocity.x * frame;
				v2_1[1] += icicle_particles[i].velocity.y * frame;
				v2_1[2] += icicle_particles[i].velocity.z * frame;
				v2_2[0] += icicle_particles[i].velocity.x * frame;
				v2_2[1] += icicle_particles[i].velocity.y * frame;
				v2_2[2] += icicle_particles[i].velocity.z * frame;
				v2_3[0] += icicle_particles[i].velocity.x * frame;
				v2_3[1] += icicle_particles[i].velocity.y * frame;
				v2_3[2] += icicle_particles[i].velocity.z * frame;
			}

			// colour
			v2_1[8] *= alpha;
			v2_1[9] *= alpha;
			v2_1[10] *= alpha;
			v2_1[11] *= alpha;
			v2_2[8] *= alpha;
			v2_2[9] *= alpha;
			v2_2[10] *= alpha;
			v2_2[11] *= alpha;
			v2_3[8] *= alpha;
			v2_3[9] *= alpha;
			v2_3[10] *= alpha;
			v2_3[11] *= alpha;
		}
		for (int i = num_tris_in_explosion; i < ntris; i++) {
			float *v1 = &node->vertices[(i * 3) * 12 + 0];
			float *v2 = &node->vertices[(i * 3) * 12 + 12];
			float *v3 = &node->vertices[(i * 3) * 12 + 24];
			float *v2_1 = &node->animated_vertices[(i * 3) * 12 + 0];
			float *v2_2 = &node->animated_vertices[(i * 3) * 12 + 12];
			float *v2_3 = &node->animated_vertices[(i * 3) * 12 + 24];
			glm::vec4 glmv1(v1[0], v1[1], v1[2], 1.0f);
			glm::vec4 glmv2(v2[0], v2[1], v2[2], 1.0f);
			glm::vec4 glmv3(v3[0], v3[1], v3[2], 1.0f);
			glmv1 = _mv * root_transform * glmv1;
			glmv2 = _mv * root_transform * glmv2;
			glmv3 = _mv * root_transform * glmv3;
			
			v2_1[0] = glmv1.x;
			v2_1[1] = glmv1.y;
			v2_1[2] = glmv1.z;
			v2_2[0] = glmv2.x;
			v2_2[1] = glmv2.y;
			v2_2[2] = glmv2.z;
			v2_3[0] = glmv3.x;
			v2_3[1] = glmv3.y;
			v2_3[2] = glmv3.z;

			// colour
			v2_1[8] = v1[8];
			v2_1[9] = v1[9];
			v2_1[10] = v1[10];
			v2_1[11] = v1[11];
			v2_2[8] = v2[8];
			v2_2[9] = v2[9];
			v2_2[10] = v2[10];
			v2_2[11] = v2[11];
			v2_3[8] = v3[8];
			v2_3[9] = v3[9];
			v2_3[10] = v3[10];
			v2_3[11] = v3[11];
		}
	
		nframes++;
	}

	last_ticks = ticks2;

	gfx::Vertex_Cache::instance()->start();
	gfx::Vertex_Cache::instance()->cache_3d_immediate(node->animated_vertices, node->num_triangles);
	
	root->transform = root_transform;

	gfx::enable_depth_write(false);
	gfx::enable_depth_test(false);

	shim::current_shader = bak_shader;
	shim::current_shader->use();

	gfx::set_matrices(save_mv, save_p);
	gfx::update_projection();
}
