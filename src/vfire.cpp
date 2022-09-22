#include <Nooskewl_Wedge/battle_enemy.h>
#include <Nooskewl_Wedge/battle_game.h>
#include <Nooskewl_Wedge/globals.h>

#include "achievements.h"
#include "fire.h"
#include "globals.h"
#include "spells.h"
#include "vfire.h"

static void spell_effect_callback(void *data)
{
	vFire_Step *v = static_cast<vFire_Step *>(data);
	v->count_fireballs();
}

static void model_callback(void *data)
{
	vFire_Step *v = static_cast<vFire_Step *>(data);
	v->model_done();
}

vFire_Step::vFire_Step(wedge::Task *task) :
	wedge::Step(task),
	prev_frame(0),
	wings_played(0),
	fireballs(0),
	fireballs_done(0),
	_model_done(false)
{
	wings = new audio::Sample("wings.ogg");
}

vFire_Step::~vFire_Step()
{
}

bool vFire_Step::run()
{
	int frame = M3_GLOBALS->dragon->get_current_frame();

	if (frame % 40 >= 20 && frame / 40 >= wings_played) {
		wings->play_stretched(10.0f, 0, audio::millis_to_samples(30*16)/*16 millis per frame @ 60 fps*/, audio::SAMPLE_TYPE_SFX);
		wings_played++;
	}

	wedge::Battle_Entity *gayan = BATTLE->get_enemies()[0];

	if (frame >= 240 && fireballs == 0) {
		fireballs++;
		SPELLS->play_sound("Fire");
		std::vector<wedge::Battle_Entity *> targets;
		targets.push_back(gayan);
		std::vector<wedge::Step *> v = SPELLS->start_effect("Fire", targets, spell_effect_callback, this);
		Fire_Step *f = static_cast<Fire_Step *>(v[0]);
		f->double_particles();
		util::Point<float> start_pos = get_mouth_pos();
		f->set_start_pos(start_pos);
		wedge::Battle_Enemy *e = static_cast<wedge::Battle_Enemy *>(gayan);
		util::Point<float> end_pos = e->get_position() + util::Point<int>(14, 32);
		f->set_end_pos(end_pos);
		f->set_start_angle((end_pos-start_pos).angle());
	}
	else if (frame >= 290 && fireballs == 1) {
		fireballs++;
		SPELLS->play_sound("Fire");
		std::vector<wedge::Battle_Entity *> targets;
		targets.push_back(gayan);
		std::vector<wedge::Step *> v = SPELLS->start_effect("Fire", targets, spell_effect_callback, this);
		Fire_Step *f = static_cast<Fire_Step *>(v[0]);
		f->double_particles();
		util::Point<float> start_pos = get_mouth_pos();
		f->set_start_pos(start_pos);
		wedge::Battle_Enemy *e = static_cast<wedge::Battle_Enemy *>(gayan);
		util::Point<float> end_pos = e->get_position() + util::Point<int>(22, 47);
		f->set_end_pos(end_pos);
		f->set_start_angle((end_pos-start_pos).angle());
	}
	else if (frame >= 340 && fireballs == 2) {
		fireballs++;
		SPELLS->play_sound("Fire");
		std::vector<wedge::Battle_Entity *> targets;
		targets.push_back(gayan);
		std::vector<wedge::Step *> v = SPELLS->start_effect("Fire", targets, spell_effect_callback, this);
		Fire_Step *f = static_cast<Fire_Step *>(v[0]);
		f->double_particles();
		util::Point<float> start_pos = get_mouth_pos();
		f->set_start_pos(start_pos);
		wedge::Battle_Enemy *e = static_cast<wedge::Battle_Enemy *>(gayan);
		util::Point<float> end_pos = e->get_position() + util::Point<int>(29, 37);
		f->set_end_pos(end_pos);
		f->set_start_angle((end_pos-start_pos).angle());
	}

	prev_frame = frame;

	if (fireballs_done == 3) {
		util::achieve((void *)ACHIEVE_MERCY);
		send_done_signal();
		return false;
	}
	else {
		return true;
	}
}

