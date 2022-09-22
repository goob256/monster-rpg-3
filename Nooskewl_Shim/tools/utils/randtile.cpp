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

		if (argc < 8) {
			util::infomsg("Usage: %s <in.map> <in_x> <in_y> <out_x> <out_y> <one_in> <out.map>\n", argv[0]);
			util::infomsg("one_in is 1 for every tile, 2 for 1/2 tiles etc\n");
			return 0;
		}

		gfx::Image::keep_data = true;
		shim::tile_size = 12;

		gfx::Tilemap *input = new gfx::Tilemap(argv[1], true);

		int in_x = atoi(argv[2]);
		int in_y = atoi(argv[3]);
		int out_x = atoi(argv[4]);
		int out_y = atoi(argv[5]);

		int one_in = atoi(argv[6]);

		util::Size<int> size = input->get_size();
		int nlayers = input->get_num_layers();

		for (int y = 0; y < size.h; y++) {
			for (int x = 0; x < size.w; x++) {
				util::Point<int> pos(x, y);
				util::Point<int> tile_xy;
				bool solid;
				for (int l = 0; l < nlayers; l++) {
					if (input->get_tile(l, pos, tile_xy, solid)) {
						if (tile_xy.x == in_x && tile_xy.y == in_y && 1 == util::rand(1, one_in)) {
							tile_xy.x = out_x;
							tile_xy.y = out_y;
							input->set_tile(l, pos, tile_xy, solid);
						}
					}
				}
			}
		}

		input->save(argv[7]);

		delete input;

		shim::end_all();

		shim::static_end();
	}
	catch (util::Error e) {
		util::errormsg("Fatal error: %s\n", e.error_message.c_str());
	}

	return 0;
}
