#ifndef NOO_SPRITE_H
#define NOO_SPRITE_H

#include "Nooskewl_Shim/main.h"

namespace noo {

namespace gfx {

class Image;

class NOOSKEWL_SHIM_EXPORT Sprite {
public:
	static void static_start();
	static void update_all(); // called each logic tick from shim::update

	Sprite(std::string json_filename, std::string image_directory, bool absolute_path = false);
	Sprite(std::string image_directory);
	~Sprite();

	bool set_animation(std::string name, util::Callback finished_callback = 0, void *finished_callback_data = 0);
	std::string get_animation();
	std::string get_previous_animation();

	void start();
	void stop();
	void reset(); // set to frame 0
	bool is_started();
	bool is_finished();

	int get_current_frame();
	int get_num_frames();
	std::vector<Uint32> get_frame_times();
	
	int get_length();

	Image *get_current_image();
	Image *get_image(int frame);

	void set_reverse(bool reverse); // play from back to front
	bool is_reversed();

	void get_filenames(std::string &json_filename, std::string &image_directory);

	void sync_with(Sprite *sprite, bool match_animation = false); // match timing with input sprite

	void get_bounds(util::Point<int> &topleft, util::Point<int> &bottomright);

private:
	struct Animation {
		Image *parent;
		std::vector<Image *> images;
		std::vector<Uint32> delays;
		Uint32 total_delays;
		bool rand_start;
		bool looping;
		util::Point<int> opaque_topleft;
		util::Point<int> opaque_bottomright;
	};

	struct Instance {
		bool started;
		Uint32 elapsed;
		// "" when not set
		std::string current_animation;
		util::Callback finished_callback;
		void *finished_callback_data;
		std::map<std::string, Animation *> animations;
	};

	struct Loaded_Sprite {
		std::vector<Instance *> instances;
	};

	void load(std::string json_filename, std::string image_directory, bool absolute_path = false);
	void update();

	Instance *instance;
	std::string previous_animation;
	bool reverse;

	std::string json_filename;
	std::string image_directory;

	static std::map<std::string, Loaded_Sprite *> loaded_sprites;
};

} // End namespace gfx

} // End namespace noo

#endif // NOO_SPRITE_H
