#ifdef __WIIU__

#include "../../Window.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <malloc.h>

#include <map>

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif
#include "PR/ultra64/gbi.h"

#include "gfx_cc.h"
#include "gfx_rendering_api.h"
#include "../../SohImGuiImpl.h"
#include "../../Environment.h"
#include "../../GlobalCtx2.h"
#include "gfx_pc.h"
#include "gfx_wiiu.h"

#include <whb/gfx.h>
#include <gx2/draw.h>
#include <gx2/state.h>
#include <gx2/swap.h>
#include <gx2/event.h>
#include <gx2/utils.h>
#include <gx2/mem.h>
#include <gx2/registers.h>
#include <gx2r/surface.h>
#include "gx2_shader_gen.h"

struct ShaderProgram {
    struct ShaderGroup group;
    uint8_t num_inputs;
    bool used_textures[2];
    bool used_noise;
    uint32_t window_params_offset;
    uint32_t samplers_location[2];
};

struct Texture {
    GX2Texture texture;
    bool textureUploaded;

    GX2Sampler sampler;
    bool samplerSet;
};

static std::map<std::pair<uint64_t, uint32_t>, struct ShaderProgram> shader_program_pool;
static std::vector<float*> vbo_pool;

static uint32_t frame_count;
static uint32_t current_height;
static bool current_depth_mask;
static struct ShaderProgram *current_shader_program;
static struct Texture *current_texture;
static int current_tile;

static inline GX2SamplerVar *GX2GetPixelSamplerVar(const GX2PixelShader *shader, const char *name)
{
    for (uint32_t i = 0; i < shader->samplerVarCount; ++i) {
        if (strcmp(name, shader->samplerVars[i].name) == 0) {
            return &shader->samplerVars[i];
        }
    }

    return NULL;
}

static inline int32_t GX2GetPixelSamplerVarLocation(const GX2PixelShader *shader, const char *name)
{
    GX2SamplerVar *sampler = GX2GetPixelSamplerVar(shader, name);
    return sampler ? sampler->location : -1;
}

static inline int32_t GX2GetPixelUniformVarOffset(const GX2PixelShader *shader, const char *name)
{
    GX2UniformVar *uniform = GX2GetPixelUniformVar(shader, name);
    return uniform ? uniform->offset : -1;
}

static bool gfx_gx2_z_is_from_0_to_1(void) {
    return false;
}

static void gfx_gx2_set_uniforms(struct ShaderProgram *prg) {
    if (prg->used_noise) {
        float window_params_array[2] = { (float) current_height, (float) frame_count };

        GX2SetPixelUniformReg(prg->window_params_offset, 2, window_params_array);
    }
}

static void gfx_gx2_unload_shader(struct ShaderProgram *old_prg) {
    current_shader_program = nullptr;
}

static void gfx_gx2_load_shader(struct ShaderProgram *new_prg) {
    current_shader_program = new_prg;

    GX2SetFetchShader(&new_prg->group.fetchShader);
    GX2SetVertexShader(&new_prg->group.vertexShader);
    GX2SetPixelShader(&new_prg->group.pixelShader);

    gfx_gx2_set_uniforms(new_prg);
}

static struct ShaderProgram* gfx_gx2_create_and_load_new_shader(uint64_t shader_id0, uint32_t shader_id1) {
    struct CCFeatures cc_features;
    gfx_cc_get_features(shader_id0, shader_id1, &cc_features);

    struct ShaderProgram* prg = &shader_program_pool[std::make_pair(shader_id0, shader_id1)];

    printf("Generating shader: %016llx-%08x\n", shader_id0, shader_id1);
    if (gx2GenerateShaderGroup(&prg->group, &cc_features) != 0) {
        printf("Failed to generate shader\n");
        current_shader_program = NULL;
        return NULL;
    }

    prg->num_inputs = cc_features.num_inputs;
    prg->used_textures[0] = cc_features.used_textures[0];
    prg->used_textures[1] = cc_features.used_textures[1];

    gfx_gx2_load_shader(prg);

