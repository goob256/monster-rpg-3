#include <Nooskewl_Wedge/area.h>
#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/map_entity.h>

#include "battle_game.h"
#include "battle_transition_in.h"
#include "battle_transition_in2.h"
#include "globals.h"
#include "milestones.h"
#include "transition.h"
#include "sailor.h"
#include "stats.h"

Battle_Transition_In_Step::Battle_Transition_In_Step(wedge::Battle_Game *battle_game, wedge::Task *task) :
	Transition_Step(true, task),
	battle_game(battle_game)
{
}

Battle_Transition_In_Step::~Battle_Transition_In_Step()
{
}

bool Battle_Transition_In_Step::run()
{
	bool ret = Transition_Step::run();
	if (ret == false) {
		if (INSTANCE->is_milestone_complete(MS_SWAPPED_TIGGY) == false) {
			INSTANCE->set_milestone_complete(MS_SWAPPED_TIGGY, true);

			wedge::Area *area = AREA->get_current_area();

			wedge::Map_Entity *old_tiggy = area->find_entity("tig");

			INSTANCE->stats.push_back(wedge::Player_Stats());
			INSTANCE->stats[1].name = "Tiggy";
			INSTANCE->stats[1].sprite = new gfx::Sprite("tiggy");
			INSTANCE->stats[1].level = 1;
			INSTANCE->stats[1].experience = 0;
			INSTANCE->stats[1].base.fixed.max_hp = 100;
			INSTANCE->stats[1].base.fixed.max_mp = 25;
			INSTANCE->stats[1].base.fixed.attack = 25;
			INSTANCE->stats[1].base.fixed.defense = 25;
			INSTANCE->stats[1].base.fixed.set_extra(LUCK, 10);
			INSTANCE->stats[1].base.hp = INSTANCE->stats[1].base.fixed.max_hp;
			INSTANCE->stats[1].base.mp = INSTANCE->stats[1].base.fixed.max_mp;

			Sailor *tiggy = new Sailor("Tiggy");
			tiggy->start(area);
			tiggy->set_position(old_tiggy->get_position());
			tiggy->set_sprite(new gfx::Sprite("tiggy"));
			tiggy->set_direction(old_tiggy->get_direction(), true, false);
			area->add_entity(tiggy);

			AREA->set_player(TIGGY, tiggy);

			INSTANCE->party_following_player = true;

			area->remove_entity(old_tiggy, true);
		}

		BATTLE = battle_game;
		BATTLE->start();
		BATTLE->show_enemy_stats(false);
		BATTLE->show_player_stats(false);
		NEW_SYSTEM_AND_TASK(BATTLE)
		Battle_Transition_In2_Step *step = new Battle_Transition_In2_Step(new_task);
		ADD_STEP(step)
		ADD_TASK(new_task)
		FINISH_SYSTEM(BATTLE)
	}
	if (ret == false) {
		send_done_signal();
	}
	return ret;
}
