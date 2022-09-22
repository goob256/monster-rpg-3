#ifndef INN_H
#define INN_H

#include <Nooskewl_Wedge/generic_gui.h>
#include <Nooskewl_Wedge/globals.h>
#include <Nooskewl_Wedge/systems.h>

class Inn_Step : public wedge::Step
{
public:
	struct Callback_Data {
		int count;
		wedge::Generic_GUI_Step *multiple_choice_gui_step;
		Inn_Step *inn_step;
	};

	/* beds must be side by side, one tile between, eny takes left tiggy takes right then they meet in the middle after sleeping */
	Inn_Step(wedge::Dialogue_Type type, std::string innkeep_name, std::string offer, std::string nope, std::string have_cash, std::string no_cash, int cost, util::Point<int> bed_middle_pos, wedge::Task *task, wedge::Step *monitor_step = NULL);
	virtual ~Inn_Step();

	void start();
	bool run();
	void done_signal(Step *step);

	int get_choice();
	void set_choice(int choice);

	void reposition_players();
	void set_player_positions(util::Point<int> player_pos, util::Point<int> tiggy_pos);

	void set_done(bool done);

private:
	bool done;
	int signal_count;
	wedge::Dialogue_Type type;
	std::string innkeep_name;
	std::string offer;
	std::string nope;
	std::string have_cash;
	std::string no_cash;
	int cost;
	util::Point<int> bed_middle_pos;

	Callback_Data callback_data;

	util::Point<int> player_pos;
	util::Point<int> tiggy_pos;

	bool sent_done_signal;
	wedge::Step *monitor_step;
	bool do_sleep;
};

#endif // INN_H
