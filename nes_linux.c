#include <stdint.h>
#include <time.h>
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

void linux_print_debug (struct NESEmu *emu, void *_other_data)
{
	printf ("%s\n", emu->line);
}

void linux_init_callbacks (struct NESCallbacks *cb)
{
	cb->print_debug = linux_print_debug;
}
