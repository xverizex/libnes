#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <cpunes.h>

void linux_wait_cycles (struct NESEmu *emu)
{
#if 0
    struct timeval tv;
    gettimeofday (&tv, NULL);

    uint64_t ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

    if (game->start_time == 0L) {
            game->start_time = ms;
            return;
    }

    uint64_t diff_time = ms - game->start_time;

    game->ftime = (float) diff_time / 10.f;
    if (game->ftime >= 20.f) {
        game->start_time = ms;
    }
#endif
}

void linux_calc_time_uint64 (struct NESEmu *emu, void *_other_data)
{
    struct timeval tv;
    gettimeofday (&tv, NULL);

    uint64_t ns = (tv.tv_sec * 1000000) + tv.tv_usec;

    emu->last_cycles_int64 -= ns;
}

void linux_print_debug (struct NESEmu *emu, void *_other_data)
{
	printf ("%s\n", emu->line);
}

void linux_init_callbacks (struct NESCallbacks *cb)
{
	cb->print_debug = linux_print_debug;
	cb->calc_time_uint64 = linux_calc_time_uint64;
}
