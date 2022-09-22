#include <Nooskewl_Wedge/area.h>
#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/map_entity.h>

#include "globals.h"
#include "play_animation_and_delete.h"

static void callback(void *data)
{
	Play_Animation_And_Delete_Step *step = static_cast<Play_Animation_And_Delete_Step *>(data);
	step->set_done(true);
}

Play_Animation_And_Delete_Step::Play_Animation_And_Delete_Step(wedge::Map_Entity *entity, std::string anim_name, wedge::Task *task) :
	wedge::Step(task),
	entity(entity),
	anim_name(anim_name),
	done(false)
{
}

Play_Animation_And_Delete_Step::~Play_Animation_And_Delete_Step()
{
}

void Play_Animation_And_Delete_Step::start()
{
	entity->get_sprite()->set_animation(anim_name, callback, this);
}

bool Play_Animation_And_Delete_Step::run()
{
	if (done) {
		AREA->get_current_area()->remove_entity(entity, true); // NOTE: can't call this directly from set_done as that is called from Sprite callback (gfx::Sprite::update_all) where the sprite list is being processed (throw iterators out of whack)
		send_done_signal();
	}
	return !done;
}

void Play_Animation_And_Delete_Step::set_done(bool done)
{
	this->done = done;
}
