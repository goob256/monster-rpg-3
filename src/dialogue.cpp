#include "Nooskewl_Wedge/area.h"
#include "Nooskewl_Wedge/area_game.h"
#include "Nooskewl_Wedge/general.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/player_input.h"
#include "Nooskewl_Wedge/map_entity.h"
#include "Nooskewl_Wedge/omnipresent.h"

#include "dialogue.h"
#include "general.h"
#include "globals.h"

static util::Size<int> get_pad()
{
	util::Size<int> pad;
	pad.w = shim::screen_size.w * 0.03f;
	pad.h = shim::screen_size.h * 0.03f;
	return pad;
}

void Dialogue_Step::get_positions(int *HEIGHT, int *indicator_height, int *y, util::Size<int> *PAD, util::Point<int> *text_pos, wedge::Dialogue_Position position)
{
	if (HEIGHT) {
		*HEIGHT = shim::font->get_height() * 2 + Dialogue_Step::BORDER * 2 + 2;
	}
	if (indicator_height) {
		*indicator_height = 5;
	}
	if (PAD) {
		*PAD = get_pad();
	}
	if (y) {
		if (position == wedge::DIALOGUE_TOP) {
			*y = PAD->h;
		}
		else {
			*y = shim::screen_size.h - 1 - PAD->h - *HEIGHT;
		}
	}
	if (text_pos) {
		*text_pos = util::Point<int>(PAD->w + Dialogue_Step::BORDER, *y + Dialogue_Step::BORDER);
	}
}

Dialogue_Step::Dialogue_Step(std::string tag, std::string text, wedge::Dialogue_Type type, wedge::Dialogue_Position position, wedge::Task *task, bool darken_screen) :
	wedge::Step(task),
	tag(tag),
	text(text),
	type(type),
	position(position),
	start_character(0),
	done(false),
	fade_out_start_time(0),
	dismissable(true),
	count(0),
	sent_done(false),
	darken_screen(darken_screen)
{
}

void Dialogue_Step::start()
{
	if (type == wedge::DIALOGUE_MESSAGE) {
		started_time = -1;
	}
	else {
		started_time = GET_TICKS();
	}
	started_time_transition = GET_TICKS();

	tag_width = MAX(0, GLOBALS->bold_font->get_text_width(tag));

	in_battle = BATTLE != NULL;
	wedge::Area *area = AREA->get_current_area();

	if (position == wedge::DIALOGUE_AUTO) {
		if (in_battle) {
			position = wedge::DIALOGUE_BOTTOM;
		}
		else {
			wedge::Map_Entity *player = AREA->get_player(0);
			util::Point<float> entity_position = player->get_position();
			util::Point<float> entity_offset = player->get_offset();

			util::Point<float> entity_pos;
			util::Point<float> centre;

			entity_pos = (entity_offset + entity_position) * shim::tile_size + shim::tile_size / 2;

			util::Size<float> half_screen = shim::screen_size;
			half_screen /= 2.0f;

			centre = -entity_pos + half_screen;

			gfx::Tilemap *tilemap = area->get_tilemap();

			util::Size<int> tilemap_size = tilemap->get_size() * shim::tile_size;

			if (tilemap_size.h < shim::screen_size.h) {
				util::Point<float> pos = player->get_position();
				util::Size<float> sz = tilemap->get_size();
				float o = pos.y - ((sz.h-1)/2);
				if (o == 0) {
					if (player->get_direction() == wedge::DIR_S) {
						position = wedge::DIALOGUE_TOP;
					}
					else {
						position = wedge::DIALOGUE_BOTTOM;
					}
				}
				else if (o < 0) {
					position = wedge::DIALOGUE_BOTTOM;
				}
				else {
					position = wedge::DIALOGUE_TOP;
				}
			}
			else {
				if (centre.y > 0.0f) {
					position = wedge::DIALOGUE_BOTTOM;
				}
				else if (entity_pos.y + half_screen.h > tilemap_size.h) {
					position = wedge::DIALOGUE_TOP;
				}
				else {
					if (player->get_direction() == wedge::DIR_S) {
						position = wedge::DIALOGUE_TOP;
					}
					else {
						position = wedge::DIALOGUE_BOTTOM;
					}
				}
			}
		}
	}

	if (in_battle == false) {
		wedge::pause_player_input(true);

		entity_movement_system = area->get_entity_movement_system();
		entity_movement_was_paused = entity_movement_system->is_paused();

		std::vector<Dialogue_Step *> dialogues = active_dialogues(task->get_system()->get_game());
		for (size_t i = 0; i < dialogues.size(); i++) {
			Dialogue_Step *d = dialogues[i];
			if (d != this && d->is_initialised()) {
				entity_movement_was_paused = d->get_entity_movement_was_paused();
				break;
			}
		}

		entity_movement_system->set_paused(true);

		std::list<wedge::Map_Entity *> &entities = area->get_entities();
		for (std::list<wedge::Map_Entity *>::iterator it = entities.begin(); it != entities.end(); it++) {
			wedge::Map_Entity *entity = *it;
			gfx::Sprite *sprite = entity->get_sprite();
			if (sprite) {
				if (sprite->is_started()) {
					sprite->stop();
					unpause.push_back(sprite);
				}
			}
		}
		wedge::Map_Entity_Input_Step *input_step = AREA->get_player(0)->get_input_step();
		if (input_step != NULL) {
			input_step->end_movement();
		}
	}
}

