#include "Nooskewl_Shim/Nooskewl_Shim.h"

using namespace noo;

int main(int argc, char **argv)
{
	try {
		shim::static_start_all();

		shim::argc = argc;
		shim::argv = argv;
		shim::hide_window = true;
		shim::use_cwd = true;
		shim::error_level = 3;
		shim::log_tags = false;

		shim::start_all();

		if (argc < 3) {
			util::infomsg("Usage: %s <in.map> <out.map>\n", argv[0]);
			return 0;
		}

		gfx::Image::keep_data = true;
		shim::tile_size = 12;

		gfx::Tilemap *input = new gfx::Tilemap(argv[1], true);

		input->save(argv[2]);

		delete input;

		shim::end_all();

		shim::static_end();
	}
	catch (util::Error e) {
		util::errormsg("Fatal error: %s\n", e.error_message.c_str());
	}

	return 0;
}
