#include <Nooskewl_Wedge/general.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/onscreen_controller.h>
#include <Nooskewl_Wedge/special_number.h>
#include <Nooskewl_Wedge/systems.h>

#include "general.h"
#include "globals.h"
#include "gui.h"
#include "inventory.h"
#include "monster_rpg_3.h"
#include "shop.h"
#include "widgets.h"
					
static void buy_callback(void *data)
{
	Yes_No_GUI::Callback_Data *d = (Yes_No_GUI::Callback_Data *)data;
	Shop_GUI::Buy_Callback_Data *user = static_cast<Shop_GUI::Buy_Callback_Data *>(d->userdata);
	if (d->cancelled == false && d->choice) {
		wedge::Object o = user->object;
		o.quantity = user->quantity;
		if (INSTANCE->inventory.add(o) == o.quantity) {
			M3_GLOBALS->buysell->play(false);
			INSTANCE->add_gold(-user->quantity * user->object.quantity); // gold is stored in quantity for shopkeeps items
		}
		else {
			Notification_GUI *notification_gui = new Notification_GUI(GLOBALS->game_t->translate(1003)/* Originally: Inventory full! */);
			shim::guis.push_back(notification_gui);
		}
		user->gui->set_text(-1, -1);
		user->gui->layout();
	}

	delete user;
}

static void sell_callback(void *data)
{
	Yes_No_GUI::Callback_Data *d = (Yes_No_GUI::Callback_Data *)data;
	Shop_GUI::Sell_Callback_Data *user = static_cast<Shop_GUI::Sell_Callback_Data *>(d->userdata);
	if (d->cancelled == false && d->choice) {
		M3_GLOBALS->buysell->play(false);
		INSTANCE->add_gold(INSTANCE->inventory.get_all()[user->index].sell_price * user->quantity);
		INSTANCE->inventory.remove(user->index, user->quantity);
		user->gui->check_equipment();
		user->gui->set_text(-1, -1);
		user->gui->layout();
	}

	delete user;
}

static void buy_quantity_callback(void *data)
{

	Get_Number_GUI::Callback_Data *d = (Get_Number_GUI::Callback_Data *)data;
	Shop_GUI::Buy_Callback_Data *user = static_cast<Shop_GUI::Buy_Callback_Data *>(d->userdata);

	if (d->number > 0) {
		user->quantity = d->number;
		Yes_No_GUI *yes_no_gui = new Yes_No_GUI(util::string_printf(GLOBALS->game_t->translate(1526)/* Originally: Buy %d for %d gold? */.c_str(), user->quantity, user->quantity * user->object.quantity/*price is in quantity*/), true, buy_callback, user);
		shim::guis.push_back(yes_no_gui);
	}
	else {
		delete user;
	}
}

static void sell_quantity_callback(void *data)
{

	Get_Number_GUI::Callback_Data *d = (Get_Number_GUI::Callback_Data *)data;
	Shop_GUI::Sell_Callback_Data *user = static_cast<Shop_GUI::Sell_Callback_Data *>(d->userdata);

	if (d->number > 0) {
		user->quantity = d->number;
		Yes_No_GUI *yes_no_gui = new Yes_No_GUI(util::string_printf(GLOBALS->game_t->translate(1527)/* Originally: Sell %d for %d gold? */.c_str(), user->quantity, user->quantity * INSTANCE->inventory.get_all()[user->index].sell_price), true, sell_callback, user);
		shim::guis.push_back(yes_no_gui);
	}
	else {
		delete user;
	}
}

