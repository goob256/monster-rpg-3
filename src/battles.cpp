#include <Nooskewl_Wedge/area.h>
#include <Nooskewl_Wedge/area_game.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/map_entity.h>

#include "battle_game.h"
#include "battles.h"
#include "enemies.h"
#include "globals.h"
#include "inventory.h"
#include "milestones.h"

Battle_Fiddler::Battle_Fiddler() :
	Monster_RPG_3_Battle_Game("forest", 0)
{
	boss_battle = true;
}

Battle_Fiddler::~Battle_Fiddler()
{
	// remove the fiddlehead
	if (AREA != NULL) { // can be NULL if you died
		wedge::Area *area = AREA->get_current_area();
		wedge::Map_Entity_List &entities = area->get_entities();
		for (wedge::Map_Entity_List::iterator it = entities.begin(); it != entities.end(); it++) {
			wedge::Map_Entity *entity = *it;
			if (entity->get_name() == "fiddler") {
				area->remove_entity(entity, true); // can't just delete it, Area_Hooks needs to run
				break;
			}
		}
	}
}

bool Battle_Fiddler::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *fiddler = new Enemy_Fiddler();
	fiddler->start();
	fiddler->set_position(util::Point<int>(shim::tile_size, shim::tile_size*1.5));
	entities.push_back(fiddler);

	gold += fiddler->get_gold();
	experience += fiddler->get_experience();

	return true;
}

wedge::Object Battle_Fiddler::get_found_object()
{
	return OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_CURE, 2);
}

//--

Battle_2Goos::Battle_2Goos() :
	Monster_RPG_3_Battle_Game("forest", 0)
{
}

Battle_2Goos::~Battle_2Goos()
{
}

bool Battle_2Goos::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *goo1 = new Enemy_Goo();
	goo1->start();
	goo1->set_position(util::Point<int>(shim::tile_size*3, shim::tile_size*1.5));
	entities.push_back(goo1);

	gold += goo1->get_gold();
	experience += goo1->get_experience();

	wedge::Battle_Enemy *goo2 = new Enemy_Goo();
	goo2->start();
	goo2->set_position(util::Point<int>(shim::tile_size, shim::tile_size));
	entities.push_back(goo2);

	gold += goo2->get_gold();
	experience += goo2->get_experience();

	return true;
}

wedge::Object Battle_2Goos::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	if (util::rand(0, 4) == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_Mushroom::Battle_Mushroom() :
	Monster_RPG_3_Battle_Game("forest", 0)
{
}

Battle_Mushroom::~Battle_Mushroom()
{
}

bool Battle_Mushroom::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *mushroom = new Enemy_Mushroom();
	mushroom->start();
	mushroom->set_position(util::Point<int>(shim::tile_size, shim::tile_size*1.5));
	entities.push_back(mushroom);

	gold += mushroom->get_gold();
	experience += mushroom->get_experience();

	return true;
}

