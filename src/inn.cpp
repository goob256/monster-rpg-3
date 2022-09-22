#include <Nooskewl_Wedge/a_star.h>
#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/check_positions.h>
#include <Nooskewl_Wedge/delay.h>
#include <Nooskewl_Wedge/fade.h>
#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/generic_callback.h>
#include <Nooskewl_Wedge/generic_immediate_callback.h>
#include <Nooskewl_Wedge/generic_gui.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/map_entity.h>
#include <Nooskewl_Wedge/pause_presses.h>
#include <Nooskewl_Wedge/play_music.h>
#include <Nooskewl_Wedge/play_sound.h>
#include <Nooskewl_Wedge/set_animation.h>
#include <Nooskewl_Wedge/set_direction.h>
#include <Nooskewl_Wedge/set_solid.h>
#include <Nooskewl_Wedge/stats.h>
#include <Nooskewl_Wedge/stop_music.h>
#include <Nooskewl_Wedge/wait.h>

#include "dialogue.h"
#include "general.h"
#include "globals.h"
#include "gui.h"
#include "inn.h"
#include "question.h"
#include "widgets.h"

static void set_following(void *data)
{
	Inn_Step *inn = static_cast<Inn_Step *>(data);
	INSTANCE->party_following_player = true;
	inn->set_done(true);
}

static void callback2(void *data)
{
	Inn_Step::Callback_Data *dat = static_cast<Inn_Step::Callback_Data *>(data);
	dat->inn_step->reposition_players();
	INSTANCE->party_following_player = false;
	dat->multiple_choice_gui_step->set_done(true);
}

static void callback(void *data)
{
	Multiple_Choice_GUI::Callback_Data *d = static_cast<Multiple_Choice_GUI::Callback_Data *>(data);
	Inn_Step::Callback_Data *dat = static_cast<Inn_Step::Callback_Data *>(d->userdata);
	
	wedge::Map_Entity *player = AREA->get_player(ENY);
	wedge::Map_Entity *tiggy = AREA->get_player(TIGGY);

	if (d->choice == 1) {
		dat->multiple_choice_gui_step->set_done(true);
	}
	else {
		dat->count++;
		// temporarily move them out of bed so when loaded they'll be there
		util::Point<int> player_pos = player->get_position();
		util::Point<int> tiggy_pos = tiggy->get_position();
		dat->inn_step->set_player_positions(player_pos, tiggy_pos);
		player->set_position(util::Point<int>(player_pos.x+1, player_pos.y));
		tiggy->set_position(util::Point<int>(tiggy_pos.x-1, tiggy_pos.y));
		player->set_direction(wedge::DIR_S, true, false);
		tiggy->set_direction(wedge::DIR_S, true, false);
		player->set_solid(true);
		tiggy->set_solid(true);
		std::string message;
		Uint32 now = GET_TICKS();
		Uint32 played_time = now - INSTANCE->play_start;
		INSTANCE->play_time += (played_time / 1000);
		INSTANCE->play_start = now;
		NEW_SYSTEM_AND_TASK(AREA)
		wedge::Generic_Callback_Step *g = new wedge::Generic_Callback_Step(callback2, dat, new_task);
		ADD_STEP(g)
		ADD_TASK(new_task)
		FINISH_SYSTEM(AREA)
		INSTANCE->party_following_player = true;
		show_save_screen(g);
	}
}

Inn_Step::Inn_Step(wedge::Dialogue_Type type, std::string innkeep_name, std::string offer, std::string nope, std::string have_cash, std::string no_cash, int cost, util::Point<int> bed_middle_pos, wedge::Task *task, wedge::Step *monitor_step) :
	wedge::Step(task),
	done(false),
	signal_count(0),
	type(type),
	innkeep_name(innkeep_name),
	offer(offer),
	nope(nope),
	have_cash(have_cash),
	no_cash(no_cash),
	cost(cost),
	bed_middle_pos(bed_middle_pos),
	sent_done_signal(false),
	monitor_step(monitor_step),
	do_sleep(false)
{
}

Inn_Step::~Inn_Step()
{
}

void Inn_Step::start()
{
	std::vector<std::string> v;
	v.push_back(GLOBALS->game_t->translate(1729)/* Originally: Yes */);
	v.push_back(GLOBALS->game_t->translate(1730)/* Originally: No */);
	do_question(innkeep_name == "" ? "" : GLOBALS->game_t->translate(1086)/* Originally: Innkeep */ + TAG_END, offer, type, v, this);
}