Shop_GUI::Shop_GUI(int character_index, int top, int selected, bool buying, wedge::Object_Type type, std::vector<wedge::Object> items) :
	Menu_GUI(character_index),
	buying(buying),
	type(type),
	items(items)
{
	if (INSTANCE->stats.size() > 1) {
		wedge::set_onscreen_controller_b3_enabled(true, M3_GLOBALS->key_switch);
	}
	else {
		wedge::set_onscreen_controller_b3_enabled(false, -1);
	}

	has_left_gui = (buying == false);
	has_right_gui = buying;

	int window_w = int(shim::screen_size.w * 0.94f);
	int window_h = int(shim::screen_size.h * 0.94f);

	window = new Widget_Window(window_w, window_h);
	window->set_centre_x(true);
	window->set_centre_y(true);

	Widget *header = new Widget(TGUI_Widget::FIT_Y, 1.0f);
	header->set_parent(window);

	gfx::Image *pic = M3_GLOBALS->mini_profile_images[character_index];

	profile_pic = new Widget_Image(pic, false);
	profile_pic->set_shadow_colour(shim::palette[24]);
	profile_pic->set_padding_top(4);
	profile_pic->set_padding_left(4);
	status_label = new Widget_Label("", -1);
	status_label->set_padding_top(2);
	status_label->set_padding_left(5);

	profile_pic->set_parent(header);
	status_label->set_parent(header);

	if (type == wedge::OBJECT_ITEM) {
		line1_label = new Widget_Label(GLOBALS->game_t->translate(1338)/* Originally: HP */, -1);
		line2_label = new Widget_Label(GLOBALS->game_t->translate(1337)/* Originally: MP */, -1);
	}
	else if (type == wedge::OBJECT_WEAPON) {
		line1_label = new Widget_Label(GLOBALS->game_t->translate(1492)/* Originally: Weapon */, -1);
		line2_label = new Widget_Label(GLOBALS->game_t->translate(1347)/* Originally: Attack */, -1);
	}
	else { // armour
		line1_label = new Widget_Label(GLOBALS->game_t->translate(1532)/* Originally: Armour */, -1);
		line2_label = new Widget_Label(GLOBALS->game_t->translate(1496)/* Originally: Defense */, -1);
	}

	int w1 = line1_label->get_width();
	int w2 = line2_label->get_width();
	int max = MAX(w1, w2);
	int add1 = (max+5)-w1;
	int add2 = (max+5)-w2;

	line1_label->set_padding_left(3);
	line2_label->set_padding_left(3);
	line1_label->set_break_line(true);
	line2_label->set_break_line(true);
	line1_label->set_parent(header);
	
	line1 = new Widget_Label("", -1);
	line1->set_padding_left(add1);
	line1->set_parent(header);

	Widget_Label *gold_label = new Widget_Label(GLOBALS->game_t->translate(1482)/* Originally: Gold */, -1);
	gold_label->set_padding_right(3);
	gold_label->set_float_right(true);
	gold_label->set_text_colour(shim::palette[11]);
	gold_label->set_shadow_colour(shim::palette[24]);
	gold_label->set_parent(header);
	
	line2_label->set_parent(header);

	line2 = new Widget_Label("", -1);
	line2->set_padding_left(add2);
	line2->set_parent(header);

	gold = new Widget_Label("", -1);
	gold->set_clear_float_x(true);
	gold->set_padding_right(3);
	gold->set_float_right(true);
	gold->set_text_colour(shim::palette[11]);
	gold->set_shadow_colour(shim::palette[24]);
	gold->set_parent(header);
	
	list = new Widget_Quantity_List(1.0f, shim::font->get_height() * 3);
	list->set_break_line(true);
	list->set_padding_left(3);
	list->set_padding_right(3);
	list->set_parent(window);
	list->set_reserved_size(SCR_H - (3.0f * shim::font->get_height() + 4));

	set_text(top, selected);

	gui = new TGUI(window, window_w, window_h);
}

Shop_GUI::~Shop_GUI()
{
	if (next_gui == NULL && next_character_gui == NULL) {
		wedge::set_onscreen_controller_b3_enabled(false, -1);
	}
}

