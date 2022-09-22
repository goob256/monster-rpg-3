#ifndef GENERAL_H
#define GENERAL_H

#include <Nooskewl_Wedge/globals.h>

class Dialogue_Step;
class Inn_Step;
class Question_Step;

void do_question(std::string tag, std::string text, wedge::Dialogue_Type type, std::vector<std::string> choices, wedge::Step *monitor); // always positioned at top

int num_bands(int height);

std::string get_status_name(int hp, int status);
SDL_Colour get_status_colour(int hp, int status);

void start_pulse_brighten(float amount, bool need_untextured, bool need_textured, int cycle = 500);
void end_pulse_brighten();
SDL_Colour *start_bander(int bands, SDL_Colour start, SDL_Colour end, float alpha = 1.0f);
void end_bander();
float get_little_bander_offset();
SDL_Colour brighten(SDL_Colour colour, float amount);

std::string save_dir();
std::string save_filename(int slot, std::string prefix = "save");
bool save();
bool save_play_time();

std::string play_time_to_string(int time);

void draw_shadowed_image(SDL_Colour shadow_colour, gfx::Image *image, util::Point<float> dest_pos, gfx::Font::Shadow_Type shadow_type);
void draw_tinted_shadowed_image(SDL_Colour tint, SDL_Colour shadow_colour, gfx::Image *image, util::Point<float> dest_pos, gfx::Font::Shadow_Type shadow_type);

bool load_settings(bool globals_created);
bool save_settings();

std::string get_key_name(int code);
std::string get_joystick_button_name(int button);

void show_save_screen(wedge::Step *monitor);

bool settings_active();
// The following function is actually blown out of its original purpose and should probably be renamed
bool can_show_settings(bool check_events = false, bool can_be_moving = false, bool only_initialised_dialogues_block = false, bool allow_paused_presses_if_changing_areas = false);
void show_settings();

float get_ship_angle();

void apply_tv_safe_mode(bool onoff);

void get_use_item_info(int amount, int id, SDL_Colour &colour, SDL_Colour &shadow_colour, std::string &text);

bool is_mapped_key(int code);

SDL_Colour make_translucent(SDL_Colour colour, float alpha);

std::vector<Dialogue_Step *> active_dialogues(wedge::Game *game);
std::vector<Inn_Step *> active_inns();
std::vector<Question_Step *> active_questions();

void barycenter(float *v1, float *v2, float *v3, float *x, float *y, float *z);

void load_translation();

util::JSON *load_savegame(int slot, std::string prefix = "save");

#ifdef TVOS
void save_autosave_index();
#endif
void autosave(bool wait);

void start_autosave_thread();
void end_autosave_thread();

bool can_autosave();

void show_notice(std::string text, bool flip = false);

#endif // GENERAL_H
