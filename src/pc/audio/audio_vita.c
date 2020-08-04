#ifdef TARGET_VITA

#include <stdbool.h>
#include <vitasdk.h>

#include "audio_api.h"

#define AUDIO_SIZE 1024
#define AUDIO_RATE 32000

static SceRtcTick initial_tick;
int chan = -1;

static bool audio_vita_init(void) {
    const int vol[] = { 32767, 32767 };
    chan = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, AUDIO_SIZE, AUDIO_RATE, SCE_AUDIO_OUT_MODE_STEREO);
    //sceAudioOutSetConfig(chan, -1, -1, -1); // Remove this line once audio is implemented.
    sceAudioOutSetVolume(chan, SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH, vol);
    return true;
}

static int audio_vita_buffered(void) {
    return AUDIO_SIZE - sceAudioOutGetRestSample(chan);
}

static int audio_vita_get_desired_buffered(void) {
    return AUDIO_SIZE;
}

static void audio_vita_play(const uint8_t *buf, size_t len) {
    sceAudioOutOutput(chan, buf);
}

static void audio_vita_shutdown(void) {
    sceAudioOutReleasePort(chan);
    chan = -1;
}

struct AudioAPI audio_vita = {
    audio_vita_init,
    audio_vita_buffered,
    audio_vita_get_desired_buffered,
    audio_vita_play
};

#endif // TARGET_VITA
