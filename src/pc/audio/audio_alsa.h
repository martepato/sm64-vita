#ifndef AUDIO_ALSA_H
#define AUDIO_ALSA_H

#include "../compat.h"


#if defined(TARGET_VITA)
    #define HAVE_ALSA 0
#elif defined(__linux__) || defined(__BSD__)
    extern struct AudioAPI audio_alsa;
    #define HAVE_ALSA 1
#else
    #define HAVE_ALSA 0
#endif

#endif
