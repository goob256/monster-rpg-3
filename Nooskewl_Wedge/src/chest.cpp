#include "Nooskewl_Wedge/chest.h"
#include "Nooskewl_Wedge/general.h"
#include "Nooskewl_Wedge/globals.h"

using namespace wedge;

namespace wedge {

Chest::Chest(std::string name, std::string sprite_name, Object contents, int milestone, Dialogue_Position dialogue_position) :
	Map_Entity(name),
	open(false),
	contents(contents),
	gold(-1),
	milestone(milestone),
	dialogue_position(dialogue_position)
{
	if (sprite_name != "") {
		sprite = new gfx::Sprite(sprite_name);
	}
	else {
		sprite = NULL;
	}
}

Chest::Chest(std::string name, std::string sprite_name, int gold, int milestone, Dialogue_Position dialogue_position) :
	Map_Entity(name),
	open(false),
	gold(gold),
	milestone(milestone),
	dialogue_position(dialogue_position)
{
	if (sprite_name != "") {
		sprite = new gfx::Sprite(sprite_name);
	}
	else {
		sprite = NULL;
	}
}

Chest::Chest(util::JSON::Node *json) :
	Map_Entity(json),
	open(false),
	gold(-1),
	dialogue_position(DIALOGUE_AUTO)
{
	Object_Type type = OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	milestone = -1; // this didn't exist before, must set a default

	for (size_t i = 0; i < json->children.size(); i++) {
		util::JSON::Node *n = json->children[i];

		if (n->key == "\"chest_open\"") {
			open = json_to_bool(n);
		}
		else if (n->key == "\"chest_type\"") {
			type = (Object_Type)json_to_integer(n);
		}
		else if (n->key == "\"chest_object_id\"") {
			id = (Item)json_to_integer(n);
		}
		else if (n->key == "\"chest_quantity\"") {
			quantity = json_to_integer(n);
		}
		else if (n->key == "\"chest_gold\"") {
			gold = json_to_integer(n);
		}
		else if (n->key == "\"chest_milestone\"") {
			milestone = json_to_integer(n);
		}
		else if (n->key == "\"chest_dialogue_position\"") {
			dialogue_position = (Dialogue_Position)json_to_integer(n);
		}
	}

	if (gold <= 0) {
		contents = OBJECT->make_object(type, id, quantity);
	}

	if (open) {
		if (sprite) {
			sprite->set_animation("open");
		}
	}
}

Chest::~Chest()
{
	// Map_Entity deletes sprite
}

void Chest::activate(Map_Entity *activator)
{
	if (open == false) {
		bool success;
		if (gold > 0) {
			INSTANCE->add_gold(gold);
			success = true;
		}
		else {
			success = INSTANCE->inventory.add(contents) == contents.quantity;
		}
		if (success) {
			if (milestone >= 0) {
				INSTANCE->set_milestone_complete(milestone, true);
			}

			globals->chest->play(false);
			open = true;
			if (sprite) {
				sprite->set_animation("open");
			}
			std::string message;
			if (gold > 0) {
				message = util::string_printf(GLOBALS->game_t->translate(1001)/* Originally: Received %d gold! */.c_str(), gold);
			}
			else {
				if (contents.quantity > 1) {
					if (GLOBALS->language == "Spanish") {
						message = util::string_printf(GLOBALS->game_t->translate(1004)/* Originally: Received %d %s! */.c_str(), GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(contents.name)).c_str(), contents.quantity);
					}
					else {
						message = util::string_printf(GLOBALS->game_t->translate(1004)/* Originally: Received %d %s! */.c_str(), contents.quantity, GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(contents.name)).c_str());
					}
				}
				else {
					message = util::string_printf(GLOBALS->game_t->translate(1005)/* Originally: Received %s! */.c_str(), GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(contents.name)).c_str());
				}
			}
			globals->do_dialogue("", message, DIALOGUE_MESSAGE, dialogue_position, NULL);
		}
		else {
			globals->do_dialogue("", GLOBALS->game_t->translate(1003)/* Originally: Inventory full! */, DIALOGUE_MESSAGE, dialogue_position, NULL);
		}

		INSTANCE->chests_opened++;
	}
}

std::string Chest::save()
{
	std::string s;

	s += util::string_printf("\"type\": \"chest\",");
	s += util::string_printf("\"chest_open\": %s,", bool_to_string(open).c_str());
	s += util::string_printf("\"chest_type\": %d,", (int)contents.type);
	s += util::string_printf("\"chest_object_id\": %d,", contents.id);
	s += util::string_printf("\"chest_quantity\": %d,", contents.quantity);
	s += util::string_printf("\"chest_gold\": %d,", gold);
	s += util::string_printf("\"chest_milestone\": %d,", milestone);
	s += util::string_printf("\"chest_dialogue_position\": %d,", (int)dialogue_position);

	s += Map_Entity::save();

	return s;
}

}
