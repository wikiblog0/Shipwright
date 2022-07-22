#ifndef GFX_WIIU_H
#define GFX_WIIU_H

#include "gfx_window_manager_api.h"

void gfx_wiiu_set_context_state(void);

extern bool has_foreground;

// make the default fb always 1080p to not mess with scaling
#define DEFAULT_FB_WIDTH  1920
#define DEFAULT_FB_HEIGHT 1080

extern struct GfxWindowManagerAPI gfx_wiiu;

#endif