    prg->window_params_offset = GX2GetPixelUniformVarOffset(&prg->group.pixelShader, "window_params");
    prg->samplers_location[0] = GX2GetPixelSamplerVarLocation(&prg->group.pixelShader, "uTex0");
    prg->samplers_location[1] = GX2GetPixelSamplerVarLocation(&prg->group.pixelShader, "uTex1");

    prg->used_noise = cc_features.opt_alpha && cc_features.opt_noise;

    printf("Generated and loaded shader\n");

    return prg;
}

static struct ShaderProgram *gfx_gx2_lookup_shader(uint64_t shader_id0, uint32_t shader_id1) {
    auto it = shader_program_pool.find(std::make_pair(shader_id0, shader_id1));
    return it == shader_program_pool.end() ? nullptr : &it->second;
}

static void gfx_gx2_shader_get_info(struct ShaderProgram *prg, uint8_t *num_inputs, bool used_textures[2]) {
    *num_inputs = prg->num_inputs;
    used_textures[0] = prg->used_textures[0];
    used_textures[1] = prg->used_textures[1];
}

static uint32_t gfx_gx2_new_texture(void) {
    // some 32-bit trickery :P
    return (uint32_t) calloc(1, sizeof(struct Texture));
}

static void gfx_gx2_delete_texture(uint32_t texture_id) {
    struct Texture *tex = (struct Texture *) texture_id;

    if (tex->texture.surface.image) {
        GX2RDestroySurfaceEx(&tex->texture.surface, GX2R_RESOURCE_BIND_NONE);
    }

    free((void* ) tex);
}

static void gfx_gx2_select_texture(int tile, uint32_t texture_id) {
    struct Texture *tex = (struct Texture *) texture_id;
    current_texture = tex;
    current_tile = tile;

    if (current_shader_program) {
        uint32_t sampler_location = current_shader_program->samplers_location[tile];

        if (tex->textureUploaded) {
            GX2SetPixelTexture(&tex->texture, sampler_location);
        }

        if (tex->samplerSet) {
            GX2SetPixelSampler(&tex->sampler, sampler_location);
        }
    }
}

static void gfx_gx2_upload_texture(const uint8_t *rgba32_buf, uint32_t width, uint32_t height) {
    struct Texture *tex = current_texture;
    assert(tex);

    tex->texture.surface.use = GX2_SURFACE_USE_TEXTURE;
    tex->texture.surface.dim = GX2_SURFACE_DIM_TEXTURE_2D;
    tex->texture.surface.width = width;
    tex->texture.surface.height = height;
    tex->texture.surface.depth = 1;
    tex->texture.surface.mipLevels = 1;
    tex->texture.surface.format = GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;
    tex->texture.surface.aa = GX2_AA_MODE1X;
    tex->texture.surface.tileMode = GX2_TILE_MODE_LINEAR_ALIGNED;
    tex->texture.viewFirstMip = 0;
    tex->texture.viewNumMips = 1;
    tex->texture.viewFirstSlice = 0;
    tex->texture.viewNumSlices = 1;
    tex->texture.compMap = GX2_COMP_MAP(GX2_SQ_SEL_R, GX2_SQ_SEL_G, GX2_SQ_SEL_B, GX2_SQ_SEL_A);

    GX2RCreateSurface(&tex->texture.surface, GX2R_RESOURCE_BIND_TEXTURE | GX2R_RESOURCE_USAGE_CPU_WRITE | GX2R_RESOURCE_USAGE_GPU_READ);
    GX2InitTextureRegs(&tex->texture);

    uint8_t* buf = (uint8_t*) GX2RLockSurfaceEx(&tex->texture.surface, 0, GX2R_RESOURCE_BIND_NONE);

    for (uint32_t y = 0; y < height; y++) {
        memcpy(buf + (y * tex->texture.surface.pitch), buf + (y * width), width * 4);
    }

    GX2RUnlockSurfaceEx(&tex->texture.surface, 0, GX2R_RESOURCE_BIND_NONE);

    if (current_shader_program != NULL) {
        GX2SetPixelTexture(&tex->texture, current_shader_program->samplers_location[current_tile]);
    }

    tex->textureUploaded = true;
}

