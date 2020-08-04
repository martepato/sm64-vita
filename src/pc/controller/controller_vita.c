#if defined(TARGET_VITA)

#include <psp2/ctrl.h>

#include <ultra64.h>

#include "controller_api.h"

#define DEADZONE 4960

static void vita_init(void) {
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
}

static void vita_read(OSContPad *pad) {
    SceCtrlData ctrl;
    sceCtrlPeekBufferPositive(0, &ctrl, 1);

    if (ctrl.buttons & SCE_CTRL_START)
        pad->button |= START_BUTTON;
    if (ctrl.buttons & SCE_CTRL_LTRIGGER)
        pad->button |= Z_TRIG;
    if (ctrl.buttons & SCE_CTRL_RTRIGGER)
        pad->button |= R_TRIG;
    if (ctrl.buttons & SCE_CTRL_CROSS)
        pad->button |= A_BUTTON;
    if (ctrl.buttons & SCE_CTRL_CIRCLE)
        pad->button |= B_BUTTON;

    pad->stick_x = (s8)((s32) ctrl.lx - 128);
    pad->stick_y = (s8)(-((s32) ctrl.ly - 127));

    if (ctrl.rx < 60)
        pad->button |= L_CBUTTONS;
    if (ctrl.rx > 180)
        pad->button |= R_CBUTTONS;
    if (ctrl.ry < 60)
        pad->button |= U_CBUTTONS;
    if (ctrl.ry > 180)
        pad->button |= D_CBUTTONS;
}

struct ControllerAPI controller_vita = {
    vita_init,
    vita_read
};

#endif
