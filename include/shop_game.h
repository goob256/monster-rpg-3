#ifndef SHOP_GAME_H
#define SHOP_GAME_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/systems.h>

class Shop_Game : public wedge::Game
{
public:
	Shop_Game(wedge::Object_Type type, std::vector<wedge::Object> items/*price goes in quantity*/);
	virtual ~Shop_Game();

	bool start();
	bool run();
	void draw();
	void draw_fore();

private:
	wedge::Object_Type type;
	std::vector<wedge::Object> items;
};

#endif // SHOP_GAME_H
