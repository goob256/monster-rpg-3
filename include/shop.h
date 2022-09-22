#ifndef SHOP_H
#define SHOP_H

#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/inventory.h>

#include "gui.h"
#include "widgets.h"

class Shop_GUI : public Menu_GUI {
public:
	struct Buy_Callback_Data {
		wedge::Object object; // price is in quantity
		int quantity;
		Shop_GUI *gui;
	};

	struct Sell_Callback_Data {
		int index;
		int quantity;
		Shop_GUI *gui;
	};

	Shop_GUI(int character_index, int top, int selected, bool buying, wedge::Object_Type type, std::vector<wedge::Object> items/*price goes in quantity*/);
	virtual ~Shop_GUI();

	void draw_fore();
	void handle_event(TGUI_Event *event);
	void update();

	void check_equipment(); // check if you sold your equipped stuff
	void set_text(int top, int selected);
	void layout();

private:
	void confirm();

	std::vector<int> inventory_indices;

	bool buying;
	wedge::Object_Type type;
	std::vector<wedge::Object> items;

	Widget_Window *window;
	Widget_Image *profile_pic;
	Widget_Label *status_label;
	Widget_Label *line1_label;
	Widget_Label *line2_label;
	Widget_Label *line1;
	Widget_Label *line2;
	Widget_Label *gold;
	Widget_Quantity_List *list;
};

#endif // SHOP_H