wedge::Object Battle_Mushroom::get_found_object()
{
	int num_poisoned = 0;
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	std::vector<wedge::Battle_Entity *> players = BATTLE->get_players();

	for (size_t i = 0; i < players.size(); i++) {
		if (players[i]->get_stats()->status == wedge::STATUS_POISONED) {
			num_poisoned++;
		}
	}

	if (num_poisoned == 0) {
		if (util::rand(0, 4) == 0) {
			type = wedge::OBJECT_ITEM;
			id = ITEM_POTION;
			quantity = 1;
		}
	}
	else {
		type = wedge::OBJECT_ITEM;
		id = ITEM_CURE;
		quantity = num_poisoned;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_3Bloated::Battle_3Bloated() :
	Monster_RPG_3_Battle_Game("forest", 0)
{
}

Battle_3Bloated::~Battle_3Bloated()
{
}

bool Battle_3Bloated::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *bloated1 = new Enemy_Bloated();
	bloated1->start();
	bloated1->set_position(util::Point<int>(shim::tile_size*1.5f, shim::tile_size*4/3));

	gold += bloated1->get_gold();
	experience += bloated1->get_experience();

	wedge::Battle_Enemy *bloated2 = new Enemy_Bloated();
	bloated2->start();
	bloated2->set_position(util::Point<int>(shim::tile_size*1.5f+bloated1->get_sprite()->get_current_image()->size.w*1.5f, shim::tile_size*4/3));

	gold += bloated2->get_gold();
	experience += bloated2->get_experience();

	wedge::Battle_Enemy *bloated3 = new Enemy_Bloated();
	bloated3->start();
	bloated3->set_position(util::Point<int>(shim::tile_size*1.5f+bloated1->get_sprite()->get_current_image()->size.w*0.75f, shim::tile_size*4/3+bloated1->get_sprite()->get_current_image()->size.h*1.1f));

	gold += bloated3->get_gold();
	experience += bloated3->get_experience();

	entities.push_back(bloated2);
	entities.push_back(bloated3);
	entities.push_back(bloated1);

	return true;
}

wedge::Object Battle_3Bloated::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	if (util::rand(0, 4) == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_2Treant::Battle_2Treant() :
	Monster_RPG_3_Battle_Game("forest", 0)
{
}

Battle_2Treant::~Battle_2Treant()
{
}

bool Battle_2Treant::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *treant1 = new Enemy_Treant();
	treant1->start();
	treant1->set_position(util::Point<int>(shim::tile_size, shim::tile_size/3));

	gold += treant1->get_gold();
	experience += treant1->get_experience();

	wedge::Battle_Enemy *treant2 = new Enemy_Treant();
	treant2->start();
	treant2->set_position(util::Point<int>(shim::tile_size+treant1->get_sprite()->get_current_image()->size.w+shim::tile_size/2, shim::tile_size/3+shim::tile_size/2));

	gold += treant2->get_gold();
	experience += treant2->get_experience();
	
	entities.push_back(treant2);
	entities.push_back(treant1);

	return true;
}

wedge::Object Battle_2Treant::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	if (util::rand(0, 10) == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_HOLY_WATER;
		quantity = 1;
	}
	else if (util::rand(0, 1) == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_3Sludge::Battle_3Sludge() :
	Monster_RPG_3_Battle_Game("hh", 0)
{
}

Battle_3Sludge::~Battle_3Sludge()
{
}

bool Battle_3Sludge::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *sludge1 = new Enemy_Sludge();
	sludge1->start();
	sludge1->set_position(util::Point<int>(shim::tile_size, shim::tile_size));

	gold += sludge1->get_gold();
	experience += sludge1->get_experience();

	wedge::Battle_Enemy *sludge2 = new Enemy_Sludge();
	sludge2->start();
	sludge2->set_position(util::Point<int>(shim::tile_size*4.0f/3.0f+sludge1->get_sprite()->get_current_image()->size.w*2.0f, shim::tile_size/2));

	gold += sludge2->get_gold();
	experience += sludge2->get_experience();

	wedge::Battle_Enemy *sludge3 = new Enemy_Sludge();
	sludge3->start();
	sludge3->set_position(util::Point<int>(shim::tile_size*7.0f/6.0f+sludge1->get_sprite()->get_current_image()->size.w, sludge1->get_sprite()->get_current_image()->size.h*1.1f));

	gold += sludge3->get_gold();
	experience += sludge3->get_experience();
	
	entities.push_back(sludge2);
	entities.push_back(sludge3);
	entities.push_back(sludge1);

	return true;
}

wedge::Object Battle_3Sludge::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 8);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION;
		quantity = 1;
	}
	else if (r == 1) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_CURE;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_2Ghastly::Battle_2Ghastly() :
	Monster_RPG_3_Battle_Game("hh", 0)
{
}

Battle_2Ghastly::~Battle_2Ghastly()
{
}

bool Battle_2Ghastly::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *ghastly1 = new Enemy_Ghastly();
	ghastly1->start();
	ghastly1->set_position(util::Point<int>(shim::tile_size*2+ghastly1->get_sprite()->get_current_image()->size.w, shim::tile_size*1.5f));
	entities.push_back(ghastly1);

	gold += ghastly1->get_gold();
	experience += ghastly1->get_experience();

	wedge::Battle_Enemy *ghastly2 = new Enemy_Ghastly();
	ghastly2->start();
	ghastly2->set_position(util::Point<int>(shim::tile_size, shim::tile_size));
	entities.push_back(ghastly2);

	gold += ghastly2->get_gold();
	experience += ghastly2->get_experience();

	return true;
}

