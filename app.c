#include <cpunes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL3/SDL.h>
#include <GLES3/gl3.h>

SDL_GLContext ctx;
SDL_Window *win;

int main (int argc, char **argv)
{
	SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);

	SDL_GL_SetAttribute (SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute (SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute (SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute (SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute (SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute (SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute (SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

	SDL_DisableScreenSaver ();

	uint32_t scale = 8;

	uint32_t width = 256 * scale;
	uint32_t height = 224 * scale;

	uint32_t flags = SDL_WINDOW_OPENGL;

	win = SDL_CreateWindow ("Test NES",
			width,
			height,
			flags);

	ctx = SDL_GL_CreateContext (win);

	SDL_GL_SetSwapInterval (1);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport (0, 0, width, height);

	FILE *fp = fopen ("mario.nes", "r");

	uint64_t sz_file = 0L;

	fseek (fp, 0, SEEK_END);

	sz_file = ftell (fp);

	fseek (fp, 0, SEEK_SET);

	uint8_t *data = malloc (sz_file);

	fread (data, 1, sz_file, fp);
	
	fclose (fp);

	struct NESEmu *emu = malloc (sizeof (struct NESEmu));

	nes_emu_init (emu, data, sz_file);
	//nes_emu_rescale (emu, scale);

	SDL_Event event;

	while (1) {
		while (SDL_PollEvent (&event)) {
			switch (event.type) {
				case SDL_EVENT_QUIT:
					exit (0);
					break;
			}
		}

		nes_emu_execute (emu, 300, win);
	}
}
