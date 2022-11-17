project(MonsterRPG3)
cmake_minimum_required(VERSION 2.6)

# Options

option(DEBUG "Build for debugging" OFF)
option(USE_STATIC_LIBS "Link with static libraries" ON)
option(RASPBERRYPI "Build for Raspberry Pi" OFF)
option(RASPBERRYPI_NORMAL "Build for Raspberry Pi (standard)" OFF)
option(USE_D3DX "Use D3DX to compiler shaders online" OFF)
option(WANT_TTF "Use TTFs" OFF)
option(STEAMWORKS "Build for Steam" off)
option(MAS "Build for Mac App Store" off)

set(USER_INCLUDE_PATH "" CACHE FILEPATH "Any extra include directories you want")
set(USER_LIBRARY_PATH "" CACHE FILEPATH "Any extra library directories you want")

include_directories("include")
include_directories(${USER_INCLUDE_PATH})
link_directories(${USER_LIBRARY_PATH})

# Inputs

set(SOURCES
	src/area_game.cpp
	src/audio_settings.cpp
	src/autosave.cpp
	src/battle_game.cpp
	src/battle_player.cpp
	src/battle_transition_in.cpp
	src/battle_transition_in2.cpp
	src/battle_transition_out.cpp
	src/battle_transition_out2.cpp
	src/battles.cpp
	src/bolt.cpp
	src/buy_scroll.cpp
	src/captain.cpp
	src/controls_settings.cpp
	src/cure.cpp
	src/custom_slide_entity.cpp
	src/dialogue.cpp
	src/enemies.cpp
	src/enemy_drawing_hook.cpp
	src/fire.cpp
	src/fishing.cpp
	src/fishing_prize.cpp
	src/general.cpp
	src/globals.cpp
	src/gui.cpp
	src/gui_drawing_hook.cpp
	src/heal.cpp
	src/hit.cpp
	src/ice.cpp
	src/inn.cpp
	src/inventory.cpp
	src/jump.cpp
	src/language_settings.cpp
	src/menu.cpp
	src/menu_game.cpp
	src/monster_rpg_3.cpp
	src/other_settings.cpp
	src/play_animation_and_delete.cpp
	src/question.cpp
	src/ran_away.cpp
	src/revive_entity.cpp
	src/sailor.cpp
	src/sailor_npc.cpp
	src/sailship.cpp
	src/save_slot.cpp
	src/scroll_help.cpp
	src/settings.cpp
	src/sliding_menu.cpp
	src/shop.cpp
	src/shop_game.cpp
	src/spells.cpp
	src/start_battle.cpp
	src/transition.cpp
	src/vampires.cpp
	src/vbolt.cpp
	src/vfire.cpp
	src/vice.cpp
	src/video_settings.cpp
	src/widgets.cpp
)

if(WIN32)
	list(APPEND SOURCES origdata/icons/windows/windows_icons.rc Monster_RPG_3.manifest versioninfo.rc)
endif()

if(APPLE)
	list(APPEND SOURCES src/apple.M)
endif()

# Outputs

add_executable(MonsterRPG3 ${SOURCES})

if(USE_STATIC_LIBS)
	set(WEDGE_LIB Nooskewl_Wedge_static)
	set(SHIM_LIB Nooskewl_Shim_static)
	set(TGUI3_LIB tgui3_static)
else()
	set(WEDGE_LIB Nooskewl_Wedge)
	set(SHIM_LIB Nooskewl_Shim)
	set(TGUI3_LIB tgui3)
endif()

# Platform specifics

if(WIN32)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /IGNORE:4075 /IGNORE:4099")
	SET(FLAGS "${FLAGS} /D_CRT_SECURE_NO_WARNINGS /D_USE_MATH_DEFINES /EHsc /MP /W3 /wd4244 /wd4800 /wd4101 /MT")
	SET(FLAGS "${FLAGS} /DNTDDI_VERSION=0x06010000 /D_WIN32_WINNT=0x601")
	if(DEBUG)
		set(FLAGS "${FLAGS} /DDEBUG")
	endif()
	if(USE_STATIC_LIBS)
		set(FLAGS "${FLAGS} /DTGUI3_STATIC /DNOOSKEWL_SHIM_STATIC /DNOOSKEWL_WEDGE_STATIC")
	endif()
	if(USE_D3DX)
		set(FLAGS "${FLAGS} /DUSE_D3DX")
	endif()
	if(STEAMWORKS)
		set(FLAGS "${FLAGS} /DSTEAMWORKS")
	endif()
	if(WANT_TTF)
		set(FLAGS "${FLAGS} /DUSE_TTF")
	endif()
	set_target_properties(MonsterRPG3 PROPERTIES COMPILE_FLAGS "${FLAGS}")
	SET(LIBS ${LIBS} ${WEDGE_LIB} ${SHIM_LIB} ${TGUI3_LIB} SDL2 SDL2main libvorbisfile_static libvorbis_static ogg d3d9 winmm dbghelp)
	if(USE_D3DX)
		set(LIBS ${LIBS} d3dx9)
	endif()
	if(WANT_TTF)
		set(LIBS ${LIBS} freetype zlibstatic)
	else()
		set(LIBS ${LIBS} zlibstatic)
	endif()
	if(STEAMWORKS)
		set(LIBS ${LIBS} steam_api)
	endif()
	set_target_properties(MonsterRPG3 PROPERTIES OUTPUT_NAME "Monster RPG 3")
