#ifndef NOO_AUDIO_H
#define NOO_AUDIO_H

#include "Nooskewl_Shim/main.h"

namespace noo {

namespace audio {

void NOOSKEWL_SHIM_EXPORT static_start();
bool NOOSKEWL_SHIM_EXPORT start();
void NOOSKEWL_SHIM_EXPORT end();
int NOOSKEWL_SHIM_EXPORT millis_to_samples(int millis);
int NOOSKEWL_SHIM_EXPORT samples_to_millis(int samples, int freq = -1);
void NOOSKEWL_SHIM_EXPORT pause_sfx(bool paused);

} // End namespace audio

} // End namespace noo

#endif // NOO_AUDIO_H
