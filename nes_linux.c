#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <cpunes.h>
#include <GLES3/gl3.h>

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

void linux_clear_screen (struct NESEmu *emu, void *_other_data)
{
	float r, g, b;
	nes_get_colors_background_clear (emu, &r, &g, &b);

	glClearColor (r, g, b, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT);
}

void linux_calc_time_uint64 (struct NESEmu *emu, void *_other_data)
{
    struct timeval tv;
    gettimeofday (&tv, NULL);

    uint64_t ns = (tv.tv_sec * 1000000) + tv.tv_usec;

    emu->last_cycles_int64 -= ns;
}

uint32_t linux_calc_time_nmi (struct NESEmu *emu, void *_other_data)
{
    struct timeval tv;
    gettimeofday (&tv, NULL);

    uint64_t ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

    if (emu->start_time_nmi == 0L) {
            emu->start_time_nmi = ms;
            return 0;
    }

    uint64_t diff_time = ms - emu->start_time_nmi;

    if (diff_time >= 16) {
        emu->start_time_nmi = ms;
	return 1;
    }

    return 0;
}

void linux_print_debug (struct NESEmu *emu, void *_other_data)
{
	printf ("%s\n", emu->line);
}

void linux_init_callbacks (struct NESCallbacks *cb)
{
	cb->print_debug = linux_print_debug;
	cb->calc_time_uint64 = linux_calc_time_uint64;
	cb->ppu_mask = linux_clear_screen;
	cb->calc_nmi = linux_calc_time_nmi;
}
