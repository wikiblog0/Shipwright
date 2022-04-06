#ifndef GFX_WIIU_H
#define GFX_WIIU_H

#include "gfx_window_manager_api.h"

void gfx_wiiu_set_context_state(void);

extern bool has_foreground;

extern uint32_t tv_width;
extern uint32_t tv_height;

extern struct GfxWindowManagerAPI gfx_wiiu;

#endif