void vFire_Step::get_mvp(glm::mat4 &mv, glm::mat4 &p)
{
	// scales adjust for screen_offset
	float scale_x = (shim::screen_size.w*shim::scale)/shim::real_screen_size.w;
	float scale_y = (shim::screen_size.h*shim::scale)/shim::real_screen_size.h;
	float eye_x = 0, eye_y, eye_z = 25;
	float centre_z = 0;

	float centre_x;

	int frame = M3_GLOBALS->dragon->get_current_frame();

	if (frame < 210) {
		float p = frame / 209.0f;
		centre_x = -4.5f*scale_x + -50.0f*scale_x*(1.0f-p);
		eye_y = -25.0f * (1.0f - p) * scale_y;
	}
	else if (frame >= 390) {
		float p = (frame-390) / 209.0f;
		centre_x = -4.5f*scale_x + 50.0f*scale_x*p;
		eye_y = -25.0f * p * scale_y;
	}
	else {
		centre_x = -4.5f*scale_x;
		eye_y = 0;
	}

	int m = frame % 40;
	if (m < 20) {
		float p = m / 19.0f;
		p -= 0.5f;
		eye_y += 1.0f * scale_y * p;
	}
	else {
		float p = (m - 20) / 19.0f;
		p -= 0.5f;
		eye_y += -1.0f * scale_y * p;
	}

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
	if (shim::allow_dpad_below && shim::screen_offset.y != 0) {
		float v;
		v = (shim::real_screen_size.h - (shim::screen_size.h*shim::scale)) / 2.0f;
		v /= shim::screen_offset.y;
		mv = glm::translate(mv, glm::vec3(0.0f, (float)v, 0.0f));
	}
}

void vFire_Step::draw_dragon()
{
	if (_model_done) {
		return;
	}

	glm::mat4 old_mv, old_p;
	gfx::get_matrices(old_mv, old_p);

	std::vector<gfx::Model::Node *> nodes = M3_GLOBALS->dragon->get_nodes();
	
	glm::mat4 mv, _p;

	get_mvp(mv, _p);

	gfx::set_matrices(mv, _p);
	gfx::update_projection();

	gfx::enable_depth_write(true);
	gfx::clear_depth_buffer(1.0f);
	gfx::enable_depth_test(true);
	gfx::set_cull_mode(gfx::NO_FACE); // its wings need to be drawn on both sides

	M3_GLOBALS->dragon->draw_textured();
	
	gfx::set_matrices(old_mv, old_p);
	gfx::update_projection();
	
	gfx::set_cull_mode(gfx::BACK_FACE);
	gfx::enable_depth_test(false);
}

void vFire_Step::draw_fore()
{
	draw_dragon();
}

void vFire_Step::start()
{
	M3_GLOBALS->dragon->set_animation("vFire", model_callback, this);
	M3_GLOBALS->dragon->reset();
	M3_GLOBALS->dragon->start();
}

util::Point<float> vFire_Step::get_mouth_pos()
{
	gfx::Model::Node *n = M3_GLOBALS->dragon->find("Model");
	float *v = n->animated_vertices;
	const int mouth_vert = 799; // examined the model (dragon.x) to find the index of this vertex inside the mouth

	// FIXME if vertex data grows more than 12 floats
	glm::vec3 mouth(v[mouth_vert*12+0], v[mouth_vert*12+1], v[mouth_vert*12+2]);
	glm::mat4 mv, _p;
	get_mvp(mv, _p);
	mouth = glm::project(mouth, mv, _p, glm::vec4(0.0f, 0.0f, (float)shim::real_screen_size.w, (float)shim::real_screen_size.h));
	// don't just use shim::screen_size above as it has to account for black bars
	mouth.x -= shim::screen_offset.x;
	//mouth.y -= shim::screen_offset.y;
	mouth.y -= shim::real_screen_size.h - shim::screen_size.h*shim::scale - shim::screen_offset.y;
	mouth.x /= shim::scale;
	mouth.y /= shim::scale;
	mouth.y = shim::screen_size.h - mouth.y;

	return util::Point<float>(mouth.x, mouth.y);
}

void vFire_Step::count_fireballs()
{
	fireballs_done++;
}

void vFire_Step::model_done()
{
	_model_done = true;
}