static GX2TexClampMode gfx_cm_to_gx2(uint32_t val) {
    switch (val) {
        case G_TX_NOMIRROR | G_TX_CLAMP:
            return GX2_TEX_CLAMP_MODE_CLAMP;
        case G_TX_MIRROR | G_TX_WRAP:
            return GX2_TEX_CLAMP_MODE_MIRROR;
        case G_TX_MIRROR | G_TX_CLAMP:
            return GX2_TEX_CLAMP_MODE_MIRROR_ONCE;
        case G_TX_NOMIRROR | G_TX_WRAP:
            return GX2_TEX_CLAMP_MODE_WRAP;
    }

    return GX2_TEX_CLAMP_MODE_WRAP;
}

static void gfx_gx2_set_sampler_parameters(int tile, bool linear_filter, uint32_t cms, uint32_t cmt) {
    struct Texture *tex = current_texture;
    assert(tex);

    current_tile = tile;

    GX2InitSampler(&tex->sampler, GX2_TEX_CLAMP_MODE_CLAMP, linear_filter ? GX2_TEX_XY_FILTER_MODE_LINEAR : GX2_TEX_XY_FILTER_MODE_POINT);
    GX2InitSamplerClamping(&tex->sampler, gfx_cm_to_gx2(cms), gfx_cm_to_gx2(cmt), GX2_TEX_CLAMP_MODE_WRAP);

    if (current_shader_program != NULL) {
        GX2SetPixelSampler(&tex->sampler, current_shader_program->samplers_location[tile]);
    }

    tex->samplerSet = true;
}

static void gfx_gx2_set_depth_test_and_mask(bool depth_test, bool z_upd) {
    current_depth_mask = z_upd;
    GX2SetDepthOnlyControl(depth_test, z_upd, GX2_COMPARE_FUNC_LEQUAL);
}