Dialogue_Step::~Dialogue_Step()
{
}

bool Dialogue_Step::run()
{
	bool ret = !(done && GET_TICKS() >= fade_out_start_time+FADE_TIME);
	return ret;
}

void Dialogue_Step::handle_event(TGUI_Event *event)
{
	if (done) {
		return;
	}

	std::string t = text.substr(util::utf8_len_bytes(text, start_character));
	bool full;
	int num_lines;
	int width;
	util::Size<int> PAD = get_pad();
	int num_chars = shim::font->draw_wrapped(shim::palette[20], t, text_pos, shim::screen_size.w-PAD.w*2-BORDER*2-1/*-1 for shadow*/, shim::font->get_height()+2, 2, (started_time < 0 ? -1 : GET_TICKS()-started_time), DELAY, true, full, num_lines, width, true, false, tag_width);

	bool next = false;

	if (event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_b1 && event->keyboard.is_repeat == false) {
		next = true;
	}
	else if (event->type == TGUI_MOUSE_DOWN && event->mouse.is_repeat == false) {
		next = true;
	}
	if (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_b1 && event->joystick.is_repeat == false) {
		next = true;
	}

	if (next && sent_done == false) {
		if (full) {
			if (num_chars+start_character == (int)util::utf8_len(text)) {
				M3_GLOBALS->button->play(false);
				if (dismissable) {
					dismiss();
				}
				send_done_signal();
				sent_done = true;
			}
			else {
				M3_GLOBALS->button->play(false);
				start_character += num_chars;
				if (type == wedge::DIALOGUE_MESSAGE) {
					started_time = -1;
				}
				else {
					started_time = GET_TICKS();
				}
			}
		}
		else {
			started_time = -1;
		}
	}
	
	if (done == false && BATTLE == NULL) {
		wedge::Map_Entity *eny = AREA->get_player(ENY);
		wedge::Player_Input_Step *pis = static_cast<wedge::Player_Input_Step *>(eny->get_input_step());
		pis->handle_event(event);
	}
}

