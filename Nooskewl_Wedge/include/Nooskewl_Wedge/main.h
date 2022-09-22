#ifndef NOOSKEWL_WEDGE_MAIN_H
#define NOOSKEWL_WEDGE_MAIN_H

#include <Nooskewl_Shim/Nooskewl_Shim.h>

#ifdef _WIN32
#ifdef NOOSKEWL_WEDGE_STATIC
#define NOOSKEWL_WEDGE_EXPORT
#else
#ifdef NOOSKEWL_WEDGE_BUILD
#define NOOSKEWL_WEDGE_EXPORT __declspec(dllexport)
#else
#define NOOSKEWL_WEDGE_EXPORT __declspec(dllimport)
#endif
#endif
#else
#define NOOSKEWL_WEDGE_EXPORT
#endif

namespace wedge {

bool NOOSKEWL_WEDGE_EXPORT start(util::Size<int> base_screen_size);
bool NOOSKEWL_WEDGE_EXPORT go();
void NOOSKEWL_WEDGE_EXPORT end();
void NOOSKEWL_WEDGE_EXPORT handle_event(TGUI_Event *event);

}

#endif // NOOSKEWL_EDGE_MAIN_H