elseif(APPLE)
	set(FLAGS "${FLAGS} -Wall -Wno-extern-c-compat -Wno-c++11-narrowing -Wno-overloaded-virtual -DTGUI3_STATIC -DNOOSKEWL_SHIM_STATIC -DNOOSKEWL_WEDGE_STATIC")
	set(FLAGS "${FLAGS} -O2 -fvisibility=hidden -fno-objc-arc")
	if(STEAMWORKS)
		set(FLAGS "${FLAGS} -DSTEAMWORKS")
	endif()
	if(MAS)
		set(FLAGS "${FLAGS} -DMAS -stdlib=libc++ -std=c++11")
	endif()
	if(DEBUG)
		set(FLAGS "${FLAGS} -DDEBUG -g")
	endif()
	if(WANT_TTF)
		set(FLAGS "${FLAGS} -DUSE_TTF")
	endif()
	set_target_properties(MonsterRPG3 PROPERTIES BUILD_WITH_INSTALL_RPATH on)
	set_target_properties(MonsterRPG3 PROPERTIES INSTALL_RPATH "@executable_path/")
	set_target_properties(MonsterRPG3 PROPERTIES COMPILE_FLAGS "${FLAGS}")
	set_target_properties(MonsterRPG3 PROPERTIES LINK_FLAGS "-framework Cocoa -framework CoreAudio -framework AudioToolbox -framework CoreVideo -framework ForceFeedback -framework IOKit -framework Carbon")
	set(LIBS ${WEDGE_LIB} ${SHIM_LIB} ${TGUI3_LIB} SDL2 vorbisfile vorbis ogg c++)
	if(WANT_TTF)
		set(LIBS ${LIBS} freetype z)
	else()
		set(LIBS ${LIBS} z)
	endif()
	if(STEAMWORKS)
		set(LIBS ${LIBS} steam_api)
	endif()
else() # Linux
	SET(FLAGS "${FLAGS} -fvisibility=hidden -Wall -Wno-psabi -Wno-strict-aliasing -Wno-comment -std=c++0x")
	if(STEAMWORKS)
		set(FLAGS "${FLAGS} -DSTEAMWORKS")
	endif()
	if(DEBUG)
		set(FLAGS "${FLAGS} -DDEBUG -g -fsanitize=address -fsanitize=leak")
	endif()
	IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND NOT USE_STATIC_LIBS)
		set(FLAGS "${FLAGS} -fPIC")
	ENDIF()
	if (RASPBERRYPI)
		set(FLAGS "${FLAGS} -DRASPBERRYPI=1")
		set(FLAGS "${FLAGS} -DRASPBERRYPI=1")
	endif()
	if (RASPBERRYPI_NORMAL)
		set(FLAGS "${FLAGS} -DRASPBERRYPI_NORMAL=1")
		set(FLAGS "${FLAGS} -DRASPBERRYPI_NORMAL=1")
	endif()
	if (NOT DEBUG AND (RASPBERRYPI OR RASPBERRYPI_NORMAL))
		set(CMAKE_C_FLAGS_RELEASE "-Os")
		set(CMAKE_CXX_FLAGS_RELEASE "-Os")
	endif()
	if (NOT RASPBERRYPI AND NOT RASPBERRYPI_NORMAL)
		set(CMAKE_C_FLAGS_RELEASE "-O2")
		set(CMAKE_CXX_FLAGS_RELEASE "-O2")
	endif()
	if(WANT_TTF)
		set(FLAGS "${FLAGS} -DUSE_TTF")
	endif()
	set(LIBS ${WEDGE_LIB} ${SHIM_LIB} ${TGUI3_LIB} vorbisfile vorbis ogg)
	if(RASPBERRYPI)
		LIST(APPEND LIBS /usr/local/lib/libSDL2.so /opt/vc/lib/libbrcmGLESv2.so)
		include_directories("/opt/vc/include")
	else()
		LIST(APPEND LIBS SDL2)
		if(USE_STATIC_LIBS)
			LIST(APPEND LIBS Xcursor X11)
		endif()
	endif()
	if(WANT_TTF)
		set(LIBS ${LIBS} freetype z)
	else()
		set(LIBS ${LIBS} z)
	endif()
	if(STEAMWORKS)
		set(LIBS ${LIBS} steam_api)
	endif()
	set_target_properties(MonsterRPG3 PROPERTIES COMPILE_FLAGS "${FLAGS}")
	if(DEBUG)
		set_target_properties(MonsterRPG3 PROPERTIES LINK_FLAGS "-Wl,-rpath,\$ORIGIN -fsanitize=address -fsanitize=leak")
	else()
		set_target_properties(MonsterRPG3 PROPERTIES LINK_FLAGS "-Wl,-rpath,\$ORIGIN")
	endif()
endif()

target_link_libraries(MonsterRPG3 ${LIBS})
