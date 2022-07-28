#ifndef GFX_GX2_H
#define GFX_GX2_H

#include "gfx_rendering_api.h"

void gfx_gx2_shutdown(void);

void* gfx_gx2_texture_for_imgui(uint32_t texture_id);

extern struct GfxRenderingAPI gfx_gx2_api;

#endif