wedge::Object Battle_2Ghastly::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	if (util::rand(0, 8) == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_1Ghastly2Sludge::Battle_1Ghastly2Sludge() :
	Monster_RPG_3_Battle_Game("hh", 0)
{
}

Battle_1Ghastly2Sludge::~Battle_1Ghastly2Sludge()
{
}

bool Battle_1Ghastly2Sludge::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *ghastly = new Enemy_Ghastly();
	ghastly->start();
	ghastly->set_position(util::Point<int>(shim::tile_size, shim::tile_size));

	gold += ghastly->get_gold();
	experience += ghastly->get_experience();

	wedge::Battle_Enemy *sludge1 = new Enemy_Sludge();
	sludge1->start();
	sludge1->set_position(util::Point<int>(shim::tile_size*4.0f/3.0f+sludge1->get_sprite()->get_current_image()->size.w+ghastly->get_sprite()->get_current_image()->size.w, shim::tile_size/2));

	gold += sludge1->get_gold();
	experience += sludge1->get_experience();

	wedge::Battle_Enemy *sludge2 = new Enemy_Sludge();
	sludge2->start();
	sludge2->set_position(util::Point<int>(shim::tile_size*7.0f/6.0f+ghastly->get_sprite()->get_current_image()->size.w, sludge1->get_sprite()->get_current_image()->size.h*1.1f));

	gold += sludge2->get_gold();
	experience += sludge2->get_experience();
	
	entities.push_back(sludge1);
	entities.push_back(sludge2);
	entities.push_back(ghastly);

	return true;
}

wedge::Object Battle_1Ghastly2Sludge::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 8);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION;
		quantity = 1;
	}
	else if (r == 1) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_CURE;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_2Werewolf::Battle_2Werewolf() :
	Monster_RPG_3_Battle_Game("hh", 0)
{
}

Battle_2Werewolf::~Battle_2Werewolf()
{
}

bool Battle_2Werewolf::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *werewolf1 = new Enemy_Werewolf();
	werewolf1->start();
	werewolf1->set_position(util::Point<int>(shim::tile_size*2.0f+werewolf1->get_sprite()->get_current_image()->size.w, shim::tile_size));
	entities.push_back(werewolf1);

	gold += werewolf1->get_gold();
	experience += werewolf1->get_experience();

	wedge::Battle_Enemy *werewolf2 = new Enemy_Werewolf();
	werewolf2->start();
	werewolf2->set_position(util::Point<int>(shim::tile_size, shim::tile_size));
	entities.push_back(werewolf2);

	gold += werewolf2->get_gold();
	experience += werewolf2->get_experience();

	return true;
}

wedge::Object Battle_2Werewolf::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 12);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_HOLY_WATER;
		quantity = 1;
	}
	else if (r == 1) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_2Knightly::Battle_2Knightly() :
	Monster_RPG_3_Battle_Game("hh", 0)
{
}

Battle_2Knightly::~Battle_2Knightly()
{
}

bool Battle_2Knightly::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *knightly1 = new Enemy_Knightly();
	knightly1->start();
	knightly1->set_position(util::Point<int>(shim::tile_size*2.0f+knightly1->get_sprite()->get_current_image()->size.w, shim::tile_size));
	entities.push_back(knightly1);

	gold += knightly1->get_gold();
	experience += knightly1->get_experience();

	wedge::Battle_Enemy *knightly2 = new Enemy_Knightly();
	knightly2->start();
	knightly2->set_position(util::Point<int>(shim::tile_size, shim::tile_size));
	entities.push_back(knightly2);

	gold += knightly2->get_gold();
	experience += knightly2->get_experience();

	return true;
}

wedge::Object Battle_2Knightly::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 10);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_HOLY_WATER;
		quantity = 1;
	}
	else if (r == 1) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_Palla::Battle_Palla() :
	Monster_RPG_3_Battle_Game("hh", 0)
{
	boss_battle = true;
}

Battle_Palla::~Battle_Palla()
{
}

bool Battle_Palla::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *vampire = new Enemy_Palla();
	vampire->start();
	util::Point<int> topleft, bottomright;
	vampire->get_sprite()->get_bounds(topleft, bottomright);
	vampire->set_position(util::Point<int>(shim::tile_size*1.5f-topleft.x+5, 8-topleft.y));
	entities.push_back(vampire);

	gold += vampire->get_gold();
	experience += vampire->get_experience();

	return true;
}

wedge::Object Battle_Palla::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;
	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_3Tentacle::Battle_3Tentacle() :
	Monster_RPG_3_Battle_Game("at_sea", 500)
{
}

Battle_3Tentacle::~Battle_3Tentacle()
{
}

