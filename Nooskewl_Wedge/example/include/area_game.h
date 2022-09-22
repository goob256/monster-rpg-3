#ifndef AREA_GAME_H
#define AREA_GAME_H

#include <Nooskewl_Wedge/area.h>
#include <Nooskewl_Wedge/area_game.h>

class Area_Game : public wedge::Area_Game
{
public:
	virtual ~Area_Game();

	wedge::Area_Hooks *get_area_hooks(std::string area_name, wedge::Area *area);
	void draw();
};

#endif // AREA_GAME_H
