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

		if (argc < 9) {
			util::infomsg("Usage: %s <file.tga> <in_r> <in_g> <in_b> <out_r> <out_g> <out_b> <out.tga>\n", argv[0]);
			util::infomsg("Use -1, -1, -1 for 'in' to change all non-0 pixels\n");
			return 0;
		}

		int in_r = atoi(argv[2]);
		int in_g = atoi(argv[3]);
		int in_b = atoi(argv[4]);
		int out_r = atoi(argv[5]);
		int out_g = atoi(argv[6]);
		int out_b = atoi(argv[7]);

		gfx::Image_Base::premultiply_alpha = false;
		gfx::Image_Base::keep_data = true;
		gfx::Image_Base::save_rgba = true;

		gfx::Image *img = new gfx::Image(argv[1], true);

		unsigned char *data = img->get_loaded_data();

		for (int y = 0; y < img->size.h; y++) {
			unsigned char *p = data + y * (img->size.w * 4);
			for (int x = 0; x < img->size.w; x++) {
				if ((in_r == -1 && in_g == -1 && in_b == -1 && p[3] != 0) || (p[0] == in_r && p[1] == in_g && p[2] == in_b)) {
					p[0] = out_r;
					p[1] = out_g;
					p[2] = out_b;
				}
				p += 4;
			}
		}

		img->save(argv[8]);

		delete img;

		shim::end_all();

		shim::static_end();
	}
	catch (util::Error e) {
		util::errormsg("Fatal error: %s\n", e.error_message.c_str());
	}

	return 0;
}