void Shop_GUI::draw_fore()
{
	Menu_GUI::draw_fore();

	GLOBALS->bold_font->enable_shadow(shim::black, gfx::Font::FULL_SHADOW);
	std::string text;
	if (buying) {
		text = GLOBALS->game_t->translate(1535)/* Originally: BUYING */;
	}
	else {
		text = GLOBALS->game_t->translate(1536)/* Originally: SELLING */;
	}
	int w = GLOBALS->bold_font->get_text_width(text);
	GLOBALS->bold_font->draw(shim::palette[20], text, util::Point<int>(shim::screen_size.w * 0.95f - w - 5, shim::screen_size.h * 0.05f));
	GLOBALS->bold_font->disable_shadow();
}

void Shop_GUI::handle_event(TGUI_Event *event)
{
	if (
		(event->type == TGUI_KEY_DOWN && event->keyboard.code == M3_GLOBALS->key_switch) ||
		(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_switch)
	) {
		if (next_character_gui == NULL && next_gui == NULL) {
			int next_index = character_index == 0 ? 1 : 0;
			Shop_GUI *new_gui = new Shop_GUI(next_index, list == NULL ? -1 : list->get_top(), list == NULL ? -1 : list->get_selected(), buying, type, items);
			set_next_character_gui(new_gui, false);
		}
	}
	else if (event->type == TGUI_FOCUS && (event->focus.type == TGUI_FOCUS_LEFT || event->focus.type == TGUI_FOCUS_RIGHT)) {
		if (next_character_gui == NULL && next_gui == NULL) {
			if (buying == false && event->focus.type == TGUI_FOCUS_LEFT) {
				Shop_GUI *new_gui = new Shop_GUI(character_index, -1, -1, true, type, items);
				set_next_gui(new_gui, true);
			}
			else if (buying && event->focus.type == TGUI_FOCUS_RIGHT) {
				Shop_GUI *new_gui = new Shop_GUI(character_index, -1, -1, false, type, items);
				set_next_gui(new_gui, false);
			}
		}
	}
	else {
		Menu_GUI::handle_event(event);
	}
}

void Shop_GUI::update()
{
	if (list != NULL) {
		int pressed;
		if ((pressed = list->pressed()) >= 0) {
			confirm();
		}
	}

	if (type == wedge::OBJECT_WEAPON) {
		int start_attack = 0;
		start_attack += stats->base.fixed.attack;
		start_attack += stats->weapon.stats.attack;
		start_attack += stats->armour.stats.attack;

		int end_attack = -1;

		if (list != NULL) {
			if (list->get_selected() >= 0) {
				wedge::Weapon w;
				if (buying) {
					w = items[list->get_selected()].id;
				}
				else {
					w = INSTANCE->inventory.get_all()[inventory_indices[list->get_selected()]].id;
				}
				end_attack = stats->base.fixed.attack;
				end_attack += wedge::Weapon_Stats(w).stats.attack;
				end_attack += stats->armour.stats.attack;
			}
		}

		if (end_attack == -1) {
			line2->set_text(util::itos(start_attack));
		}
		else {
			std::string c;
			if (end_attack < start_attack) {
				c = "|09";
			}
			else if (end_attack > start_attack) {
				c = "|0d";
			}	
			line2->set_text(util::itos(start_attack) + " → " + c + util::itos(end_attack));
		}
	}
	else if (type == wedge::OBJECT_ARMOUR) {
		int start_defense = 0;
		start_defense += stats->base.fixed.defense;
		start_defense += stats->weapon.stats.defense;
		start_defense += stats->armour.stats.defense;

		int end_defense = -1;

		if (list != NULL) {
			if (list->get_selected() >= 0) {
				wedge::Armour a;
				if (buying) {
					a = items[list->get_selected()].id;
				}
				else {
					a = INSTANCE->inventory.get_all()[inventory_indices[list->get_selected()]].id;
				}
				end_defense = stats->base.fixed.defense;
				end_defense += wedge::Armour_Stats(a).stats.defense;
				end_defense += stats->weapon.stats.defense;
			}
		}

		if (end_defense == -1) {
			line2->set_text(util::itos(start_defense));
		}
		else {
			std::string c;
			if (end_defense < start_defense) {
				c = "|09";
			}
			else if (end_defense > start_defense) {
				c = "|0d";
			}	
			line2->set_text(util::itos(start_defense) + " → " + c + util::itos(end_defense));
		}
	}

	Menu_GUI::update();
}