bool Battle_3Tentacle::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *tentacle1 = new Enemy_Tentacle();
	tentacle1->start();
	tentacle1->set_position(util::Point<int>(shim::tile_size/4, shim::tile_size/2));

	gold += tentacle1->get_gold();
	experience += tentacle1->get_experience();

	wedge::Battle_Enemy *tentacle2 = new Enemy_Tentacle();
	tentacle2->start();
	tentacle2->set_position(util::Point<int>(shim::tile_size*3/4+tentacle1->get_sprite()->get_current_image()->size.w/2, shim::tile_size/4));

	gold += tentacle2->get_gold();
	experience += tentacle2->get_experience();

	wedge::Battle_Enemy *tentacle3 = new Enemy_Tentacle();
	tentacle3->start();
	tentacle3->set_position(util::Point<int>(shim::tile_size+shim::tile_size/4+tentacle1->get_sprite()->get_current_image()->size.w, shim::tile_size/4));

	gold += tentacle3->get_gold();
	experience += tentacle3->get_experience();
	
	entities.push_back(tentacle3);
	entities.push_back(tentacle2);
	entities.push_back(tentacle1);

	return true;
}

wedge::Object Battle_3Tentacle::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 9);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION_PLUS;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_Wave::Battle_Wave() :
	Monster_RPG_3_Battle_Game("at_sea", 500)
{
}

Battle_Wave::~Battle_Wave()
{
}

bool Battle_Wave::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *wave = new Enemy_Wave();
	wave->start();
	wave->set_position(util::Point<int>(0, -(shim::tile_size+4)));
	entities.push_back(wave);

	gold += wave->get_gold();
	experience += wave->get_experience();

	return true;
}

wedge::Object Battle_Wave::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 11);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION_PLUS;
		quantity = util::rand(1, 2);
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_3Shocker::Battle_3Shocker() :
	Monster_RPG_3_Battle_Game("at_sea", 500)
{
}

Battle_3Shocker::~Battle_3Shocker()
{
}

bool Battle_3Shocker::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *shocker1 = new Enemy_Shocker();
	shocker1->start();

	const int pad = shocker1->get_sprite()->get_current_image()->size.w * 0.1;
	const int sprite_w = shocker1->get_sprite()->get_current_image()->size.w;

	shocker1->set_position(util::Point<int>(-pad/2, shim::tile_size*3/2));

	gold += shocker1->get_gold();
	experience += shocker1->get_experience();

	wedge::Battle_Enemy *shocker2 = new Enemy_Shocker();
	shocker2->start();
	shocker2->set_position(util::Point<int>(sprite_w-pad, shim::tile_size/2));

	gold += shocker2->get_gold();
	experience += shocker2->get_experience();

	wedge::Battle_Enemy *shocker3 = new Enemy_Shocker();
	shocker3->start();
	shocker3->set_position(util::Point<int>(sprite_w*2-pad*2, shim::tile_size*3/2));

	gold += shocker3->get_gold();
	experience += shocker3->get_experience();
	
	entities.push_back(shocker3);
	entities.push_back(shocker2);
	entities.push_back(shocker1);

	return true;
}

wedge::Object Battle_3Shocker::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 19);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_ELIXIR;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_1Shocker2Tentacle::Battle_1Shocker2Tentacle() :
	Monster_RPG_3_Battle_Game("at_sea", 500)
{
}

Battle_1Shocker2Tentacle::~Battle_1Shocker2Tentacle()
{
}

bool Battle_1Shocker2Tentacle::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *shocker = new Enemy_Shocker();
	shocker->start();
	shocker->set_position(util::Point<int>(0, shim::tile_size/2));

	const int sprite_w = shocker->get_sprite()->get_current_image()->size.w * 0.9f;

	gold += shocker->get_gold();
	experience += shocker->get_experience();

	wedge::Battle_Enemy *tentacle1 = new Enemy_Tentacle();
	tentacle1->start();
	tentacle1->set_position(util::Point<int>(sprite_w, shim::tile_size/4));

	gold += tentacle1->get_gold();
	experience += tentacle1->get_experience();
	
	const int sprite2_w = tentacle1->get_sprite()->get_current_image()->size.w * 0.8f;

	wedge::Battle_Enemy *tentacle2 = new Enemy_Tentacle();
	tentacle2->start();
	tentacle2->set_position(util::Point<int>(sprite_w+sprite2_w, shim::tile_size/4));

	gold += tentacle2->get_gold();
	experience += tentacle2->get_experience();

	entities.push_back(tentacle2);
	entities.push_back(tentacle1);
	entities.push_back(shocker);

	return true;
}

