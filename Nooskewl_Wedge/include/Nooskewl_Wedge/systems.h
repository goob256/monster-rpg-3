#ifndef NOOSKEWL_WEDGE_SYSTEMS_H
#define NOOSKEWL_WEDGE_SYSTEMS_H

#include "Nooskewl_Wedge/main.h"

namespace wedge {

// Systems conveniences
#define NEW_SYSTEM_AND_TASK(game) \
	wedge::System *new_system = new wedge::System(game); \
	wedge::Task *new_task = new wedge::Task(new_system);
#define ANOTHER_SYSTEM_AND_TASK(game) \
	new_system = new wedge::System(game); \
	new_task = new wedge::Task(new_system);
#define ANOTHER_TASK \
	new_task = new wedge::Task(new_system);
#define ADD_STEP(s) \
	new_task->get_steps().push_back(s);
#define ADD_TASK(t) \
	new_system->get_tasks().push_back(t);
#define FINISH_SYSTEM(game) \
	game->get_systems().push_back(new_system);

class Step;
class Task;
class System;
class Game;

typedef std::list<Step *> Step_List;
typedef std::list<Task *> Task_List;
typedef std::list<System *> System_List;

class NOOSKEWL_WEDGE_EXPORT Step
{
public:
	Step(Task *task);
	virtual ~Step();

	virtual bool run(); // return false to pop this Step and advance to the next
	virtual void handle_event(TGUI_Event *event); // called for every event
	virtual void draw();
	virtual void draw_fore();
	virtual void start(); // called upon first use of this step, eg after finishing previous step
	virtual void lost_device();
	virtual void found_device();

	bool is_initialised();
	void set_initialised(bool initialised);

	Task *get_task();

	void add_monitor(Step *step); // send certain signals to step, see below
	void remove_monitor(Step *step);
	// monitoring Steps receive the following signals:
	virtual void done_signal(Step *step); // when the current step is finished, parameter is current step

	void set_monitoring(Step *step); // this is monitoring step. NEVER CALL THIS DIRECTLY
	void stop_monitoring(Step *step); // NEVER CALL THIS DIRECTLY

	void die();
	bool is_dying();
	bool is_disowned();

protected:
	void send_done_signal();
	void remove_monitors();
	void disown();

	Task *task;
	Step_List monitors;
	Step_List monitoring;
	bool step_initialised;
	bool dying;
	bool disowned;
};

class NOOSKEWL_WEDGE_EXPORT Task
{
public:
	Task(System *system);
	virtual ~Task();

	virtual bool run();
	virtual void handle_event(TGUI_Event *event);
	virtual void draw();
	virtual void draw_fore();
	virtual void lost_device();
	virtual void found_device();

	void set_paused(bool paused);
	bool is_paused();

	Step_List &get_steps();
	void remove_step(Step *step);

	System *get_system();

protected:
	Step_List steps;
	System *system;
	bool paused;
};

class NOOSKEWL_WEDGE_EXPORT System
{
public:
	System(Game *game);
	virtual ~System();

	virtual bool run();
	virtual void handle_event(TGUI_Event *event);
	virtual void draw();
	virtual void draw_fore();
	virtual void lost_device();
	virtual void found_device();

	void set_paused(bool paused);
	bool is_paused();

	Task_List &get_tasks();
	void remove_task(Task *task);

	Game *get_game();

protected:
	Task_List tasks;
	Game *game;
	bool paused;
};

class NOOSKEWL_WEDGE_EXPORT Game
{
public:
	Game();
	virtual ~Game();

	virtual bool start();
	virtual void handle_event(TGUI_Event *event);
	virtual bool run();
	virtual void draw();
	virtual void draw_fore();
	virtual void lost_device();
	virtual void found_device();
	virtual void resize(util::Size<int> new_size);

	System_List &get_systems();
	void remove_system(System *system);

protected:
	System_List systems;
};

}

#endif // NOOSKEWL_WEDGE_SYSTEMS_H
