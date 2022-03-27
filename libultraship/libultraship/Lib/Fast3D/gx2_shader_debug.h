#pragma once

#include <stdint.h>
#include "gfx_cc.h"
#include "gx2_shader_gen.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GFDHeader GFDHeader;
typedef struct GFDBlockHeader GFDBlockHeader;
typedef struct GFDRelocationHeader GFDRelocationHeader;

#define GFD_HEADER_MAGIC (0x47667832)
#define GFD_BLOCK_HEADER_MAGIC (0x424C4B7B)
#define GFD_RELOCATION_HEADER_MAGIC (0x7D424C4B)

#define GFD_FILE_VERSION_MAJOR (7)
#define GFD_FILE_VERSION_MINOR (1)
#define GFD_BLOCK_VERSION_MAJOR (1)

#define GFD_PATCH_MASK (0xFFF00000)
#define GFD_PATCH_DATA (0xD0600000)
#define GFD_PATCH_TEXT (0xCA700000)

typedef enum GFDBlockType
{
   GFD_BLOCK_END_OF_FILE                  = 1,
   GFD_BLOCK_PADDING                      = 2,
   GFD_BLOCK_VERTEX_SHADER_HEADER         = 3,
   GFD_BLOCK_VERTEX_SHADER_PROGRAM        = 5,
   GFD_BLOCK_PIXEL_SHADER_HEADER          = 6,
   GFD_BLOCK_PIXEL_SHADER_PROGRAM         = 7,
   GFD_BLOCK_GEOMETRY_SHADER_HEADER       = 8,
   GFD_BLOCK_GEOMETRY_SHADER_PROGRAM      = 9,
   GFD_BLOCK_GEOMETRY_SHADER_COPY_PROGRAM = 10,
   GFD_BLOCK_TEXTURE_HEADER               = 11,
   GFD_BLOCK_TEXTURE_IMAGE                = 12,
   GFD_BLOCK_TEXTURE_MIPMAP               = 13,
   GFD_BLOCK_COMPUTE_SHADER_HEADER        = 14,
   GFD_BLOCK_COMPUTE_SHADER_PROGRAM       = 15,
} GFDBlockType;

struct GFDHeader
{
   uint32_t magic;
   uint32_t headerSize;
   uint32_t majorVersion;
   uint32_t minorVersion;
   uint32_t gpuVersion;
   uint32_t align;
   uint32_t unk1;
   uint32_t unk2;
};
WUT_CHECK_OFFSET(GFDHeader, 0x00, magic);
WUT_CHECK_OFFSET(GFDHeader, 0x04, headerSize);
WUT_CHECK_OFFSET(GFDHeader, 0x08, majorVersion);
WUT_CHECK_OFFSET(GFDHeader, 0x0C, minorVersion);
WUT_CHECK_OFFSET(GFDHeader, 0x10, gpuVersion);
WUT_CHECK_OFFSET(GFDHeader, 0x14, align);
WUT_CHECK_OFFSET(GFDHeader, 0x18, unk1);
WUT_CHECK_OFFSET(GFDHeader, 0x1C, unk2);
WUT_CHECK_SIZE(GFDHeader, 0x20);

struct GFDBlockHeader
{
   uint32_t magic;
   uint32_t headerSize;
   uint32_t majorVersion;
   uint32_t minorVersion;
   GFDBlockType type;
   uint32_t dataSize;
   uint32_t id;
   uint32_t index;
};
WUT_CHECK_OFFSET(GFDBlockHeader, 0x00, magic);
WUT_CHECK_OFFSET(GFDBlockHeader, 0x04, headerSize);
WUT_CHECK_OFFSET(GFDBlockHeader, 0x08, majorVersion);
WUT_CHECK_OFFSET(GFDBlockHeader, 0x0C, minorVersion);
WUT_CHECK_OFFSET(GFDBlockHeader, 0x10, type);
WUT_CHECK_OFFSET(GFDBlockHeader, 0x14, dataSize);
WUT_CHECK_OFFSET(GFDBlockHeader, 0x18, id);
WUT_CHECK_OFFSET(GFDBlockHeader, 0x1C, index);
WUT_CHECK_SIZE(GFDHeader, 0x20);

struct GFDRelocationHeader
{
   uint32_t magic;
   uint32_t headerSize;
   uint32_t unk1;
   uint32_t dataSize;
   uint32_t dataOffset;
   uint32_t textSize;
   uint32_t textOffset;
   uint32_t patchBase;
   uint32_t patchCount;
   uint32_t patchOffset;
};
WUT_CHECK_OFFSET(GFDRelocationHeader, 0x00, magic);
WUT_CHECK_OFFSET(GFDRelocationHeader, 0x04, headerSize);
WUT_CHECK_OFFSET(GFDRelocationHeader, 0x08, unk1);
WUT_CHECK_OFFSET(GFDRelocationHeader, 0x0C, dataSize);
WUT_CHECK_OFFSET(GFDRelocationHeader, 0x10, dataOffset);
WUT_CHECK_OFFSET(GFDRelocationHeader, 0x14, textSize);
WUT_CHECK_OFFSET(GFDRelocationHeader, 0x18, textOffset);
WUT_CHECK_OFFSET(GFDRelocationHeader, 0x1C, patchBase);
WUT_CHECK_OFFSET(GFDRelocationHeader, 0x20, patchCount);
WUT_CHECK_OFFSET(GFDRelocationHeader, 0x24, patchOffset);
WUT_CHECK_SIZE(GFDRelocationHeader, 0x28);

int dumpGfdFile(uint64_t shader_id0, uint32_t shader_id1, struct ShaderGroup *group);

int dumpShaderFeatures(uint64_t shader_id0, uint32_t shader_id1, struct CCFeatures *cc_features);

#ifdef __cplusplus
}
#endif
