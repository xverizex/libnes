#include <cpunes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL3/SDL.h>
#include <glad/gl.h>
#include <controller.h>

static SDL_GLContext ctx;
static SDL_Window *win;

int main (int argc, char **argv)
{
	if (argc < 3) {
		fprintf (stderr, "./app [rom file] [scale]\n");
		exit (EXIT_FAILURE);
	}

	SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK);

	SDL_GL_SetAttribute (SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute (SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute (SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute (SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute (SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute (SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute (SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

	SDL_DisableScreenSaver ();

	uint8_t *data = NULL;
	uint64_t sz_file = 0;

	nes_get_rom (argv[1], &data, &sz_file);

	struct NESEmu *emu = malloc (sizeof (struct NESEmu));

	nes_emu_init (emu, data, sz_file);

	uint32_t scale = atoi (argv[2]);

	uint32_t width = emu->width * scale;
	uint32_t height = emu->height * scale;
	emu->scale = scale;

	uint32_t flags = SDL_WINDOW_OPENGL;

	win = SDL_CreateWindow ("Test NES",
			width,
			height,
			flags);

	ctx = SDL_GL_CreateContext (win);

	gladLoadGL ((GLADloadfunc) SDL_GL_GetProcAddress);

	SDL_GL_SetSwapInterval (1);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	nes_platform_init (emu, NULL);

	nes_init_surface (emu);

	SDL_Event event;

	while (1) {
		uint32_t is_written = 0;
		while (SDL_PollEvent (&event)) {
			nes_event (emu, &event);
		}

		nes_emu_execute (emu, 300, win);
	}

}
