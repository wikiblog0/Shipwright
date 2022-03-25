#ifdef __WIIU__

#include <stdio.h>
#include <time.h>

#include <coreinit/time.h>
#include <coreinit/thread.h>
#include <coreinit/foreground.h>

#include <whb/gfx.h>
#include <gx2/display.h>
#include <gx2/event.h>
#include <gx2/swap.h>

#include <proc_ui/procui.h>

#include "gfx_window_manager_api.h"
#include "gfx_gx2.h"

static bool is_running;

static uint64_t previous_time;
static int frameDivisor = 1;

uint32_t window_height = 0;

#define FRAME_INTERVAL_US_NUMERATOR_ 50000
#define FRAME_INTERVAL_US_DENOMINATOR 3
#define FRAME_INTERVAL_US_NUMERATOR (FRAME_INTERVAL_US_NUMERATOR_ * frameDivisor)

static void gfx_wiiu_window_proc_ui_save_callback(void) {
    OSSavesDone_ReadyToRelease();
}

static uint32_t gfx_wiiu_window_proc_ui_exit_callback(void* data) {
    is_running = false;

    WHBGfxShutdown();

    return 0;
}

static void gfx_wiiu_init(const char *game_name, bool start_in_fullscreen) {
    ProcUIInit(&gfx_wiiu_window_proc_ui_save_callback);
    ProcUIRegisterCallback(PROCUI_CALLBACK_EXIT, &gfx_wiiu_window_proc_ui_exit_callback, NULL, 0);

    is_running = true;

    WHBGfxInit();
}

static void gfx_wiiu_set_fullscreen_changed_callback(void (*on_fullscreen_changed)(bool is_now_fullscreen)) {

}

static void gfx_wiiu_set_fullscreen(bool enable) {

}

static void gfx_wiiu_show_cursor(bool hide) {

}

static void gfx_wiiu_set_keyboard_callbacks(bool (*on_key_down)(int scancode), bool (*on_key_up)(int scancode), void (*on_all_keys_up)(void)) {

}

static bool whb_window_is_running(void) {
    if (!is_running) {
        return false;
    }

    ProcUIStatus status = ProcUIProcessMessages(true);

    switch (status) {
        case PROCUI_STATUS_EXITING:
            ProcUIShutdown();
            is_running = false;
            return false;
        case PROCUI_STATUS_RELEASE_FOREGROUND:
            ProcUIDrawDoneRelease();
            break;
        case PROCUI_STATUS_IN_BACKGROUND:
        case PROCUI_STATUS_IN_FOREGROUND:
            break;
    }
    return true;
}

static void gfx_wiiu_main_loop(void (*run_one_game_iter)(void)) {
    while (whb_window_is_running()) {
        run_one_game_iter();
    }
}

static void gfx_wiiu_get_dimensions(uint32_t *width, uint32_t *height) {
    switch (GX2GetSystemTVScanMode()) {
        case GX2_TV_SCAN_MODE_480I:
        case GX2_TV_SCAN_MODE_480P:
            *width = 854;
            *height = 480; window_height = 480;
            break;
        case GX2_TV_SCAN_MODE_1080I:
        case GX2_TV_SCAN_MODE_1080P:
            *width = 1920;
            *height = 1080; window_height = 1080;
            break;
        case GX2_TV_SCAN_MODE_720P:
        default:
            *width = 1280;
            *height = 720; window_height = 720;
            break;
    }
}

static void gfx_wiiu_handle_events(void) {

}

static bool gfx_wiiu_start_frame(void) {
    if (!is_running) {
        return false;
    } else {
        GX2WaitForFlip();
        return true;
    }
}

static uint64_t qpc_to_100ns(uint64_t qpc) {
    const uint64_t qpc_freq = OSSecondsToTicks(1);
    return qpc / qpc_freq * 10000000 + qpc % qpc_freq * 10000000 / qpc_freq;
}

static inline void sync_framerate_with_timer(void) {
    uint64_t t;
    t = OSGetSystemTime();

    const int64_t next = qpc_to_100ns(previous_time) + 10 * FRAME_INTERVAL_US_NUMERATOR / FRAME_INTERVAL_US_DENOMINATOR;
    const int64_t left = next - qpc_to_100ns(t);
    if (left > 0) {
        OSSleepTicks(OSNanosecondsToTicks(left * 100));
    }

    t = OSGetSystemTime();
    previous_time = t;
}

static void gfx_wiiu_swap_buffers_begin(void) {
    sync_framerate_with_timer();
    WHBGfxFinishRender();

    gfx_gx2_free_vbos();
}

static void gfx_wiiu_swap_buffers_end(void) {

}

static double gfx_wiiu_get_time(void) {
    return 0.0;
}

static void gfx_wiiu_set_framedivisor(int divisor)
{
    frameDivisor = divisor;
}

struct GfxWindowManagerAPI gfx_wiiu = {
    gfx_wiiu_init,
    gfx_wiiu_set_keyboard_callbacks,
    gfx_wiiu_set_fullscreen_changed_callback,
    gfx_wiiu_set_fullscreen,
    gfx_wiiu_show_cursor,
    gfx_wiiu_main_loop,
    gfx_wiiu_get_dimensions,
    gfx_wiiu_handle_events,
    gfx_wiiu_start_frame,
    gfx_wiiu_swap_buffers_begin,
    gfx_wiiu_swap_buffers_end,
    gfx_wiiu_get_time,
    gfx_wiiu_set_framedivisor
};

#endif