static void gfx_gx2_set_zmode_decal(bool zmode_decal) {
    if (zmode_decal) {
        GX2SetPolygonControl(GX2_FRONT_FACE_CCW, FALSE, FALSE, TRUE,
                             GX2_POLYGON_MODE_TRIANGLE, GX2_POLYGON_MODE_TRIANGLE,
                             TRUE, TRUE, FALSE);
        GX2SetPolygonOffset(-2.0f, -2.0f, -2.0f, -2.0f, 0.0f);
    } else {
        GX2SetPolygonControl(GX2_FRONT_FACE_CCW, FALSE, FALSE, FALSE,
                             GX2_POLYGON_MODE_TRIANGLE, GX2_POLYGON_MODE_TRIANGLE,
                             FALSE, FALSE, FALSE);
        GX2SetPolygonOffset(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    }
}

static void gfx_gx2_set_viewport(int x, int y, int width, int height) {
    //GX2SetViewport(x, window_height - y - height, width, height, 0.0f, 1.0f);
    current_height = height;
}

static void gfx_gx2_set_scissor(int x, int y, int width, int height) {
    //GX2SetScissor(x, window_height - y - height, width, height);
}

static void gfx_gx2_set_use_alpha(bool use_alpha) {
    if (use_alpha) {
        GX2SetBlendControl(GX2_RENDER_TARGET_0,
                           GX2_BLEND_MODE_SRC_ALPHA, GX2_BLEND_MODE_INV_SRC_ALPHA,
                           GX2_BLEND_COMBINE_MODE_ADD, FALSE,
                           GX2_BLEND_MODE_ZERO, GX2_BLEND_MODE_ZERO,
                           GX2_BLEND_COMBINE_MODE_ADD);
        GX2SetColorControl(GX2_LOGIC_OP_COPY, 1, FALSE, TRUE);
    } else {
        GX2SetBlendControl(GX2_RENDER_TARGET_0,
                           GX2_BLEND_MODE_ONE, GX2_BLEND_MODE_ZERO,
                           GX2_BLEND_COMBINE_MODE_ADD, FALSE,
                           GX2_BLEND_MODE_ZERO, GX2_BLEND_MODE_ZERO,
                           GX2_BLEND_COMBINE_MODE_ADD);
        GX2SetColorControl(GX2_LOGIC_OP_COPY, 0, FALSE, TRUE);
    }
}

static void gfx_gx2_draw_triangles(float buf_vbo[], size_t buf_vbo_len, size_t buf_vbo_num_tris) {
    if (current_shader_program == NULL) {
        return;
    }

    uint32_t idx = vbo_pool.size();
    vbo_pool.resize(idx + 1);

    size_t vbo_len = sizeof(float) * buf_vbo_len;
    vbo_pool[idx] = static_cast<float*>(memalign(GX2_VERTEX_BUFFER_ALIGNMENT, vbo_len));

    float* new_vbo = vbo_pool[idx];
    memcpy(new_vbo, buf_vbo, vbo_len);

    GX2Invalidate(GX2_INVALIDATE_MODE_CPU_ATTRIBUTE_BUFFER, new_vbo, vbo_len);

    GX2SetAttribBuffer(0, vbo_len, current_shader_program->group.stride, new_vbo);
    GX2DrawEx(GX2_PRIMITIVE_MODE_TRIANGLES, 3 * buf_vbo_num_tris, 0, 1);
}

void gfx_gx2_free_vbos(void) {
    for (uint32_t i = 0; i < vbo_pool.size(); i++) {
        free(vbo_pool[i]);
    }

    vbo_pool.clear();
}

static void gfx_gx2_init(void) {
}

static void gfx_gx2_on_resize(void) {
}

static void gfx_gx2_start_frame(void) {
    frame_count++;

    WHBGfxBeginRenderTV();
    WHBGfxClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

static void gfx_gx2_end_frame(void) {
    GX2Flush();
    GX2DrawDone();
    WHBGfxFinishRenderTV();
    GX2CopyColorBufferToScanBuffer(WHBGfxGetTVColourBuffer(), GX2_SCAN_TARGET_DRC);
}

static void gfx_gx2_finish_render(void) {
}

static int gfx_gx2_create_framebuffer(uint32_t width, uint32_t height) {
    return 0;
}

static void gfx_gx2_resize_framebuffer(int fb, uint32_t width, uint32_t height) {

}

void gfx_gx2_set_framebuffer(int fb) {

}

void gfx_gx2_reset_framebuffer(void) {

}

void gfx_gx2_select_texture_fb(int fbID) {

}

static uint16_t gfx_gx2_get_pixel_depth(float x, float y) {
    // TODO
    return 0;
}

struct GfxRenderingAPI gfx_gx2_api = {
    gfx_gx2_z_is_from_0_to_1,
    gfx_gx2_unload_shader,
    gfx_gx2_load_shader,
    gfx_gx2_create_and_load_new_shader,
    gfx_gx2_lookup_shader,
    gfx_gx2_shader_get_info,
    gfx_gx2_new_texture,
    gfx_gx2_select_texture,
    gfx_gx2_upload_texture,
    gfx_gx2_set_sampler_parameters,
    gfx_gx2_set_depth_test_and_mask,
    gfx_gx2_get_pixel_depth,
    gfx_gx2_set_zmode_decal,
    gfx_gx2_set_viewport,
    gfx_gx2_set_scissor,
    gfx_gx2_set_use_alpha,
    gfx_gx2_draw_triangles,
    gfx_gx2_init,
    gfx_gx2_on_resize,
    gfx_gx2_start_frame,
    gfx_gx2_end_frame,
    gfx_gx2_finish_render,
    gfx_gx2_create_framebuffer,
    gfx_gx2_resize_framebuffer,
    gfx_gx2_set_framebuffer,
    gfx_gx2_reset_framebuffer,
    gfx_gx2_select_texture_fb,
    gfx_gx2_delete_texture
};

#endif
