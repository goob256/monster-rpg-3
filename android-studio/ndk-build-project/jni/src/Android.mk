LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL2
TGUI3_PATH := ../../../../tgui3
SHIM_PATH := ../../../../Nooskewl_Shim
WEDGE_PATH := ../../../../Nooskewl_Wedge
M3_PATH := ../../../../../monster-rpg-3
ANDROID_DIR := ../../../../../android.older

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include \
	$(LOCAL_PATH)/$(TGUI3_PATH)/include \
	$(LOCAL_PATH)/$(SHIM_PATH)/include \
	$(LOCAL_PATH)/$(WEDGE_PATH)/include \
	$(LOCAL_PATH)/$(M3_PATH)/include \
	$(LOCAL_PATH)/$(ANDROID_DIR)/$(TARGET_ARCH_ABI)/include \

# Add your application source files here...
LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.c \
	$(TGUI3_PATH)/src/tgui3.cpp \
	$(TGUI3_PATH)/src/tgui3_sdl.cpp \
	$(SHIM_PATH)/src/a_star.cpp \
	$(SHIM_PATH)/src/achievements.cpp \
	$(SHIM_PATH)/src/audio.cpp \
	$(SHIM_PATH)/src/cd.cpp \
	$(SHIM_PATH)/src/cpa.cpp \
	$(SHIM_PATH)/src/crash.cpp \
	$(SHIM_PATH)/src/shim.cpp \
	$(SHIM_PATH)/src/error.cpp \
	$(SHIM_PATH)/src/font.cpp \
	$(SHIM_PATH)/src/gfx.cpp \
	$(SHIM_PATH)/src/gui.cpp \
	$(SHIM_PATH)/src/image.cpp \
	$(SHIM_PATH)/src/image_base.cpp \
	$(SHIM_PATH)/src/input.cpp \
	$(SHIM_PATH)/src/json.cpp \
	$(SHIM_PATH)/src/mml.cpp \
	$(SHIM_PATH)/src/model.cpp \
	$(SHIM_PATH)/src/mt19937ar.cpp \
	$(SHIM_PATH)/src/pixel_font.cpp \
	$(SHIM_PATH)/src/primitives.cpp \
	$(SHIM_PATH)/src/sample.cpp \
	$(SHIM_PATH)/src/shader.cpp \
	$(SHIM_PATH)/src/sound.cpp \
	$(SHIM_PATH)/src/sprite.cpp \
	$(SHIM_PATH)/src/tilemap.cpp \
	$(SHIM_PATH)/src/tokenizer.cpp \
	$(SHIM_PATH)/src/translation.cpp \
	$(SHIM_PATH)/src/utf8.cpp \
	$(SHIM_PATH)/src/util.cpp \
	$(SHIM_PATH)/src/vertex_cache.cpp \
	$(SHIM_PATH)/src/vorbis.cpp \
	$(WEDGE_PATH)/src/area.cpp \
	$(WEDGE_PATH)/src/area_game.cpp \
	$(WEDGE_PATH)/src/a_star.cpp \
	$(WEDGE_PATH)/src/battle_end.cpp \
	$(WEDGE_PATH)/src/battle_enemy.cpp \
	$(WEDGE_PATH)/src/battle_entity.cpp \
	$(WEDGE_PATH)/src/battle_game.cpp \
	$(WEDGE_PATH)/src/battle_player.cpp \
	$(WEDGE_PATH)/src/branch.cpp \
	$(WEDGE_PATH)/src/check_positions.cpp \
	$(WEDGE_PATH)/src/chest.cpp \
	$(WEDGE_PATH)/src/delay.cpp \
	$(WEDGE_PATH)/src/delete_map_entity.cpp \
	$(WEDGE_PATH)/src/fade.cpp \
	$(WEDGE_PATH)/src/game.cpp \
	$(WEDGE_PATH)/src/general.cpp \
	$(WEDGE_PATH)/src/generic_callback.cpp \
	$(WEDGE_PATH)/src/generic_immediate_callback.cpp \
	$(WEDGE_PATH)/src/generic_gui.cpp \
	$(WEDGE_PATH)/src/give_object.cpp \
	$(WEDGE_PATH)/src/globals.cpp \
	$(WEDGE_PATH)/src/input.cpp \
	$(WEDGE_PATH)/src/inventory.cpp \
	$(WEDGE_PATH)/src/look_around_input.cpp \
	$(WEDGE_PATH)/src/main.cpp \
	$(WEDGE_PATH)/src/map_entity.cpp \
	$(WEDGE_PATH)/src/npc.cpp \
	$(WEDGE_PATH)/src/omnipresent.cpp \
	$(WEDGE_PATH)/src/onscreen_controller.cpp \
	$(WEDGE_PATH)/src/pause_player_input.cpp \
	$(WEDGE_PATH)/src/pause_presses.cpp \
	$(WEDGE_PATH)/src/pause_sprite.cpp \
	$(WEDGE_PATH)/src/pause_task.cpp \
	$(WEDGE_PATH)/src/play_animation.cpp \
	$(WEDGE_PATH)/src/play_music.cpp \
	$(WEDGE_PATH)/src/play_sound.cpp \
	$(WEDGE_PATH)/src/player_input.cpp \
	$(WEDGE_PATH)/src/set_animation.cpp \
	$(WEDGE_PATH)/src/set_direction.cpp \
	$(WEDGE_PATH)/src/set_integer.cpp \
	$(WEDGE_PATH)/src/set_music_volume.cpp \
	$(WEDGE_PATH)/src/set_solid.cpp \
	$(WEDGE_PATH)/src/set_visible.cpp \
	$(WEDGE_PATH)/src/shop_step.cpp \
	$(WEDGE_PATH)/src/slide_entity.cpp \
	$(WEDGE_PATH)/src/special_number.cpp \
	$(WEDGE_PATH)/src/spells.cpp \
	$(WEDGE_PATH)/src/stats.cpp \
	$(WEDGE_PATH)/src/step.cpp \
	$(WEDGE_PATH)/src/stop_music.cpp \
	$(WEDGE_PATH)/src/stop_sound.cpp \
	$(WEDGE_PATH)/src/system.cpp \
	$(WEDGE_PATH)/src/task.cpp \
	$(WEDGE_PATH)/src/tile_movement.cpp \
	$(WEDGE_PATH)/src/wait.cpp \
	$(WEDGE_PATH)/src/wait_for_integer.cpp \
	$(WEDGE_PATH)/src/wander_input.cpp \
	$(M3_PATH)/src/area_game.cpp \
	$(M3_PATH)/src/audio_settings.cpp \
	$(M3_PATH)/src/autosave.cpp \
	$(M3_PATH)/src/battle_game.cpp \
	$(M3_PATH)/src/battle_player.cpp \
	$(M3_PATH)/src/battle_transition_in.cpp \
	$(M3_PATH)/src/battle_transition_in2.cpp \
	$(M3_PATH)/src/battle_transition_out.cpp \
	$(M3_PATH)/src/battle_transition_out2.cpp \
	$(M3_PATH)/src/battles.cpp \
	$(M3_PATH)/src/bolt.cpp \
	$(M3_PATH)/src/buy_scroll.cpp \
	$(M3_PATH)/src/captain.cpp \
	$(M3_PATH)/src/controls_settings.cpp \
	$(M3_PATH)/src/cure.cpp \
	$(M3_PATH)/src/custom_slide_entity.cpp \
	$(M3_PATH)/src/dialogue.cpp \
	$(M3_PATH)/src/enemies.cpp \
	$(M3_PATH)/src/enemy_drawing_hook.cpp \
	$(M3_PATH)/src/fire.cpp \
	$(M3_PATH)/src/fishing.cpp \
	$(M3_PATH)/src/fishing_prize.cpp \
	$(M3_PATH)/src/general.cpp \
	$(M3_PATH)/src/globals.cpp \
	$(M3_PATH)/src/gui.cpp \
	$(M3_PATH)/src/gui_drawing_hook.cpp \
	$(M3_PATH)/src/heal.cpp \
	$(M3_PATH)/src/hit.cpp \
	$(M3_PATH)/src/ice.cpp \
	$(M3_PATH)/src/inn.cpp \
	$(M3_PATH)/src/inventory.cpp \
	$(M3_PATH)/src/jump.cpp \
	$(M3_PATH)/src/language_settings.cpp \
	$(M3_PATH)/src/menu.cpp \
	$(M3_PATH)/src/menu_game.cpp \
	$(M3_PATH)/src/monster_rpg_3.cpp \
	$(M3_PATH)/src/other_settings.cpp \
	$(M3_PATH)/src/play_animation_and_delete.cpp \
	$(M3_PATH)/src/question.cpp \
	$(M3_PATH)/src/ran_away.cpp \
	$(M3_PATH)/src/revive_entity.cpp \
	$(M3_PATH)/src/sailor.cpp \
	$(M3_PATH)/src/sailor_npc.cpp \
	$(M3_PATH)/src/sailship.cpp \
	$(M3_PATH)/src/save_slot.cpp \
	$(M3_PATH)/src/scroll_help.cpp \
	$(M3_PATH)/src/settings.cpp \
	$(M3_PATH)/src/sliding_menu.cpp \
	$(M3_PATH)/src/shop.cpp \
	$(M3_PATH)/src/shop_game.cpp \
	$(M3_PATH)/src/spells.cpp \
	$(M3_PATH)/src/start_battle.cpp \
	$(M3_PATH)/src/transition.cpp \
	$(M3_PATH)/src/vampires.cpp \
	$(M3_PATH)/src/vbolt.cpp \
	$(M3_PATH)/src/vfire.cpp \
	$(M3_PATH)/src/vice.cpp \
	$(M3_PATH)/src/video_settings.cpp \
	$(M3_PATH)/src/widgets.cpp \

LOCAL_SHARED_LIBRARIES := SDL2

LOCAL_CFLAGS := -Wall -DANDROID -DUSE_VORBISFILE -DUSE_VORBIS -Wno-absolute-value -fvisibility=hidden
LOCAL_LDLIBS := -L$(LOCAL_PATH)/$(ANDROID_DIR)/$(TARGET_ARCH_ABI)/lib -llog -lGLESv1_CM -lGLESv2 -lvorbisfile -lvorbis -logg -lzstatic

include $(BUILD_SHARED_LIBRARY)