wedge::Object Battle_1Shocker2Tentacle::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 19);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_ELIXIR;
		quantity = 1;
	}
	else if (r == 1 || r == 2) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION_PLUS;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_Monster::Battle_Monster() :
	Monster_RPG_3_Battle_Game("at_sea", 500),
	made_noise(false)
{
	boss_battle = true;

	add_dialogue(GLOBALS->game_t->translate(1356)/* Originally: Monster */ + TAG_END, GLOBALS->game_t->translate(1357)/* Originally: Are you here to take my Rune? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM);
	add_dialogue(GLOBALS->game_t->translate(0)/* Originally: Eny */ + TAG_END, GLOBALS->game_t->translate(1359)/* Originally: What Rune? What are you talking about? */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM);
	add_dialogue(GLOBALS->game_t->translate(1356)/* Originally: Monster */ + TAG_END, GLOBALS->game_t->translate(1361)/* Originally: You'll never take my Rune! */, wedge::DIALOGUE_SPEECH, wedge::DIALOGUE_BOTTOM);
}

Battle_Monster::~Battle_Monster()
{
	delete monster_sound;
}

bool Battle_Monster::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *monster = new Enemy_Monster();
	monster->start();
	monster->set_position(util::Point<int>(-30, shim::tile_size/2));
	entities.push_back(monster);

	gold += monster->get_gold();
	experience += monster->get_experience();

	monster_sound = new audio::MML("sfx/monster.mml");

	return true;
}

bool Battle_Monster::run()
{
	if (made_noise == false && dialogue_tags.size() == 0) {
		made_noise = true;
		monster_sound->play(false);
	}
	return Monster_RPG_3_Battle_Game::run();
}

wedge::Object Battle_Monster::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_SPECIAL;
	int id = SPECIAL_RUNE;
	int quantity = 1;
	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_1Sandworm2Flare::Battle_1Sandworm2Flare() :
	Monster_RPG_3_Battle_Game("desert", 0)
{
}

Battle_1Sandworm2Flare::~Battle_1Sandworm2Flare()
{
}

bool Battle_1Sandworm2Flare::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *sandworm = new Enemy_Sandworm();
	sandworm->start();
	sandworm->set_position(util::Point<int>(shim::tile_size/2, shim::tile_size));

	gold += sandworm->get_gold();
	experience += sandworm->get_experience();

	wedge::Battle_Enemy *flare1 = new Enemy_Flare();
	flare1->start();
	flare1->set_position(util::Point<int>(shim::tile_size*2.0f+flare1->get_sprite()->get_current_image()->size.w+sandworm->get_sprite()->get_current_image()->size.w, shim::tile_size*2/3));

	gold += flare1->get_gold();
	experience += flare1->get_experience();

	wedge::Battle_Enemy *flare2 = new Enemy_Flare();
	flare2->start();
	flare2->set_position(util::Point<int>(1/*just shimmy it over a lil*/+shim::tile_size+sandworm->get_sprite()->get_current_image()->size.w, flare1->get_sprite()->get_current_image()->size.h*1.1f));

	gold += flare2->get_gold();
	experience += flare2->get_experience();
	
	entities.push_back(flare1);
	entities.push_back(flare2);
	entities.push_back(sandworm);

	return true;
}

wedge::Object Battle_1Sandworm2Flare::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 16);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION_PLUS;
		quantity = 1;
	}
	else if (r == 1) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_ELIXIR;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_3Cyclone::Battle_3Cyclone() :
	Monster_RPG_3_Battle_Game("desert", 0)
{
}

Battle_3Cyclone::~Battle_3Cyclone()
{
}

bool Battle_3Cyclone::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *cyclone1 = new Enemy_Cyclone();
	cyclone1->start();
	cyclone1->set_position(util::Point<int>(shim::tile_size/2, shim::tile_size));

	gold += cyclone1->get_gold();
	experience += cyclone1->get_experience();

	wedge::Battle_Enemy *cyclone2 = new Enemy_Cyclone();
	cyclone2->start();
	cyclone2->set_position(util::Point<int>(shim::tile_size*2.5f/3.0f+cyclone1->get_sprite()->get_current_image()->size.w*2.0f, shim::tile_size/2));

	gold += cyclone2->get_gold();
	experience += cyclone2->get_experience();

	wedge::Battle_Enemy *cyclone3 = new Enemy_Cyclone();
	cyclone3->start();
	cyclone3->set_position(util::Point<int>(shim::tile_size*2.0f/3.0f+cyclone1->get_sprite()->get_current_image()->size.w, shim::tile_size*3/4));

	gold += cyclone3->get_gold();
	experience += cyclone3->get_experience();
	
	entities.push_back(cyclone2);
	entities.push_back(cyclone3);
	entities.push_back(cyclone1);

	return true;
}