void Dialogue_Step::draw_fore()
{
	// Every even, hook, every odd, draw (causes draws to happen in Omnipresent::draw_fore)
	int mod = count % 2;
	count++;
	if (mod == 0) {
		OMNIPRESENT->hook_draw_fore(this);
		return;
	}

	gfx::Font::end_batches();

	if (darken_screen) {
		SDL_Colour colour = shim::black;
		colour.r *= 0.75;
		colour.g *= 0.75;
		colour.b *= 0.75;
		colour.a *= 0.75;
		gfx::draw_filled_rectangle(colour, util::Point<int>(0, 0), shim::screen_size);
	}

	int HEIGHT, indicator_height, y;
	util::Size<int> PAD;
	util::Point<int> text_pos;

	get_positions(&HEIGHT, &indicator_height, &y, &PAD, &text_pos, position);
	HEIGHT += indicator_height;

	if (position == wedge::DIALOGUE_BOTTOM) {
		y -= indicator_height;
		text_pos.y -= indicator_height;
	}

	float alpha;
	float offset;
	Uint32 now = GET_TICKS();
	if (done) {
		float p = (now - fade_out_start_time) / (float)FADE_TIME;
		p = p * p;
		alpha = MAX(0.0f, 1.0f - p);
		offset = 0.0f;
	}
	else {
		alpha = 1.0f;
		float max_offset;
		if (position == wedge::DIALOGUE_TOP) {
			max_offset = -(PAD.h + HEIGHT);
		}
		else {
			max_offset = PAD.h + HEIGHT;
		}
		float p;
		if (start_character != 0) {
			p = 1.0f;
		}
		else {
			p = MIN(1.0f, (now - started_time_transition) / (float)FADE_TIME);
		}
		p = p * p;
		offset = (1.0f - p) * max_offset;
	}

	// set up projection

	glm::mat4 modelview, proj;
	gfx::get_matrices(modelview, proj);
	glm::mat4 mv = glm::translate(modelview, glm::vec3(0.0f, offset, 0.0f));

	// draw window

	float little = get_little_bander_offset();

	SDL_Colour *colours;
	SDL_Colour colour;
	SDL_Colour border_colour;
	if (type == wedge::DIALOGUE_SPEECH) {
		colour = shim::palette[20];
		colours = start_bander(num_bands(HEIGHT-4), shim::palette[17], shim::palette[19]);
		border_colour = shim::palette[17];
	}
	else {
		colour = shim::palette[20];
		colours = start_bander(num_bands(HEIGHT-4), shim::palette[24], shim::palette[22]);
		border_colour = shim::palette[24];
	}
	shim::current_shader->set_float("alpha", alpha);
	gfx::set_matrices(mv, proj);
	gfx::update_projection();

	/*
	gfx::enable_depth_test(true);
	gfx::enable_depth_write(true);
	gfx::clear_depth_buffer(1.0f);
	gfx::set_depth_mode(gfx::COMPARE_LESS);
	*/

	gfx::draw_filled_rectangle(colours, util::Point<float>(PAD.w+2-little, y+2-little), util::Size<float>(shim::screen_size.w-PAD.w*2-4+little*2.0f, HEIGHT-4+little*2.0f));	
	end_bander();

	// draw border and text

	gfx::draw_rectangle(make_translucent(border_colour, alpha), util::Point<int>(PAD.w, y)+util::Point<int>(1, 1), util::Size<int>(shim::screen_size.w-PAD.w*2, HEIGHT)-util::Size<int>(2, 2));
	gfx::draw_rectangle(make_translucent(shim::palette[27], alpha), util::Point<int>(PAD.w, y), util::Size<int>(shim::screen_size.w-PAD.w*2, HEIGHT));

	/*
	gfx::set_depth_mode(gfx::COMPARE_LESSEQUAL);
	gfx::enable_depth_test(false);
	*/

	shim::current_shader = M3_GLOBALS->alpha_shader;
	shim::current_shader->use();
	shim::current_shader->set_float("alpha", alpha);

	gfx::set_matrices(mv, proj);
	gfx::update_projection();

	GLOBALS->bold_font->enable_shadow(border_colour, gfx::Font::DROP_SHADOW);
	GLOBALS->bold_font->draw(colour, tag, text_pos);
	GLOBALS->bold_font->disable_shadow();
	std::string t = text.substr(util::utf8_len_bytes(text, start_character));
	bool full;
	int num_lines;
	int width;
	shim::font->enable_shadow(border_colour, gfx::Font::DROP_SHADOW);
	int num_drawn = shim::font->draw_wrapped(colour, t, text_pos, shim::screen_size.w-PAD.w*2-BORDER*2-1/*-1 for shadow*/, shim::font->get_height()+2, 2, (started_time < 0 ? -1 : GET_TICKS()-started_time), DELAY, false, full, num_lines, width, true, false, tag_width);
	shim::font->disable_shadow();

	util::Point<float> draw_pos(shim::screen_size.w-PAD.w-M3_GLOBALS->down_arrow->size.w-3, y+HEIGHT-M3_GLOBALS->down_arrow->size.h-4);
	if (num_drawn+start_character < util::utf8_len(text)) {
		const int anim_time = 1000;
		Uint32 ticks = GET_TICKS() % anim_time;
		if (ticks < anim_time/2) {
			draw_pos += util::Point<int>(0, 1);
		}
		M3_GLOBALS->down_arrow->draw_tinted(border_colour, draw_pos+util::Point<int>(0, 1));
		M3_GLOBALS->down_arrow->draw_tinted(border_colour, draw_pos+util::Point<int>(1, 0));
		M3_GLOBALS->down_arrow->draw_tinted(border_colour, draw_pos+util::Point<int>(1, 1));
		M3_GLOBALS->down_arrow->draw_tinted(colour, draw_pos);
	}
	else {
		M3_GLOBALS->nomore->draw_tinted(border_colour, draw_pos+util::Point<int>(0, 1));
		M3_GLOBALS->nomore->draw_tinted(border_colour, draw_pos+util::Point<int>(1, 0));
		M3_GLOBALS->nomore->draw_tinted(border_colour, draw_pos+util::Point<int>(1, 1));
		M3_GLOBALS->nomore->draw_tinted(colour, draw_pos);
	}

	gfx::Font::end_batches();
	
	shim::current_shader = shim::default_shader;
	shim::current_shader->use();
	gfx::set_matrices(modelview, proj);
	gfx::update_projection();
}

void Dialogue_Step::set_dismissable(bool dismissable)
{
	this->dismissable = dismissable;
}

void Dialogue_Step::dismiss()
{
	if (in_battle == false) {
		wedge::pause_player_input(false);

		entity_movement_system->set_paused(entity_movement_was_paused);
		
		for (std::list<gfx::Sprite *>::iterator it = unpause.begin(); it != unpause.end(); it++) {
			gfx::Sprite *sprite = *it;
			sprite->start();
		}
	}
	done = true;
	fade_out_start_time = GET_TICKS();
	disown();
}

bool Dialogue_Step::get_entity_movement_was_paused()
{
	return entity_movement_was_paused;
}
