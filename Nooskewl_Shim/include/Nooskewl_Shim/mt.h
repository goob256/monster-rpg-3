#ifndef NOO_MT_H
#define NOO_MT_H

#include "Nooskewl_Shim/main.h"

using namespace noo;

namespace noo {

namespace util {

void NOOSKEWL_SHIM_EXPORT srand(uint32_t s);
uint32_t NOOSKEWL_SHIM_EXPORT rand();
uint32_t NOOSKEWL_SHIM_EXPORT rand(int min, int max_inclusive);

} // End namespace util

} // End namespace noo

#endif // NOO_MT_H
