#ifndef MONSTER_RPG_3_H
#define MONSTER_RPG_3_H

#define SCR_W 144
#define SCR_H 81

#ifdef _WIN32
#define STRUCT_ALIGN(x, n) __declspec(align(n)) struct x
#define CLASS_ALIGN(x, n) __declspec(align(n)) class x
#else
#define STRUCT_ALIGN(x, n) struct __attribute__((aligned(n))) x
#define CLASS_ALIGN(x, n) class __attribute__((aligned(n))) x
#endif

// we mix sfx a little higher than music
#define sfx_amp 1.0f
#define music_amp 0.8f

extern util::Size<int> desktop_resolution;
extern bool reload_m3;
extern bool force_windowed;
extern bool force_fullscreen;
extern util::Size<int> force_screen_size;
extern int tmp_key_b1;
extern int tmp_key_b2;
extern int tmp_key_b3;
extern int tmp_key_b4;
extern int tmp_key_switch;
extern int tmp_key_fs;
extern int tmp_key_l;
extern int tmp_key_r;
extern int tmp_key_u;
extern int tmp_key_d;
extern int tmp_joy_b1;
extern int tmp_joy_b2;
extern int tmp_joy_b3;
extern int tmp_joy_b4;
extern int tmp_joy_switch;
extern bool tmp_tv_safe_mode;
extern bool tmp_hide_onscreen_settings_button;
extern bool tmp_rumble_enabled;
extern bool tmp_use_onscreen_controller;
extern std::string tmp_language;
#if defined __linux__ && !defined ANDROID
extern bool prompt_to_install_desktop;
#endif

extern float sfx_volume;
extern float music_volume;
extern std::vector<std::string> cfg_args;
extern std::vector<std::string> bak_args;

void delete_shim_args();
void set_shim_args(bool keep_initial);

#endif // MONSTER_RPG_3_H