wedge::Object Battle_3Cyclone::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 5);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_CURE;
		quantity = 1;
	}
	else if (r == 1) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_CURE;
		quantity = 2;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_2Bones::Battle_2Bones() :
	Monster_RPG_3_Battle_Game("desert", 0)
{
}

Battle_2Bones::~Battle_2Bones()
{
}

bool Battle_2Bones::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *bones1 = new Enemy_Bones();
	bones1->start();
	bones1->set_position(util::Point<int>(shim::tile_size*2.0f+bones1->get_sprite()->get_current_image()->size.w, shim::tile_size*1.5f));

	gold += bones1->get_gold();
	experience += bones1->get_experience();

	wedge::Battle_Enemy *bones2 = new Enemy_Bones();
	bones2->start();
	bones2->set_position(util::Point<int>(shim::tile_size, shim::tile_size*2.0f));

	gold += bones2->get_gold();
	experience += bones2->get_experience();
	
	entities.push_back(bones1);
	entities.push_back(bones2);

	return true;
}

wedge::Object Battle_2Bones::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 20);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION_PLUS;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_1Bones2Flare::Battle_1Bones2Flare() :
	Monster_RPG_3_Battle_Game("desert", 0)
{
}

Battle_1Bones2Flare::~Battle_1Bones2Flare()
{
}

bool Battle_1Bones2Flare::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *flare1 = new Enemy_Flare();
	flare1->start();
	flare1->set_position(util::Point<int>(shim::tile_size, shim::tile_size));

	gold += flare1->get_gold();
	experience += flare1->get_experience();

	wedge::Battle_Enemy *flare2 = new Enemy_Flare();
	flare2->start();
	flare2->set_position(util::Point<int>(1/*just shimmy it over a lil*/+shim::tile_size*2.0f+flare1->get_sprite()->get_current_image()->size.w, shim::tile_size*1.5f));

	gold += flare2->get_gold();
	experience += flare2->get_experience();
	
	wedge::Battle_Enemy *bones = new Enemy_Bones();
	bones->start();
	bones->set_position(util::Point<int>(shim::tile_size*3.0f+flare1->get_sprite()->get_current_image()->size.w*2.0f, shim::tile_size));

	gold += bones->get_gold();
	experience += bones->get_experience();

	entities.push_back(bones);
	entities.push_back(flare2);
	entities.push_back(flare1);

	return true;
}

wedge::Object Battle_1Bones2Flare::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 15);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION_PLUS;
		quantity = 1;
	}
	else if (r == 1) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_ELIXIR;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_1Reaper2Rocky::Battle_1Reaper2Rocky() :
	Monster_RPG_3_Battle_Game("mountains", 0)
{
}

Battle_1Reaper2Rocky::~Battle_1Reaper2Rocky()
{
}

bool Battle_1Reaper2Rocky::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *reaper = new Enemy_Reaper();
	reaper->start();
	reaper->set_position(util::Point<int>(shim::tile_size/2, shim::tile_size));

	gold += reaper->get_gold();
	experience += reaper->get_experience();
	
	wedge::Battle_Enemy *rocky1 = new Enemy_Rocky();
	rocky1->start();
	rocky1->set_position(util::Point<int>(shim::tile_size*1.5f+rocky1->get_sprite()->get_current_image()->size.w+rocky1->get_sprite()->get_current_image()->size.w, shim::tile_size*2/3));

	gold += rocky1->get_gold();
	experience += rocky1->get_experience();

	wedge::Battle_Enemy *rocky2 = new Enemy_Rocky();
	rocky2->start();
	rocky2->set_position(util::Point<int>(shim::tile_size*1.25f+rocky1->get_sprite()->get_current_image()->size.w, rocky1->get_sprite()->get_current_image()->size.h*1.1f));

	gold += rocky2->get_gold();
	experience += rocky2->get_experience();
	
	entities.push_back(rocky1);
	entities.push_back(rocky2);
	entities.push_back(reaper);

	return true;
}

wedge::Object Battle_1Reaper2Rocky::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 12);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION_OMEGA;
		quantity = 1;
	}
	else if (r == 1) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_ELIXIR;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_3Rocky::Battle_3Rocky() :
	Monster_RPG_3_Battle_Game("mountains", 0)
{
}

Battle_3Rocky::~Battle_3Rocky()
{
}

