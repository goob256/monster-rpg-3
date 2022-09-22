#ifndef NOO_ACHIEVEMENTS_H
#define NOO_ACHIEVEMENTS_H

#include "Nooskewl_Shim/main.h"

namespace noo {

namespace util {

bool NOOSKEWL_SHIM_EXPORT achieve(void *id);
bool NOOSKEWL_SHIM_EXPORT show_achievements();

// FIXME: stuff for setting up achievements (store system-specific ids/etc)

} // End namespace util

} // End namespace noo

#endif // NOO_ACHIEVEMENTS_H
