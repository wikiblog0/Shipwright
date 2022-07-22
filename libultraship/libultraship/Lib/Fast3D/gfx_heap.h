// adapted from https://github.com/devkitPro/wut/blob/master/libraries/libwhb/src/gfx_heap.h

#pragma once
#include <wut.h>

#ifdef __cplusplus
extern "C" {
#endif

BOOL
_GfxHeapInitMEM1();

BOOL
_GfxHeapDestroyMEM1();

BOOL
_GfxHeapInitForeground();

BOOL
_GfxHeapDestroyForeground();

void *
_GfxHeapAllocMEM1(uint32_t size,
                 uint32_t alignment);

void
_GfxHeapFreeMEM1(void *block);

void *
_GfxHeapAllocForeground(uint32_t size,
                       uint32_t alignment);

void
_GfxHeapFreeForeground(void *block);

#ifdef __cplusplus
}
#endif
