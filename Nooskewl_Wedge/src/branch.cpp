#include "Nooskewl_Wedge/branch.h"

using namespace wedge;

namespace wedge {

Branch_Step::Branch_Step(int *i, Game *game, std::vector< std::vector< std::vector< Step * > > > steps, Task *task) :
	Step(task),
	i(i),
	game(game),
	steps(steps)
{
}

Branch_Step::~Branch_Step()
{
}

bool Branch_Step::run()
{
	for (size_t j = 0; j < steps.size(); j++) {
		if ((int)j != *i) {
			for (size_t k = 0; k < steps[j].size(); k++) {
				for (size_t l = 0; l < steps[j][k].size(); l++) {
					delete steps[j][k][l];
				}
			}
		}
	}

	for (size_t j = 0; j < steps[*i].size(); j++) {
		NEW_SYSTEM_AND_TASK(game)
		for (size_t k = 0; k < steps[*i][j].size(); k++) {
			ADD_STEP(steps[*i][j][k])
		}
		ADD_TASK(new_task)
		// run the system right away so there's no glitch delay
		if (new_system->run() != false) {
			FINISH_SYSTEM(game)
		}
		else {
			delete new_system;
		}
	}

	send_done_signal();

	return false;
}

}
