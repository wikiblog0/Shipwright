#ifdef _DEBUG
#include <coreinit/time.h>
#include <stdio.h>

/*
extern "C" {
    void start_time();
    void end_time(const char* name);
    void end_time_microssec(const char* name);
    void start_time_segment(int idx);
    void end_time_segment(int idx);
    void dump_time_segment(int idx, const char* name);
    void dump_time_segment_ids(void);
}
*/

static OSTick tick;

void start_time()
{
    tick = OSGetSystemTick();
}

void end_time(const char* name)
{
    uint32_t ms = OSTicksToMilliseconds(OSGetSystemTick() - tick);
    printf("%s: %dms\n", name, ms);
}

void end_time_microssec(const char* name)
{
    uint32_t ms = OSTicksToMicroseconds(OSGetSystemTick() - tick);
    printf("%s: %dµs\n", name, ms);
}

#define NUM_TIMES 0x10000

struct DebugTime {
    OSTick time;
    OSTick total;
    BOOL used;
};

static struct DebugTime times[NUM_TIMES] = { 0 };
static struct DebugTime totalTimes[NUM_TIMES] = { 0 };

void start_time_segment(int idx)
{
    times[idx].used = true;
    times[idx].time = OSGetSystemTick();
}

void end_time_segment(int idx)
{
    times[idx].total += (OSGetSystemTick() - times[idx].time); 
}

void dump_time_segment(int idx, const char* name)
{
    uint32_t ms = OSTicksToMilliseconds(times[idx].total);
    printf("%s: %dms\n", name, ms);

    times[idx].total = 0;
}

void dump_time_segment_ids(void)
{
    for (int i = 0; i < NUM_TIMES; i++) {
        uint32_t ms = OSTicksToMilliseconds(times[i].total);
        if (ms != 0) {
            printf("%x: %dms\n", i, ms);
        }
        times[i].total = 0;
    }
}

uint32_t get_wiiu_time_ms(void)
{
    return OSTicksToMilliseconds(OSGetSystemTick());
}

#endif