bool Battle_3Rocky::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *rocky3 = new Enemy_Rocky();
	rocky3->start();
	rocky3->set_position(util::Point<int>(shim::tile_size/2, shim::tile_size*2.25f));

	gold += rocky3->get_gold();
	experience += rocky3->get_experience();
	
	wedge::Battle_Enemy *rocky1 = new Enemy_Rocky();
	rocky1->start();
	rocky1->set_position(util::Point<int>(shim::tile_size+rocky1->get_sprite()->get_current_image()->size.w+rocky3->get_sprite()->get_current_image()->size.w, shim::tile_size*1.25f));

	gold += rocky1->get_gold();
	experience += rocky1->get_experience();

	wedge::Battle_Enemy *rocky2 = new Enemy_Rocky();
	rocky2->start();
	rocky2->set_position(util::Point<int>(shim::tile_size*3/4+rocky3->get_sprite()->get_current_image()->size.w, shim::tile_size*1.75f));

	gold += rocky2->get_gold();
	experience += rocky2->get_experience();
	
	entities.push_back(rocky1);
	entities.push_back(rocky2);
	entities.push_back(rocky3);

	return true;
}

wedge::Object Battle_3Rocky::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 16);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION_OMEGA;
		quantity = 1;
	}
	else if (r == 1) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_ELIXIR;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_2Wraith::Battle_2Wraith() :
	Monster_RPG_3_Battle_Game("mountains", 0)
{
}

Battle_2Wraith::~Battle_2Wraith()
{
}

bool Battle_2Wraith::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *wraith1 = new Enemy_Wraith();
	wraith1->start();
	wraith1->set_position(util::Point<int>(wraith1->get_sprite()->get_current_image()->size.w-shim::tile_size/2, shim::tile_size));
	entities.push_back(wraith1);

	gold += wraith1->get_gold();
	experience += wraith1->get_experience();

	wedge::Battle_Enemy *wraith2 = new Enemy_Wraith();
	wraith2->start();
	wraith2->set_position(util::Point<int>(shim::tile_size/2, shim::tile_size));
	entities.push_back(wraith2);

	gold += wraith2->get_gold();
	experience += wraith2->get_experience();

	return true;
}

wedge::Object Battle_2Wraith::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 15);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_HOLY_WATER;
		quantity = 1;
	}
	else if (r == 1) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION_OMEGA;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_1Reaper1Wraith::Battle_1Reaper1Wraith() :
	Monster_RPG_3_Battle_Game("mountains", 0)
{
}

Battle_1Reaper1Wraith::~Battle_1Reaper1Wraith()
{
}

bool Battle_1Reaper1Wraith::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *reaper = new Enemy_Reaper();
	reaper->start();
	reaper->set_position(util::Point<int>(shim::tile_size/2, shim::tile_size));

	gold += reaper->get_gold();
	experience += reaper->get_experience();

	wedge::Battle_Enemy *wraith = new Enemy_Wraith();
	wraith->start();
	wraith->set_position(util::Point<int>(wraith->get_sprite()->get_current_image()->size.w-shim::tile_size/2, shim::tile_size*5/6));

	gold += wraith->get_gold();
	experience += wraith->get_experience();

	entities.push_back(wraith);
	entities.push_back(reaper);

	return true;
}

wedge::Object Battle_1Reaper1Wraith::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 12);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_ELIXIR;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_2Wraith_Caves::Battle_2Wraith_Caves() :
	Monster_RPG_3_Battle_Game("caves", 0)
{
}

Battle_2Wraith_Caves::~Battle_2Wraith_Caves()
{
}

bool Battle_2Wraith_Caves::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *wraith1 = new Enemy_Wraith();
	wraith1->start();
	wraith1->set_position(util::Point<int>(wraith1->get_sprite()->get_current_image()->size.w-shim::tile_size/2, shim::tile_size));
	entities.push_back(wraith1);

	gold += wraith1->get_gold();
	experience += wraith1->get_experience();

	wedge::Battle_Enemy *wraith2 = new Enemy_Wraith();
	wraith2->start();
	wraith2->set_position(util::Point<int>(shim::tile_size/2, shim::tile_size));
	entities.push_back(wraith2);

	gold += wraith2->get_gold();
	experience += wraith2->get_experience();

	return true;
}

wedge::Object Battle_2Wraith_Caves::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 15);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_HOLY_WATER;
		quantity = 1;
	}
	else if (r == 1) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION_OMEGA;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_1Reaper1Wraith_Caves::Battle_1Reaper1Wraith_Caves() :
	Monster_RPG_3_Battle_Game("caves", 0)
{
}

Battle_1Reaper1Wraith_Caves::~Battle_1Reaper1Wraith_Caves()
{
}

