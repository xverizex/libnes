#include <cpunes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL3/SDL.h>
#include <GLES3/gl3.h>
#include <controller.h>

SDL_GLContext ctx;
SDL_Window *win;

static SDL_Joystick *joy[2];
static int joystick_count = 0;

static void connect_joystick ()
{
	SDL_JoystickID *joystick_ids = SDL_GetJoysticks (&joystick_count);

	if (joystick_count == 0)
		return;

	for (int i = 0; i < joystick_count; i++) {
		SDL_JoystickID id = joystick_ids[i];

		joy[i] = SDL_OpenJoystick (id);
	}

}

static void close_all_joystick ()
{
	for (int i = 0; i < joystick_count; i++) {
		SDL_CloseJoystick (joy[i]);
	}
}

static void state_hat_buttons_get (uint8_t *state, uint8_t value)
{
	switch (value) {
		case SDL_HAT_CENTERED:
			(*state) &= 0x0f;
			break;
		case SDL_HAT_UP:
			(*state) |= (1 << JOY_UP);
			break;
		case SDL_HAT_RIGHT:
			(*state) |= (1 << JOY_RIGHT);
			break;
		case SDL_HAT_DOWN:
			(*state) |= (1 << JOY_DOWN);
			break;
		case SDL_HAT_LEFT:
			(*state) |= (1 << JOY_LEFT);
			break;
		case SDL_HAT_RIGHTUP:
			break;
		case SDL_HAT_RIGHTDOWN:
			break;
		case SDL_HAT_LEFTUP:
			break;
		case SDL_HAT_LEFTDOWN:
			break;
	}
}

enum {
	BUTTON_A = 0,
	BUTTON_B = 1,
	BUTTON_SELECT = 6,
	BUTTON_START = 7
};

static void state_button_get (uint8_t *state, uint8_t value, uint8_t is_down)
{
	switch (value) {
		case BUTTON_A:
			if (is_down)
				(*state) |= (1 << JOY_A);
			else
				(*state) &= 0xfe;
			break;
		case BUTTON_B:
			if (is_down)
				(*state) |= (1 << JOY_B);
			else
				(*state) &= 0xfd;
			break;
		case BUTTON_SELECT:
			if (is_down)
				(*state) |= (1 << JOY_SELECT);
			else
				(*state) &= 0xfb;
			break;
		case BUTTON_START:
			if (is_down)
				(*state) |= (1 << JOY_START);
			else
				(*state) &= 0xf7;
			break;
	}
}


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

	platform_get_rom (argv[1], &data, &sz_file);

	struct NESEmu *emu = malloc (sizeof (struct NESEmu));

	nes_emu_init (emu, data, sz_file);

	uint32_t scale = atoi (argv[2]);

	uint32_t width = emu->width * scale;
	uint32_t height = emu->height * scale;

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

	platform_init (emu, NULL);

	SDL_Event event;

	uint8_t state = 0;

	while (1) {
		uint32_t is_written = 0;
		while (SDL_PollEvent (&event)) {
			switch (event.type) {
				case SDL_EVENT_JOYSTICK_HAT_MOTION:
					//if (emu->new_state == 0) {
					if (emu->is_new_state) {
						uint8_t temp = emu->state_buttons0 & 0x0f;
						emu->state_buttons0 &= (state & 0xf0);
						if ((state & 0xf0) == 0)
							emu->state_buttons0 &= 0x0f;
						state_hat_buttons_get (&emu->state_buttons0, event.jhat.value);
						if ((emu->state_buttons0 & 0x80) && (temp & 0x40)) {
							emu->state_buttons0 &= ~(0x80);
							emu->state_buttons0 |= (temp);
						} else if ((emu->state_buttons0 & 0x40) && (temp & 0x80)) {
							emu->state_buttons0 &= ~(0x40);
							emu->state_buttons0 |= (temp);
						}
						if ((emu->state_buttons0 & 0x10) && (temp & 0x20)) {
							emu->state_buttons0 &= ~(0x10);
							emu->state_buttons0 |= (temp);
						} else if ((emu->state_buttons0 & 0x20) && (temp & 0x10)) {
							emu->state_buttons0 &= ~(0x20);
							emu->state_buttons0 |= (temp);
						}
						state = emu->state_buttons0;
						is_written = 1;
						emu->is_new_state = 0;
					} else {
						state_hat_buttons_get (&state, event.jhat.value);
					}
					break;
				case SDL_EVENT_JOYSTICK_BALL_MOTION:
					break;
				case SDL_EVENT_JOYSTICK_AXIS_MOTION:
					break;
				case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
					//if (emu->new_state == 0) {
					if (emu->is_new_state) {
						state_button_get (&emu->state_buttons0, event.jbutton.button, event.jbutton.down);
						emu->state_buttons0 |= state;
						is_written = 1;
						emu->is_new_state = 0;
					} else {
						state_button_get (&state, event.jbutton.button, event.jbutton.down);
					}
					break;
				case SDL_EVENT_JOYSTICK_BUTTON_UP:
					if (emu->is_new_state) {
					//if (emu->new_state == 0) {
						state_button_get (&emu->state_buttons0, event.jbutton.button, event.jbutton.down);
						emu->state_buttons0 |= state;
						is_written = 1;
						emu->is_new_state = 0;
					} else {
						state_button_get (&state, event.jbutton.button, event.jbutton.down);
					}
					break;
				case SDL_EVENT_JOYSTICK_ADDED:
					connect_joystick ();
					break;
				case SDL_EVENT_QUIT:
					close_all_joystick ();
					exit (0);
					break;
			}
		}
		if (is_written) {
			nes_write_state (emu);
		}

		nes_emu_execute (emu, 300, win);
	}

}
