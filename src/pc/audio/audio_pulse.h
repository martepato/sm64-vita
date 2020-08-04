#ifndef AUDIO_PULSE_H
#define AUDIO_PULSE_H

#include "../compat.h"


#if defined(TARGET_VITA)
    #define HAVE_PULSE_AUDIO 0
#elif defined(__linux__) || defined(__BSD__)
    extern struct AudioAPI audio_pulse;
    #define HAVE_PULSE_AUDIO 1
#else
    #define HAVE_PULSE_AUDIO 0
#endif

#endif
