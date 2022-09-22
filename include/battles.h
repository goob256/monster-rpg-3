#ifndef BATTLES_H
#define BATTLES_H

#include <Nooskewl_Wedge/inventory.h>

#include "battle_game.h"

class Battle_Fiddler : public Monster_RPG_3_Battle_Game
{
public:
	Battle_Fiddler();
	virtual ~Battle_Fiddler();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_2Goos : public Monster_RPG_3_Battle_Game
{
public:
	Battle_2Goos();
	virtual ~Battle_2Goos();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_Mushroom : public Monster_RPG_3_Battle_Game
{
public:
	Battle_Mushroom();
	virtual ~Battle_Mushroom();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_3Bloated : public Monster_RPG_3_Battle_Game
{
public:
	Battle_3Bloated();
	virtual ~Battle_3Bloated();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_2Treant : public Monster_RPG_3_Battle_Game
{
public:
	Battle_2Treant();
	virtual ~Battle_2Treant();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_3Sludge : public Monster_RPG_3_Battle_Game
{
public:
	Battle_3Sludge();
	virtual ~Battle_3Sludge();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_2Ghastly : public Monster_RPG_3_Battle_Game
{
public:
	Battle_2Ghastly();
	virtual ~Battle_2Ghastly();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_1Ghastly2Sludge : public Monster_RPG_3_Battle_Game
{
public:
	Battle_1Ghastly2Sludge();
	virtual ~Battle_1Ghastly2Sludge();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_2Werewolf : public Monster_RPG_3_Battle_Game
{
public:
	Battle_2Werewolf();
	virtual ~Battle_2Werewolf();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_2Knightly : public Monster_RPG_3_Battle_Game
{
public:
	Battle_2Knightly();
	virtual ~Battle_2Knightly();

	bool start();

private:
	wedge::Object get_found_object();
};


class Battle_Palla : public Monster_RPG_3_Battle_Game
{
public:
	Battle_Palla();
	virtual ~Battle_Palla();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_3Tentacle : public Monster_RPG_3_Battle_Game
{
public:
	Battle_3Tentacle();
	virtual ~Battle_3Tentacle();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_Wave : public Monster_RPG_3_Battle_Game
{
public:
	Battle_Wave();
	virtual ~Battle_Wave();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_3Shocker : public Monster_RPG_3_Battle_Game
{
public:
	Battle_3Shocker();
	virtual ~Battle_3Shocker();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_1Shocker2Tentacle : public Monster_RPG_3_Battle_Game
{
public:
	Battle_1Shocker2Tentacle();
	virtual ~Battle_1Shocker2Tentacle();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_Monster : public Monster_RPG_3_Battle_Game
{
public:
	Battle_Monster();
	virtual ~Battle_Monster();

	bool start();
	bool run();

private:
	wedge::Object get_found_object();
	bool made_noise;
	audio::Sound *monster_sound;
};

class Battle_1Sandworm2Flare : public Monster_RPG_3_Battle_Game
{
public:
	Battle_1Sandworm2Flare();
	virtual ~Battle_1Sandworm2Flare();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_3Cyclone : public Monster_RPG_3_Battle_Game
{
public:
	Battle_3Cyclone();
	virtual ~Battle_3Cyclone();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_2Bones : public Monster_RPG_3_Battle_Game
{
public:
	Battle_2Bones();
	virtual ~Battle_2Bones();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_1Bones2Flare : public Monster_RPG_3_Battle_Game
{
public:
	Battle_1Bones2Flare();
	virtual ~Battle_1Bones2Flare();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_1Reaper2Rocky : public Monster_RPG_3_Battle_Game
{
public:
	Battle_1Reaper2Rocky();
	virtual ~Battle_1Reaper2Rocky();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_3Rocky : public Monster_RPG_3_Battle_Game
{
public:
	Battle_3Rocky();
	virtual ~Battle_3Rocky();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_2Wraith : public Monster_RPG_3_Battle_Game
{
public:
	Battle_2Wraith();
	virtual ~Battle_2Wraith();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_1Reaper1Wraith : public Monster_RPG_3_Battle_Game
{
public:
	Battle_1Reaper1Wraith();
	virtual ~Battle_1Reaper1Wraith();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_2Wraith_Caves : public Monster_RPG_3_Battle_Game
{
public:
	Battle_2Wraith_Caves();
	virtual ~Battle_2Wraith_Caves();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_1Reaper1Wraith_Caves : public Monster_RPG_3_Battle_Game
{
public:
	Battle_1Reaper1Wraith_Caves();
	virtual ~Battle_1Reaper1Wraith_Caves();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_3Shadow : public Monster_RPG_3_Battle_Game
{
public:
	Battle_3Shadow();
	virtual ~Battle_3Shadow();

	bool start();

private:
	wedge::Object get_found_object();
};

class Battle_Gayan : public Monster_RPG_3_Battle_Game
{
public:
	Battle_Gayan();
	virtual ~Battle_Gayan();

	bool start();
	bool run();

private:
	bool played_grow;
	bool battle_ending;
	Uint32 battle_ending_time;
};

#endif // BATTLES_H