void Shop_GUI::set_text(int top, int selected)
{
	status_label->set_text(get_status_name(stats->base.hp, stats->base.status));
	status_label->set_text_colour(get_status_colour(stats->base.hp, stats->base.status));

	gold->set_text(util::itos(INSTANCE->get_gold()));

	if (type == wedge::OBJECT_ITEM) {
		line1->set_text(util::itos(stats->base.hp) + "/" + util::itos(stats->base.fixed.max_hp));
		line1->set_text_colour(get_status_colour(stats->base.hp - stats->base.fixed.max_hp * 0.15f, wedge::STATUS_OK));
		line2->set_text(util::itos(stats->base.mp) + "/" + util::itos(stats->base.fixed.max_mp));
	}
	else if (type == wedge::OBJECT_WEAPON) {
		wedge::Object weapon = OBJECT->make_object(wedge::OBJECT_WEAPON, stats->weapon.id, 1);
		line1->set_text(weapon.name == "" ? "" : GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(weapon.name)));
	}
	else {
		wedge::Object armour = OBJECT->make_object(wedge::OBJECT_ARMOUR, stats->armour.id, 1);
		line1->set_text(armour.name == "" ? "" : GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(armour.name)));
	}

	inventory_indices.clear();

	std::vector< std::pair<int, std::string> > v;
	std::vector<std::string> descriptions;

	std::vector<wedge::Object> objects;

	if (buying) {
		objects = items;
	}
	else {
		wedge::Object *inv = INSTANCE->inventory.get_all();
		for (int i = 0; i < wedge::Inventory::MAX_OBJECTS; i++) {
			if (inv[i].type == type && (type != wedge::OBJECT_ITEM || is_scroll(inv[i].id) == false)) {
				objects.push_back(inv[i]);
				inventory_indices.push_back(i);
			}
		}
	}

	std::vector<int> highlight;

	for (size_t i = 0; i < objects.size(); i++) {
		// highlight equipped weapon and armour (you can still sell it)
		if (buying == false) {
			if (type == wedge::OBJECT_WEAPON) {
				for (size_t j = 0; j < MAX_PARTY; j++) {
					if (objects[i].id == INSTANCE->stats[j].weapon.id) {
						if (std::find(highlight.begin(), highlight.end(), i) == highlight.end()) {
							highlight.push_back((int)i);
						}
						break;
					}
				}
			}
			else if (type == wedge::OBJECT_ARMOUR) {
				for (size_t j = 0; j < MAX_PARTY; j++) {
					if (objects[i].id == INSTANCE->stats[j].armour.id) {
						if (std::find(highlight.begin(), highlight.end(), i) == highlight.end()) {
							highlight.push_back((int)i);
						}
						break;
					}
				}
			}
		}
		std::pair<int, std::string> p;
		p.first = objects[i].quantity;
		p.second = GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(objects[i].name));
		v.push_back(p);
		descriptions.push_back(objects[i].description);
	}

	if (v.size() == 0) {
		list->set_parent(NULL);
		delete list;
		list = NULL;
		if (gui != NULL) {
			gui->focus_something();
		}
		Widget_Label *empty_label;
		if (type == wedge::OBJECT_ITEM) {
			empty_label = new Widget_Label(GLOBALS->game_t->translate(1485)/* Originally: No items */, -1);
		}
		else if (type == wedge::OBJECT_WEAPON) {
			empty_label = new Widget_Label(GLOBALS->game_t->translate(1494)/* Originally: No weapons */, -1);
		}
		else {
			empty_label = new Widget_Label(GLOBALS->game_t->translate(1497)/* Originally: No armour */, -1);
		}
		empty_label->set_padding_top(10);
		empty_label->set_centre_x(true);
		empty_label->set_break_line(true);
		empty_label->set_parent(window);
		if (gui) {
			gui->layout();
		}
	}
	else {
		list->set_items(v);
		list->set_descriptions(descriptions);
		int selected = list->get_selected();
		if (selected >= (int)v.size()) {
			list->set_selected(selected-1);
		}
		for (size_t i = 0; i < v.size(); i++) {
			list->set_highlight((int)i, false);
			list->set_disabled((int)i, false);
		}
		for (size_t i = 0; i < highlight.size(); i++) {
			list->set_highlight(highlight[i], true);
		}
		if (buying) {
			for (size_t i = 0; i < items.size(); i++) {
				if (items[i].quantity > INSTANCE->get_gold()) {
					list->set_disabled((int)i, true);
				}
				wedge::Object o = items[i];
				o.quantity = wedge::Inventory::MAX_STACK+1;
				int max = INSTANCE->inventory.add(o);
				if (max <= 0) {
					list->set_disabled((int)i, true); // already have 99
				}
			}
		}
	}

	if (list != NULL && top >= 0 && selected >= 0) {
		list->set_top(top);
		list->set_selected(selected);
	}
}

