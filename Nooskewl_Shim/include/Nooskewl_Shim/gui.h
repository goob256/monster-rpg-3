#ifndef NOO_GUI_H
#define NOO_GUI_H

#include "Nooskewl_Shim/main.h"
#include "Nooskewl_Shim/shim.h"
#include "Nooskewl_Shim/translation.h"

namespace noo {

namespace gui {

EXPORT_CLASS_ALIGN(GUI, 16) {
public:
	TGUI *gui;
	TGUI_Widget *focus; // backup focus

	GUI();
	virtual ~GUI();

	bool is_transitioning_out();
	bool is_transition_out_finished();

	virtual void handle_event(TGUI_Event *event);

	virtual void update();
	virtual void update_background(); // called when the GUI is not the foremost

	void pre_draw(); // special stuff (starts transition timer)

	virtual void draw_back();
	virtual void draw();
	virtual void draw_fore();

	virtual void resize(util::Size<int> new_size);

	virtual bool is_fullscreen(); // if the top gui returns true, other guis don't get drawn

	bool do_return(bool ret);

	virtual bool transition_done(bool transition_in); // return true to cancel and keep this GUI alive

	virtual void transition_start(float p);
	virtual void transition_end();

	// normally a fade is done if transitions are enabled, but these can be used instead
	void use_enlarge_transition(bool onoff);
	void use_shrink_transition(bool onoff);

	virtual void exit(); // call this to exit this GUI and remove it from shim::guis after transition and update()

	virtual void lost_device();
	virtual void found_device();

	virtual void transition_in_done(); // called when transition in is done (only if transition is true)

	void set_transition(bool transition);

	// For 16 byte alignment to make glm::mat4 able to use SIMD
#ifdef _WIN32
	void *operator new(size_t i);
	void operator delete(void* p);
#endif

protected:
	static const int MAX_FADE_SCALE = 10;

	void fade_transition(float p);
	void scale_transition(float scale);

	bool transition;
	bool transitioning_in;
	bool transitioning_out;
	Uint32 transition_start_time;
	bool transition_is_enlarge;
	bool transition_is_shrink;
	glm::mat4 mv_backup;
	float last_transition_p;
	bool started_transition_timer;
};

enum Popup_Type {
	OK = 0,
	YESNO = 1
};

// Functions
int NOOSKEWL_SHIM_EXPORT popup(std::string caption, std::string text, Popup_Type type);

} // End namespace gui

} // End namespace noo

#endif // NOO_GUI_H