bool Battle_1Reaper1Wraith_Caves::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *reaper = new Enemy_Reaper();
	reaper->start();
	reaper->set_position(util::Point<int>(shim::tile_size/2, shim::tile_size));

	gold += reaper->get_gold();
	experience += reaper->get_experience();

	wedge::Battle_Enemy *wraith = new Enemy_Wraith();
	wraith->start();
	wraith->set_position(util::Point<int>(wraith->get_sprite()->get_current_image()->size.w-shim::tile_size/2, shim::tile_size*5/6));

	gold += wraith->get_gold();
	experience += wraith->get_experience();

	entities.push_back(wraith);
	entities.push_back(reaper);

	return true;
}

wedge::Object Battle_1Reaper1Wraith_Caves::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 12);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_ELIXIR;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_3Shadow::Battle_3Shadow() :
	Monster_RPG_3_Battle_Game("caves", 0)
{
}

Battle_3Shadow::~Battle_3Shadow()
{
}

bool Battle_3Shadow::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *shadow1 = new Enemy_Shadow();
	shadow1->start();
	shadow1->set_position(util::Point<int>(shim::tile_size, shim::tile_size*2.5f));

	gold += shadow1->get_gold();
	experience += shadow1->get_experience();

	wedge::Battle_Enemy *shadow2 = new Enemy_Shadow();
	shadow2->start();
	shadow2->set_position(util::Point<int>(shim::tile_size*2+shadow1->get_sprite()->get_current_image()->size.w*2.0f, shim::tile_size*2.25f));

	gold += shadow2->get_gold();
	experience += shadow2->get_experience();

	wedge::Battle_Enemy *shadow3 = new Enemy_Shadow();
	shadow3->start();
	shadow3->set_position(util::Point<int>(shim::tile_size*3/2+shadow1->get_sprite()->get_current_image()->size.w, shim::tile_size*2.0f));

	gold += shadow3->get_gold();
	experience += shadow3->get_experience();
	
	entities.push_back(shadow2);
	entities.push_back(shadow3);
	entities.push_back(shadow1);

	return true;
}

wedge::Object Battle_3Shadow::get_found_object()
{
	wedge::Object_Type type = wedge::OBJECT_NONE;
	int id = 0;
	int quantity = 0;

	int r = util::rand(0, 8);

	if (r == 0) {
		type = wedge::OBJECT_ITEM;
		id = ITEM_POTION_OMEGA;
		quantity = 1;
	}

	return OBJECT->make_object(type, id, quantity);
}

//--

Battle_Gayan::Battle_Gayan() :
	Monster_RPG_3_Battle_Game("mountains", 0),
	played_grow(false),
	battle_ending(false)
{
	boss_battle = true;

	add_dialogue("", GLOBALS->game_t->translate(1663)/* Originally: Gayan is changing forms! */, wedge::DIALOGUE_MESSAGE, wedge::DIALOGUE_BOTTOM);
}

Battle_Gayan::~Battle_Gayan()
{
}

bool Battle_Gayan::start()
{
	if (Monster_RPG_3_Battle_Game::start() == false) {
		return false;
	}

	wedge::Battle_Enemy *gayan = new Enemy_Gayan();
	gayan->start();
	gayan->set_position(util::Point<int>(shim::tile_size, 0));
	entities.push_back(gayan);

	gold += gayan->get_gold();
	experience += gayan->get_experience();

	return true;
}

bool Battle_Gayan::run()
{
	if (played_grow == false) {
		played_grow = true;
		M3_GLOBALS->grow->play(false);
	}

	std::vector<wedge::Battle_Entity *> players = get_players();
	bool all_players_dead = true;
	for (size_t i = 0; i < players.size(); i++) {
		if (players[i]->get_stats()->hp > 0) {
			all_players_dead = false;
			break;
		}
	}

	std::vector<wedge::Battle_Entity *> enemies = get_enemies();
	bool all_enemies_dead = true;
	for (size_t i = 0; i < enemies.size(); i++) {
		if (enemies[i]->get_stats()->hp > 0) {
			all_enemies_dead = false;
			break;
		}
	}

	if (all_players_dead && all_enemies_dead && INSTANCE->is_milestone_complete(MS_USED_VFIRE)) {
		if (battle_ending == false) {
			battle_ending_time = GET_TICKS();
			battle_ending = true;
			BATTLE->show_enemy_stats(false);
			BATTLE->show_player_stats(false);
		}
		else if (GET_TICKS()-battle_ending_time > 4000) {
			set_done(true);
		}
	}

	return Monster_RPG_3_Battle_Game::run();
}