void Shop_GUI::confirm()
{
	int selected = list->get_selected();
	if (buying) {
		Buy_Callback_Data *dat = new Buy_Callback_Data;
		dat->object = items[selected];
		dat->gui = this;

		int stops = INSTANCE->get_gold() / dat->object.quantity + 1;

		wedge::Object o = dat->object;
		o.quantity = wedge::Inventory::MAX_STACK+1;
		int max = INSTANCE->inventory.add(o);

		if (max <= 0) {
			Notification_GUI *notification_gui = new Notification_GUI(GLOBALS->game_t->translate(1003)/* Originally: Inventory full! */);
			shim::guis.push_back(notification_gui);
		}
		else {
			max = MIN(stops, max+1);
			Get_Number_GUI *quantity_gui = new Get_Number_GUI(GLOBALS->game_t->translate(1541)/* Originally: Buy how many? */, max, 0, buy_quantity_callback, dat);
			shim::guis.push_back(quantity_gui);
		}
	}
	else {
		Sell_Callback_Data *dat = new Sell_Callback_Data;
		dat->index = inventory_indices[selected];
		dat->gui = this;

		Get_Number_GUI *quantity_gui = new Get_Number_GUI(GLOBALS->game_t->translate(1542)/* Originally: Sell how many? */, INSTANCE->inventory.get_all()[dat->index].quantity+1, 0, sell_quantity_callback, dat);
		shim::guis.push_back(quantity_gui);
	}
}

void Shop_GUI::layout()
{
	gui->layout();
}

void Shop_GUI::check_equipment()
{
	for (size_t i = 0; i < MAX_PARTY; i++) {
		wedge::Player_Stats *stats = &INSTANCE->stats[i];
		wedge::Object *objects = INSTANCE->inventory.get_all();
		bool found_weapon = false;
		bool found_armour = false;
		for (int j = 0; j < wedge::Inventory::MAX_OBJECTS; j++) {
			if (objects[j].type == wedge::OBJECT_WEAPON && objects[j].id == stats->weapon.id) {
				found_weapon = true;
			}
			else if (objects[j].type == wedge::OBJECT_ARMOUR && objects[j].id == stats->armour.id) {
				found_armour = true;
			}
		}
		if (found_weapon == false) {
			stats->weapon = wedge::Weapon_Stats();
		}
		if (found_armour == false) {
			stats->armour = wedge::Armour_Stats();
		}
	}
}
