#ifndef SAVE_SLOT_H
#define SAVE_SLOT_H

#include <Nooskewl_Wedge/main.h>

#include "monster_rpg_3.h"
#include "sliding_menu.h"
#include "widgets.h"

CLASS_ALIGN(Save_Slot_GUI : public Sliding_Menu_GUI, 16) {
public:
	static const int NUM_SLOTS = 5;

	struct Callback_Data {
		int slot;
		bool exists;
		void *userdata;
		bool is_auto;
	};

	Save_Slot_GUI(bool is_save, int start_selection, util::Callback callback, void *callback_data, bool is_auto = false);
	virtual ~Save_Slot_GUI();

	void handle_event(TGUI_Event *event);
	void update();
	void draw();
	void draw_fore();

	void load();
	void erase();

#ifdef _WIN32
	void *operator new(size_t i);
	void operator delete(void* p);
#endif

private:
	void set_text();
	void change_guis();
	float get_change_p();

	util::Callback callback;
	void *callback_data;

	bool exists[NUM_SLOTS];
	bool corrupt[NUM_SLOTS];

	int pressed;

	bool is_save;
	bool is_auto;

	Save_Slot_GUI *next_gui;
	Uint32 next_gui_start;
	bool changing_guis;

	glm::mat4 old_mv, old_p;
};

#endif // SAVE_SLOT_H
