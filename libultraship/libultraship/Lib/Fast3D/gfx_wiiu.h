#ifndef GFX_WIIU_H
#define GFX_WIIU_H

#include <vpad/input.h>
#include <padscore/kpad.h>

#include "gfx_window_manager_api.h"

// make the default fb always 1080p to not mess with scaling
#define WIIU_DEFAULT_FB_WIDTH  1920
#define WIIU_DEFAULT_FB_HEIGHT 1080

extern bool has_foreground;
extern uint32_t frametime;

void gfx_wiiu_set_context_state(void);

VPADStatus *gfx_wiiu_get_vpad_status(VPADReadError *error);

KPADStatus *gfx_wiiu_get_kpad_status(WPADChan chan, KPADError *error);

extern struct GfxWindowManagerAPI gfx_wiiu;

#endif
