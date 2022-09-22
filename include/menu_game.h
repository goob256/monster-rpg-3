#ifndef MENU_GAME_H
#define MENU_GAME_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

class Menu_Game : public wedge::Game
{
public:
	Menu_Game();
	virtual ~Menu_Game();

	bool start();
	bool run();
	void draw();
	void draw_fore();
};

#endif // MENU_GAME_H