bool Inn_Step::run()
{
	if (done) {
		send_done_signal();
		if (sent_done_signal == false && monitor_step != NULL) {
			sent_done_signal = true;
			monitor_step->done_signal(this);
		}
	}
	return done == false;
}

void Inn_Step::done_signal(wedge::Step *step)
{
	signal_count++;

	if (signal_count == 1) {
		Question_Step *q = dynamic_cast<Question_Step *>(step);
		if (q && q->get_choice() == 0) {
			if (cost <= 0 || INSTANCE->get_gold() >= cost) {
				INSTANCE->add_gold(-cost);
				do_sleep = true;
				if (have_cash == "") {
					done_signal(step);
				}
				else {
					wedge::globals->do_dialogue(innkeep_name == "" ? "" : GLOBALS->game_t->translate(1086)/* Originally: Innkeep */ + TAG_END, have_cash, type, wedge::DIALOGUE_AUTO, this);
				}
			}
			else {
				wedge::globals->do_dialogue(innkeep_name == "" ? "" : GLOBALS->game_t->translate(1086)/* Originally: Innkeep */ + TAG_END, no_cash, type, wedge::DIALOGUE_AUTO, this);
			}
		}
		else {
			if (nope != "") {
				wedge::globals->do_dialogue(innkeep_name == "" ? "" : GLOBALS->game_t->translate(1086)/* Originally: Innkeep */ + TAG_END, nope, type, wedge::DIALOGUE_AUTO, this);
			}
			else {
				done = true;
			}
		}
	}
	else if (do_sleep == false) {
		done = true;
	}
	else {
		if (monitor_step != NULL) {
			sent_done_signal = true;
			monitor_step->done_signal(this);
		}
		INSTANCE->party_following_player = false;
		for (size_t i = 0; i < MAX_PARTY; i++) {
			INSTANCE->stats[i].base.status = wedge::STATUS_OK;
			INSTANCE->stats[i].base.hp = INSTANCE->stats[i].base.fixed.max_hp;
			INSTANCE->stats[i].base.mp = INSTANCE->stats[i].base.fixed.max_mp;
		}

		std::string music_name;
		if (shim::music) {
			music_name = shim::music->get_name();
		}
		wedge::Map_Entity *player = AREA->get_player(ENY);
		wedge::Map_Entity *tiggy = AREA->get_player(TIGGY);
		NEW_SYSTEM_AND_TASK(AREA)
		wedge::Pause_Presses_Step *pp1 = new wedge::Pause_Presses_Step(true, false, new_task);
		wedge::Set_Solid_Step *sss1 = new wedge::Set_Solid_Step(player, false, new_task);
		wedge::A_Star_Step *as1 = new wedge::A_Star_Step(player, bed_middle_pos, new_task);
		wedge::A_Star_Step *as2 = new wedge::A_Star_Step(player, util::Point<int>(bed_middle_pos.x-1, bed_middle_pos.y), new_task);
		as2->set_check_solids(false);
		wedge::Set_Animation_Step *sa1 = new wedge::Set_Animation_Step(player, "sleep", new_task);
		std::vector<wedge::Map_Entity *> entities;
		entities.push_back(player);
		entities.push_back(tiggy);
		std::vector< util::Point<int> > positions;
		positions.push_back(util::Point<int>(bed_middle_pos.x-1, bed_middle_pos.y));
		positions.push_back(util::Point<int>(bed_middle_pos.x+1, bed_middle_pos.y));
		wedge::Check_Positions_Step *cps = new wedge::Check_Positions_Step(entities, positions, true, new_task);
		wedge::Stop_Music_Step *sms = new wedge::Stop_Music_Step(new_task);
		wedge::Play_Sound_Step *pss = new wedge::Play_Sound_Step(M3_GLOBALS->sleep, false, false, new_task);
		wedge::Fade_Step *fs1 = new wedge::Fade_Step(shim::black, true, 2000, new_task);
		wedge::Delay_Step *delay = new wedge::Delay_Step(1000, new_task);
		std::vector<std::string> choices;
		choices.push_back(GLOBALS->game_t->translate(1729)/* Originally: Yes */);
		choices.push_back(GLOBALS->game_t->translate(1730)/* Originally: No */);
		callback_data.count = 0;
		callback_data.inn_step = this;
		Positioned_Multiple_Choice_GUI *multiple_choice_gui = new Positioned_Multiple_Choice_GUI(false, GLOBALS->game_t->translate(1457)/* Originally: Save your game? */, choices, 1, 0, 0, 0, 0, 0, 0, 0.03f, 0.03f, callback, &callback_data);
		multiple_choice_gui->resize(shim::screen_size);
		wedge::Generic_GUI_Step *ggs = new wedge::Generic_GUI_Step(multiple_choice_gui, true, new_task);
		callback_data.multiple_choice_gui_step = ggs;
		wedge::Play_Music_Step *pms;
		if (music_name == "") {
			pms = NULL;
		}
		else {
			pms = new wedge::Play_Music_Step(music_name, new_task);
		}
		wedge::Fade_Step *fs2 = new wedge::Fade_Step(shim::black, false, 2000, new_task);
		wedge::A_Star_Step *as3 = new wedge::A_Star_Step(player, bed_middle_pos, new_task);
		wedge::Set_Direction_Step *sd1 = new wedge::Set_Direction_Step(player, wedge::DIR_S, true, false, new_task);
		wedge::Set_Solid_Step *sss2 = new wedge::Set_Solid_Step(player, true, new_task);
		wedge::Pause_Presses_Step *pp2 = new wedge::Pause_Presses_Step(false, true, new_task);
		ADD_STEP(pp1)
		ADD_STEP(sss1)
		ADD_STEP(as1)
		ADD_STEP(as2)
		ADD_STEP(sa1)
		ADD_STEP(cps)
		ADD_STEP(sms)
		ADD_STEP(pss)
		ADD_STEP(fs1)
		ADD_STEP(delay)
		ADD_STEP(ggs)
		if (pms != NULL) {
			ADD_STEP(pms)
		}
		ADD_STEP(fs2)
		ADD_STEP(as3)
		ADD_STEP(sd1)
		ADD_STEP(sss2)
		ADD_STEP(pp2)
		ADD_TASK(new_task)
		ANOTHER_TASK
		wedge::Set_Solid_Step *sss3 = new wedge::Set_Solid_Step(tiggy, false, new_task);
		wedge::A_Star_Step *as4 = new wedge::A_Star_Step(tiggy, bed_middle_pos, new_task);
		wedge::A_Star_Step *as5 = new wedge::A_Star_Step(tiggy, util::Point<int>(bed_middle_pos.x+1, bed_middle_pos.y), new_task);
		as5->set_check_solids(false);
		wedge::Set_Animation_Step *sa2 = new wedge::Set_Animation_Step(tiggy, "sleep", new_task);
		wedge::Wait_Step *ws = new wedge::Wait_Step(new_task);
		fs2->add_monitor(ws);
		wedge::A_Star_Step *as6 = new wedge::A_Star_Step(tiggy, bed_middle_pos, new_task);
		as6->set_check_solids(false);
		wedge::Set_Direction_Step *sd2 = new wedge::Set_Direction_Step(tiggy, wedge::DIR_S, true, false, new_task);
		wedge::Set_Solid_Step *sss4 = new wedge::Set_Solid_Step(tiggy, true, new_task);
		ADD_STEP(sss3)
		ADD_STEP(as4)
		ADD_STEP(as5)
		ADD_STEP(sa2)
		ADD_STEP(ws)
		ADD_STEP(as6)
		ADD_STEP(sd2)
		ADD_STEP(sss4)
		ADD_STEP(new wedge::Generic_Immediate_Callback_Step(set_following, this, new_task))
		ADD_TASK(new_task)
		FINISH_SYSTEM(AREA)
	}
}

void Inn_Step::reposition_players()
{
	wedge::Map_Entity *player = AREA->get_player(ENY);
	wedge::Map_Entity *tiggy = AREA->get_player(TIGGY);
	player->set_position(player_pos);
	tiggy->set_position(tiggy_pos);
	player->get_sprite()->set_animation("sleep");
	tiggy->get_sprite()->set_animation("sleep");
	player->set_solid(false);
	tiggy->set_solid(false);
}

void Inn_Step::set_player_positions(util::Point<int> player_pos, util::Point<int> tiggy_pos)
{
	this->player_pos = player_pos;
	this->tiggy_pos = tiggy_pos;
}

void Inn_Step::set_done(bool done)
{
	this->done = done;
}
