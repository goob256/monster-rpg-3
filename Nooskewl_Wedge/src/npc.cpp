#include "Nooskewl_Wedge/area.h"
#include "Nooskewl_Wedge/area_game.h"
#include "Nooskewl_Wedge/general.h"
#include "Nooskewl_Wedge/generic_callback.h"
#include "Nooskewl_Wedge/globals.h"
#include "Nooskewl_Wedge/npc.h"

using namespace wedge;

static void callback(void *data)
{
	if (AREA) {
		NPC *npc = static_cast<NPC *>(data);
		if (npc->changed_anim()) {
			npc->set_direction(npc->get_previous_direction(), true, npc->get_was_moving());
		}
		AREA->get_current_area()->dialogue_done(static_cast<Map_Entity *>(data));
	}
}

namespace wedge {

NPC::NPC(std::string name, std::string tag, std::string sprite_name, std::string dialogue_name) :
	Map_Entity(name),
	tag(tag),
	dialogue_name(dialogue_name)
{
	if (sprite_name != "") {
		sprite = new gfx::Sprite(sprite_name);
	}
	else {
		sprite = NULL;
	}

	load_dialogue();
}

NPC::NPC(util::JSON::Node *json) :
	Map_Entity(json)
{
	for (size_t i = 0; i < json->children.size(); i++) {
		util::JSON::Node *n = json->children[i];

		if (n->key == "\"tag\"") {
			tag = util::JSON::trim_quotes(n->value);
		}
		else if (n->key == "\"dialogue_name\"") {
			dialogue_name = util::JSON::trim_quotes(n->value);
		}
	}

	load_dialogue();
}

NPC::~NPC()
{
	// Map_Entity deletes sprite
}

void NPC::activate(Map_Entity *activator)
{
	for (size_t i = 0; i < dialogue.size(); i++) {
		bool ok = true;
		Text &t = dialogue[i];
		for (size_t j = 0; j < t.milestones.size(); j++) {
			if (INSTANCE->is_milestone_complete(t.milestones[j]) == false) {
				ok = false;
				break;
			}
		}
		if (ok) {
			std::string anim = sprite->get_animation();
			anim_changed = anim.substr(0, 5) == "stand" || anim.substr(0, 4) == "walk";

			if (anim_changed) {
				was_moving = is_moving();
				previous_direction = get_direction();

				util::Point<int> diff = activator->get_position() - get_position();
				if (diff.x < 0) {
					direction = DIR_W;
				}
				else if (diff.x > 0) {
					direction = DIR_E;
				}
				else if (diff.y < 0) {
					direction = DIR_N;
				}
				else if (diff.y > 0) {
					direction = DIR_S;
				}
				if (direction != DIR_NONE) {
					set_direction(direction, true, false);
				}
			}

			NEW_SYSTEM_AND_TASK(AREA)
			Generic_Callback_Step *step = new Generic_Callback_Step(callback, this, new_task);
			ADD_STEP(step)
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
			globals->do_dialogue(GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(tag)) + TAG_END, GLOBALS->game_t->translate(t.id), DIALOGUE_SPEECH, DIALOGUE_AUTO, step);
			return;
		}
	}
}

std::string NPC::save()
{
	std::string s;

	s += util::string_printf("\"type\": \"npc\",");
	s += util::string_printf("\"tag\": \"%s\",", tag.c_str());
	s += util::string_printf("\"dialogue_name\": \"%s\",", dialogue_name.c_str());

	s += Map_Entity::save();

	return s;
}

void NPC::load_dialogue()
{
	std::string text;
	
	try {
		text = util::load_text("text/dialogue/" + dialogue_name + "/English.utf8");
	}
	catch (util::Error e) {
		return;
	}

	util::Tokenizer t(text, '\n');
	std::string line;

	while ((line = t.next()) != "") {
		Text txt;

		util::trim(line);

		util::Tokenizer t2(line, '|');
		std::string ms = t2.next();
		txt.id = atoi(t2.next().c_str());

		std::string m;
		util::Tokenizer t3(ms, ',');

		while ((m = t3.next()) != "") {
			txt.milestones.push_back(atoi(m.c_str()));
		}

		dialogue.push_back(txt);
	}
}

Direction NPC::get_previous_direction()
{
	return previous_direction;
}

bool NPC::get_was_moving()
{
	return was_moving;
}

bool NPC::changed_anim()
{
	return anim_changed;
}

}
