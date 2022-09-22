#include <Nooskewl_Wedge/main.h>
#include <Nooskewl_Wedge/globals.h>

#include "globals.h"

#define SCR_W 144
#define SCR_H 81

int main(int argc, char **argv)
{
	try {
#ifdef _WIN32
		SDL_RegisterApp("WedgeExample", 0, 0);
#endif

		shim::window_title = "Nooskewl Wedge Example";
		shim::organization_name = "Nooskewl";
		shim::game_name = "Nooskewl Wedge Example";
		shim::convert_xbox_dpad_to_arrows = true;
		shim::tile_size = 12;
		gfx::Image::ignore_palette = true;

		if (wedge::start(argc, argv, util::Size<int>(SCR_W, SCR_H)) == false) {
			util::debugmsg("wedge::start returned false.\n");
			return 1;
		}

		shim::white = shim::palette[20]; // use our own white, it's actual white but could change
		shim::black = shim::palette[27]; // black doesn't exist in our palette

		wedge::globals = new Globals();

		if (wedge::go() == false) {
			util::debugmsg("wedge::go return false.\n");
			return 1;
		}
	}
	catch (util::Error e) {
		util::errormsg("Fatal error: %s\n", e.error_message.c_str());
		gui::popup("Fatal Error", e.error_message, gui::OK);
		return 1;
	}

	return 0;
}
