#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <cpunes.h>
#include <string.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <assert.h>
#include <controller.h>
#include "sdl3_opengl4.h"

struct render_opengl_data {
	uint32_t program;
	float ortho[16];
	float transform[16];
	float scale[16];
	float model[16];
	uint32_t count_bytes;
	uint32_t id_ortho;
	uint32_t id_transform;
	uint32_t id_scale;
	uint32_t id_model;
	uint32_t id_sampler;
	uint32_t texture;
	uint32_t background_texture[960 + 960];
	uint32_t sprite_texture[64];
	uint32_t vao;
	uint32_t vbo;
	uint8_t palette_image[16];
	uint32_t is_need_reorder_palette_background;
	uint32_t fbo;
	uint32_t tex_fbo;
	uint32_t rbo;
	uint32_t framebuffer_vao;
	uint32_t framebuffer_vbo;
};

struct sdl_data {
	SDL_Joystick *joy[2];
	int joystick_count;
};

uint32_t palette_get_color (struct NESEmu *emu, uint8_t idx);

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

void nes_copy_texture_surface (struct NESEmu *emu,
		uint32_t *tex)
{
	struct render_opengl_data *r = emu->_render_data;

	*tex = r->tex_fbo;
}

static void framebuffer_init (struct NESEmu *emu)
{
	struct render_opengl_data *r = emu->_render_data;

	uint32_t width = emu->width;
	uint32_t height = emu->height;

	glGenFramebuffers (1, &r->fbo);
	glBindFramebuffer (GL_FRAMEBUFFER, r->fbo);
	glGenTextures (1, &r->tex_fbo);
	glBindTexture (GL_TEXTURE_2D, r->tex_fbo);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r->tex_fbo, 0);
	glFramebufferTexture2D (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, r->tex_fbo, 0);
	glGenRenderbuffers (1, &r->rbo);
	glBindRenderbuffer (GL_RENDERBUFFER, r->rbo);
	glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, r->rbo);
	if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		assert (1 == 0);
	}
	glBindFramebuffer (GL_FRAMEBUFFER, 0);
}

void platform_ppu_mask (struct NESEmu *emu, void *_other_data)
{
	float r, g, b;
	nes_get_colors_background_clear (emu, &r, &g, &b);

	glClearColor (r, g, b, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT);
}

void platform_clear_mask (struct NESEmu *emu, uint8_t indx, void *_other_data)
{
	uint32_t plt = palette_get_color (emu, indx);
	uint8_t r = plt >> 24;
	uint8_t g = plt >> 16;
	uint8_t b = plt >> 8;
	glClearColor (r / 255.f, g / 255.f, b / 255.f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT);
}

int scanline_delay (struct NESEmu *emu)
{
	if (emu->cur_scanline_cycles >= SCANLINE_CYCLES_TOTAL) {
		emu->cur_scanline_cycles = 0;
		emu->scanline++;
		emu->indx_scroll_linex++;
		if (emu->scanline >= SCANLINE_SCREEN_HEIGHT) {
			emu->scanline = 0;
			emu->indx_scroll_linex = 0;
		}
		return 1;
	} else {
		return 0;
	}
}

int scanline_vblank (struct NESEmu *emu)
{
	if (emu->vblank_scanline_cycles >= 20) {
		emu->is_ready_to_vertical_blank = 0;
		emu->vblank_scanline_cycles = 0;
		return 1;
	}
	return 0;
}

int platform_delay (struct NESEmu *emu, void *_other_data)
{
    struct timeval tv;
    //gettimeofday (&tv, NULL);

    uint64_t ns = SDL_GetTicksNS ();

    if (emu->timestamp_cycles == 0L) {
	    emu->timestamp_cycles = ns;
    }

    uint64_t ret = ns - emu->timestamp_cycles;

    if (ret >= emu->last_cycles_int64) {
	    uint64_t last = ret - emu->last_cycles_int64;
	    emu->last_cycles_int64 = last;
	    emu->timestamp_cycles = ns;
	    return 0;
    } else {
	    return 1;
    }
}


uint32_t platform_and_scanline_delay (struct NESEmu *emu)
{
	uint32_t rets = 0;

	uint64_t ns = SDL_GetTicksNS ();

	if (emu->timestamp_cycles == 0L) {
		emu->timestamp_cycles = ns;
	}

	uint64_t ret = ns - emu->timestamp_cycles;

	if (ret >= emu->last_cycles_int64) {
		uint64_t last = 0; 
		emu->last_cycles_int64 = last;
		emu->timestamp_cycles = ns;
		rets |= DELAY_CYCLES;
		emu->cycles_to_scanline += emu->work_cycles;
	}

	//TODO: fix this
	if (emu->cycles_to_scanline >= CYCLES_TO_SCANLINE) {
		emu->cycles_to_scanline = emu->cycles_to_scanline - CYCLES_TO_SCANLINE;
		uint64_t last = 0;
		emu->scanline++;
		emu->vblank_scanline_cycles++;
		emu->indx_scroll_linex++;
		emu->last_scanline_int64 = last;
		emu->timestamp_scanline = ns;
		rets |= DELAY_SCANLINE;
		if (emu->scanline >= SCANLINE_SCREEN_HEIGHT) {
			emu->scanline = 0;
			emu->indx_scroll_linex = 0;
			emu->ppu_status |= PPUCTRL_VBLANK_NMI;
		}
	}
	return rets;
}

uint32_t platform_delay_nmi (struct NESEmu *emu, void *_other_data)
{
#if 0
    struct timeval tv;
    gettimeofday (&tv, NULL);

    uint64_t ms = SDL_GetTicksNS ();

    if (emu->start_time_nmi == 0L) {
            emu->start_time_nmi = ms;
            return 0;
    }

    uint64_t diff_time = ms - emu->start_time_nmi;

    if (diff_time >= 16666670) {
        emu->start_time_nmi = ms;
	return 1;
    }

    return 0;
#endif
    if (emu->cur_cycles >= 29829) {
	    emu->cur_cycles = 0;
	    return 1;
    }

    return 0;
}

static const char *vert_shader_str = 
"#version 450 core\n"
"layout (location = 0) in vec3 pos;\n"
"layout (location = 1) in vec2 tex_coord;\n"
"\n"
"uniform mat4 Ortho;\n"
"uniform mat4 Transform;\n"
"uniform mat4 Model;\n"
"uniform mat4 Scale;\n"
"\n"
"out vec2 tex_coordinates;\n"
"\n"
"void main ()\n"
"{\n"
"	gl_Position = Ortho * Transform * Scale * Model * vec4 (pos, 1.0);\n"
"	tex_coordinates = tex_coord;\n"
"}\n"
;

static const char *frag_shader_str =
"#version 450 core\n"
"\n"
"precision mediump float;\n"
"\n"
"in vec2 tex_coordinates;\n"
"\n"
"uniform sampler2D Texture0;\n"
"\n"
"layout (location = 0) out vec4 color_point;\n"
"\n"
"void main ()\n"
"{\n"
"	color_point = texture (Texture0, tex_coordinates);\n"
"}\n"
;


static void math_scale (float *e, float sc)
{
	e[0] = sc;
	e[5] = sc;
	e[10] = sc;
	e[15] = 1.f;
}

static void math_model (float *e, float sc)
{
	e[0] = sc;
	e[5] = sc;
	e[10] = sc;
	e[15] = sc;
}


static void math_ortho_rh (float *e, float left, float right,
		float bottom, float top,
		float near, float far)
{
	memset (e, 0, sizeof (float) * 16);

	float rl, tb, fn;

	rl = 1.f / (right - left);
	tb = 1.f / (top - bottom);
	fn = 1.f / (far - near);

	e[0] = 2.f * rl;
	e[5] = 2.f * tb;
	e[10] = 2.f * fn;
	e[12] = -(right + left) * rl;
	e[13] = -(top + bottom) * tb;
	e[14] = (far + near) * fn;
	e[15] = 1.f;
}

static void math_ortho_lh (float *e, float left, float right,
		float bottom, float top,
		float near, float far)
{
	memset (e, 0, sizeof (float) * 16);

	float rl, tb, fn;

	rl = 1.f / (right - left);
	tb = 1.f / (top - bottom);
	fn = -1.f / (far - near);

	e[0] = 2.f * rl;
	e[5] = 2.f * tb;
	e[10] = -2.f * fn;
	e[12] = -(right + left) * rl;
	e[13] = -(top + bottom) * tb;
	e[14] = (far + near) * fn;
	e[15] = 1.f;
}

static void math_translate (float *e, float x, float y, float z)
{
	memset (e, 0, sizeof (float) * 16);

#if 1
	e[0] = 1.f;
	e[5] = 1.f;
	e[10] = 1.f;
	e[15] = 1.f;
	e[12] = x;
	e[13] = y;
	e[14] = z;
#else
	e[0] = 1.f;
	e[5] = 1.f;
	e[10] = 1.f;
	e[15] = 1.f;
	e[3] = x;
	e[7] = y;
	e[11] = z;
#endif
}

static uint32_t create_shader (const char *shader_str, uint32_t type)
{
	uint32_t shader = glCreateShader (type);

	glShaderSource (shader, 1, (const GLchar * const *) &shader_str, NULL);

	glCompileShader (shader);

	return shader;
}

static uint32_t compile_shader (const char *vert_str, const char *frag_str)
{
	uint32_t vertex_shader = create_shader (vert_str, GL_VERTEX_SHADER);
	uint32_t fragment_shader = create_shader (frag_str, GL_FRAGMENT_SHADER);
	uint32_t shader = glCreateProgram ();
	glAttachShader (shader, vertex_shader);
	glAttachShader (shader, fragment_shader);
	glLinkProgram (shader);

	glDeleteShader (vertex_shader);
	glDeleteShader (fragment_shader);

	return shader;
}


static void init_space (struct NESEmu *emu, struct render_opengl_data *r)
{
	math_ortho_lh (r->ortho, 0, emu->width, emu->height, 0, -1, 10);
	math_model (r->model, 1.f);
	math_translate (r->transform, 0.f, 0.f, 0.f);
	math_scale (r->scale, 1.f);
}

static void init_sprite_array (struct NESEmu *emu, struct render_opengl_data *r)
{
	uint32_t count_bytes = 4 * 8 * 8;
	r->count_bytes = count_bytes;
}

static void init_textures (struct render_opengl_data *r, uint32_t tex_width, uint32_t tex_height)
{
	glGenTextures (64, r->sprite_texture);

	for (int i = 0; i < 64; i++) {
		glBindTexture (GL_TEXTURE_2D, r->sprite_texture[i]);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glBindTexture (GL_TEXTURE_2D, 0);

	// build background textures;
	glGenTextures (960 + 960, r->background_texture);
	for (int i = 0; i < (960 + 960); i++) {
		glBindTexture (GL_TEXTURE_2D, r->background_texture[i]);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glBindTexture (GL_TEXTURE_2D, 0);
}

static void connect_joystick (struct NESEmu *emu)
{
	struct sdl_data *s = emu->_window_data;

	SDL_JoystickID *joystick_ids = SDL_GetJoysticks (&s->joystick_count);

	if (s->joystick_count == 0)
		return;

	for (int i = 0; i < s->joystick_count; i++) {
		SDL_JoystickID id = joystick_ids[i];

		s->joy[i] = SDL_OpenJoystick (id);
	}

}

static void close_all_joystick (struct NESEmu *emu)
{
	struct sdl_data *s = emu->_window_data;

	for (int i = 0; i < s->joystick_count; i++) {
		SDL_CloseJoystick (s->joy[i]);
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

uint32_t nes_event (struct NESEmu *emu, void *_data)
{
	SDL_Event event = *((SDL_Event *) _data);

	uint32_t is_written = 0;

	switch (event.type) {
		case SDL_EVENT_JOYSTICK_HAT_MOTION:
			if (emu->is_new_state) {
				uint8_t temp = emu->state_buttons0 & 0x0f;
				emu->state_buttons0 &= (emu->state & 0xf0);
				if ((emu->state & 0xf0) == 0)
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
				emu->state = emu->state_buttons0;
				is_written = 1;
				emu->is_new_state = 0;
			} else {
				state_hat_buttons_get (&emu->state, event.jhat.value);
			}
			break;
		case SDL_EVENT_JOYSTICK_BALL_MOTION:
			break;
		case SDL_EVENT_JOYSTICK_AXIS_MOTION:
			break;
		case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
			if (emu->is_new_state) {
				state_button_get (&emu->state_buttons0, event.jbutton.button, event.jbutton.down);
				emu->state_buttons0 |= emu->state;
				is_written = 1;
				emu->is_new_state = 0;
			} else {
				state_button_get (&emu->state, event.jbutton.button, event.jbutton.down);
			}
			break;
		case SDL_EVENT_JOYSTICK_BUTTON_UP:
			if (emu->is_new_state) {
				state_button_get (&emu->state_buttons0, event.jbutton.button, event.jbutton.down);
				emu->state_buttons0 |= emu->state;
				is_written = 1;
				emu->is_new_state = 0;
			} else {
				state_button_get (&emu->state, event.jbutton.button, event.jbutton.down);
			}
			break;
		case SDL_EVENT_JOYSTICK_ADDED:
			connect_joystick (emu);
			break;
		case SDL_EVENT_QUIT:
			close_all_joystick (emu);
			exit (0);
			break;
	}

	if (is_written) {
		nes_write_state (emu);
	}
}

static void flip_hor (struct render_opengl_data *r, uint32_t is_h)
{
	float w = 8.f;
	float h = 8.f;

    	float vertices[] = {
             0.f,    0.f,  0.f, 0.f, 0.f,
             0.0f,     h,  0.f, 0.f, 1.f,
                w,   0.f,  0.f, 1.f, 0.f,
                w,   0.f,  0.f, 1.f, 0.f,
                w,     h,  0.f, 1.f, 1.f,
              0.f,     h,  0.f, 0.f, 1.f
    	};

    float *v = vertices;

	if (!is_h) {
		v[3]  = 1.f; v[4]  = 0.f;
		v[8]  = 1.f; v[9]  = 1.f;
		v[13] = 0.f; v[14] = 0.f;
		v[18] = 0.f; v[19] = 0.f;
		v[23] = 0.f; v[24] = 1.f;
		v[28] = 1.f; v[29] = 1.f;
	} else {
		v[3]  = 0.f; v[4]  = 0.f;
		v[8]  = 0.f; v[9]  = 1.f;
		v[13] = 1.f; v[14] = 0.f;
		v[18] = 1.f; v[19] = 0.f;
		v[23] = 1.f; v[24] = 1.f;
		v[28] = 0.f; v[29] = 1.f;
	}


    glBindBuffer (GL_ARRAY_BUFFER, r->vbo);
    glBufferSubData (GL_ARRAY_BUFFER, 0, sizeof (vertices), vertices);
}

void nes_init_surface (struct NESEmu *emu)
{
	framebuffer_init (emu);

	struct render_opengl_data *r = emu->_render_data;

	float w = emu->width;
	float h = emu->height;

#if 0
	float vertices[] = {
             0.f,    0.f,  0.f, 0.f, 0.f,
             0.0f,     h,  0.f, 0.f, 1.f,
                w,   0.f,  0.f, 1.f, 0.f,
                w,   0.f,  0.f, 1.f, 0.f,
                w,     h,  0.f, 1.f, 1.f,
              0.f,     h,  0.f, 0.f, 1.f
	};
#else
	float vertices[] = {
              0.f,    0.f,  0.f, 0.f, 1.f,
              0.f,      h,  0.f, 0.f, 0.f,
                w,    0.f,  0.f, 1.f, 1.f,
                w,    0.f,  0.f, 1.f, 1.f,
                w,      h,  0.f, 1.f, 0.f,
              0.f,      h,  0.f, 0.f, 0.f
	};
#endif

	float *v = vertices;

	uint32_t vao, vbo;

	glGenVertexArrays (1, &vao);
	glGenBuffers (1, &vbo);
	glBindVertexArray (vao);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glBufferData (GL_ARRAY_BUFFER, sizeof (vertices), vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof (float), (void *) 0);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof (float), (void *) (3 * sizeof (float)));

	glEnableVertexAttribArray (0);
	glEnableVertexAttribArray (1);

	glBindVertexArray (0);

	r->framebuffer_vao = vao;
	r->framebuffer_vbo = vbo;
}

static void init_vao (struct NESEmu *emu, struct render_opengl_data *r)
{
	float w = 8.f;
	float h = 8.f;

#if 1
    float vertices[] = {
             0.f,    0.f,  0.f, 0.f, 0.f,
             0.0f,     h,  0.f, 0.f, 1.f,
                w,   0.f,  0.f, 1.f, 0.f,
                w,   0.f,  0.f, 1.f, 0.f,
                w,     h,  0.f, 1.f, 1.f,
              0.f,     h,  0.f, 0.f, 1.f
    };
#else
    float vertices[] = {
         w,    0,  0.f, 1.f, 0.f,
         w,    h,  0.f, 1.f, 1.f,
         0.f,  h,  0.f, 0.f, 1.f,
         0.f,  h,  0.f, 0.f, 1.f,
         0.f,  0.f,  0.f, 0.f, 0.f,
         w,    0.f,  0.f, 1.f, 0.f
    };
#endif

    float *v = vertices;

    uint32_t vao, vbo;

    glGenVertexArrays (1, &vao);
    glGenBuffers (1, &vbo);
    glBindVertexArray (vao);
    glBindBuffer (GL_ARRAY_BUFFER, vbo);
    glBufferData (GL_ARRAY_BUFFER, sizeof (vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof (float), (void *) 0);
    glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof (float), (void *) (3 * sizeof (float)));

    glEnableVertexAttribArray (0);
    glEnableVertexAttribArray (1);

    glBindVertexArray (0);

    r->vao = vao;
    r->vbo = vbo;
}

static void init_id (struct NESEmu *emu, struct render_opengl_data *r)
{
	glUseProgram (r->program);

	r->id_ortho = glGetUniformLocation (r->program, "Ortho");
	r->id_transform = glGetUniformLocation (r->program, "Transform");
	r->id_scale = glGetUniformLocation (r->program, "Scale");
	r->id_model = glGetUniformLocation (r->program, "Model");
	r->id_sampler = glGetUniformLocation (r->program, "Texture0");

	glUseProgram (0);
}

void platform_alloc_memory_map (struct NESEmu *emu)
{
	emu->mem = malloc (emu->sz_prg_rom);
	emu->chr = malloc (emu->sz_chr_rom);
	emu->ppu = malloc (0x4000);
	emu->ppu_copy = malloc (0x4000);
	emu->ppu_scroll = malloc (0x4000);

	memset (emu->mem, 0, emu->sz_prg_rom);
	memset (emu->chr, 0, emu->sz_chr_rom);
	memset (emu->ppu, 0, 0x4000);
	memset (emu->ppu_copy, 0, 0x4000);
	memset (emu->ppu_scroll, 0, 0x4000);
}

void nes_platform_init (struct NESEmu *emu, void *_other_data)
{
	struct render_opengl_data *r = malloc (sizeof (struct render_opengl_data));
	memset (r, 0, sizeof (struct render_opengl_data));
	emu->_render_data = r;

	struct sdl_data *s = malloc (sizeof (struct sdl_data));
	memset (s, 0, sizeof (struct sdl_data));
	emu->_window_data = s;

	r->program = compile_shader (vert_shader_str, frag_shader_str);
	init_space (emu, r);
	init_sprite_array (emu, r);
	init_textures (r, 8, 8);
	init_vao (emu, r);
	init_id (emu, r);
}


static void build_background (struct NESEmu *emu, struct render_opengl_data *r, uint8_t id_texture, uint8_t x, uint8_t y, uint16_t naddr, uint32_t indx_screen)
{
	static const uint16_t palette[4] = {
		0x23c0,
		0x27c0,
		0x2bc0,
		0x2fc0
	};

	uint32_t indx = 0;

	uint32_t py = y * 8 / 32;
	uint32_t px = x * 8 / 32; 
	uint16_t id = py * 8 + px;

	uint8_t pal = emu->ppu[palette[indx_screen] + id];

	float mx, my;
	mx = my = 0.f;
	mx = x;
	my = y;
	float mmx = mx * 8 / 32;
	float mmy = my * 8 / 32;
	float lx = mmx - px;
	float ly = mmy - py;

	uint8_t col = ly >= 0.5f? 4: 0;
	col += lx >= 0.5f? 2: 0;

	uint16_t addr = ((emu->ctrl[REAL_PPUCTRL] & PPUCTRL_BACKGROUND_PATTERN) == 0x0? 0x0: 0x1000);

	uint8_t *ptr = &emu->chr[addr];
	ptr += id_texture * 16; 

	uint8_t sprite_data[16];
	memcpy (sprite_data, ptr, 16);

	uint8_t sprite_buffer[4 * 8 * 8];
	uint8_t *sp = sprite_buffer;

	indx = (pal >> col) & 0x03;

	for (int i = 0; i < 8; i++) {
		uint8_t s = 0x80;
		uint8_t low = sprite_data[i + 0];
		uint8_t high = sprite_data[i + 8];

		for (int ii = 0; ii < 8; ii++) {
			uint8_t n = 0;
			if (low & s) n = 1;
			if (high & s) n |= 2;
			uint32_t plt = palette_get_color (emu, r->palette_image[indx * 4 + n]);
			*sp++ = (plt >>  0) & 0xff;
			*sp++ = (plt >>  8) & 0xff;
			*sp++ = (plt >> 16) & 0xff;
			if (n == 0)
				*sp++ = 0x00;
			else
				*sp++ = 0xff;
			s >>= 1;
		}

	}

	glBindTexture (GL_TEXTURE_2D, r->background_texture[naddr]);

	glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, 8, 8, GL_RGBA, GL_UNSIGNED_BYTE, sprite_buffer);

	glBindTexture (GL_TEXTURE_2D, 0);
}

static void build_texture (struct NESEmu *emu, struct render_opengl_data *r, uint8_t id_texture, uint8_t flags, uint32_t index_texture)
{
	uint16_t addr_palette = 0x3f10;
	uint8_t p[4][4];
	for (int i = 0; i < 4; i++) {
		p[i][0] = emu->ppu[addr_palette + 0];
		p[i][1] = emu->ppu[addr_palette + 1];
		p[i][2] = emu->ppu[addr_palette + 2];
		p[i][3] = emu->ppu[addr_palette + 3];

		addr_palette += 4;
	}

	uint32_t is_hor = 0;

	if (flags & FLIP_SPRITE_HORIZONTALLY) {
		is_hor = 1;
	}

	uint16_t addr = ((emu->ctrl[REAL_PPUCTRL] & PPUCTRL_SPRITE_PATTERN) == 0x0? 0x0: 0x1000);

	uint8_t *ptr = &emu->chr[addr];
	ptr += (id_texture * 16); 

	uint8_t sprite_data[16];
	memcpy (sprite_data, ptr, 16);

	uint8_t sprite_buffer[4 * 8 * 8];
	uint8_t *sp = sprite_buffer;

	uint8_t palette_cur = flags & 0x3;

	for (int i = 0; i < 8; i++) {
		uint8_t s = is_hor? 0x01: 0x80;
		uint8_t low = sprite_data[i + 0];
		uint8_t high = sprite_data[i + 8];

		for (int ii = 0; ii < 8; ii++) {
			uint8_t n = 0;
			if (low & s) n = 1;
			if (high & s) n |= 2;
			uint32_t plt = palette_get_color (emu, p[palette_cur][n]);
			*sp++ = (plt >>  0) & 0xff;
			*sp++ = (plt >>  8) & 0xff;
			*sp++ = (plt >> 16) & 0xff;
			if (n == 0)
				*sp++ = 0x00;
			else
				*sp++ = 0xff;
			if (is_hor)
				s <<= 1;
			else
				s >>= 1;
		}
	}

	glBindTexture (GL_TEXTURE_2D, r->sprite_texture[index_texture]);

	glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, 8, 8, GL_RGBA, GL_UNSIGNED_BYTE, sprite_buffer);

	glBindTexture (GL_TEXTURE_2D, 0);
}

static uint8_t pp[256];
static int is_pp;

enum {
	SPRITE_BEFORE_BACKGROUND,
	SPRITE_AFTER_BACKGROUND,
	N_SPRITE_CONDITION
};

static void draw_sprite_if (struct NESEmu *emu, uint32_t condition)
{
	struct render_opengl_data *r = emu->_render_data;


	uint8_t LOWER_BACKGROUND = 32;
	uint32_t idx = 0;
	for (int i = 0; i < 64; i++) {

		uint8_t flags = emu->oam[idx + 2];

		if ((flags & LOWER_BACKGROUND) && (condition == SPRITE_AFTER_BACKGROUND)) {
			idx += 4;
			continue;
		}
		
		if (!(flags & LOWER_BACKGROUND) && (condition == SPRITE_BEFORE_BACKGROUND)) {
			idx += 4;
			continue;
		}

		uint8_t py = emu->oam[idx + 0];
		uint8_t id_texture = emu->oam[idx + 1];
		uint8_t px = emu->oam[idx + 3];

		math_translate (r->transform, px, py - 8, 0.f);

		build_texture (emu, r, id_texture, flags, i);

		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, r->sprite_texture[i]);
		glUniform1i (r->id_sampler, 0);

		glUniformMatrix4fv (r->id_ortho, 1, GL_FALSE, r->ortho);
		glUniformMatrix4fv (r->id_transform, 1, GL_FALSE, r->transform);
		glUniformMatrix4fv (r->id_scale, 1, GL_FALSE, r->scale);
		glUniformMatrix4fv (r->id_model, 1, GL_FALSE, r->model);

		glEnableVertexAttribArray (0);
		glEnableVertexAttribArray (1);

		glDrawArrays (GL_TRIANGLES, 0, 6);

		idx += 4;
	}
}

#define WIDTH_LINE                           32

static uint32_t draw_screen (struct NESEmu *emu, uint16_t addr, uint32_t indx_screen,
		int32_t _offset)
{
	struct render_opengl_data *r = emu->_render_data;

	int32_t ppx = _offset;
	int32_t off_r = 0;
	if (_offset > 0) {
		off_r = 960;
	}
	int32_t ppy = 0;
	uint8_t x, y;
	x = y = 0;
	uint16_t off_addr = 0;

	uint8_t off_x = _offset / 8;

	uint32_t cur_indx_screen = indx_screen;
	uint32_t is_next_screen = 0;
	uint16_t naddr = addr;

	uint16_t line = WIDTH_LINE;

	uint32_t scanline = 0;

	uint32_t indx_scr_x = 0;

	uint8_t offx = 0;
	uint16_t last = 0;
	uint16_t last_off = 0;
	uint16_t next_screen = 0;
	uint8_t off = 0;

	offx = emu->scroll_x[0];
	last_off = offx / 8;
	next_screen = WIDTH_LINE - last_off;
	off = emu->offx % 8;

	uint8_t start_y = 0;

	for (uint16_t i = 0; i < 960; ) {

		if (
				((indx_scr_x + 1) < emu->max_scroll_indx) && 
				(scanline > emu->scroll_tile_x[indx_scr_x]) &&
				(scanline >= emu->scroll_tile_x[indx_scr_x + 1])) {
			indx_scr_x++;
			offx = emu->scroll_x[indx_scr_x];
			last_off = offx / 8;
			off = offx % 8;
			if (offx > 0) {
				is_next_screen = 1;
			}
		} else {
			offx = emu->scroll_x[indx_scr_x];
		}




		naddr = i + addr;

		uint8_t id_texture = emu->ppu[naddr];

		math_translate (r->transform, ppx - offx, ppy - 8, 0.f);

		if ((emu->ppu_copy[naddr] != emu->ppu[naddr]) || emu->is_new_palette_background) {
			build_background (emu, r, id_texture, x, y, i + off_r, cur_indx_screen);
			emu->ppu_copy[naddr] = emu->ppu[naddr];
		}

		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, r->background_texture[i + off_r]);
		glUniform1i (r->id_sampler, 0);

		glUniformMatrix4fv (r->id_ortho, 1, GL_FALSE, r->ortho);
		glUniformMatrix4fv (r->id_transform, 1, GL_FALSE, r->transform);
		glUniformMatrix4fv (r->id_scale, 1, GL_FALSE, r->scale);
		glUniformMatrix4fv (r->id_model, 1, GL_FALSE, r->model);

		glEnableVertexAttribArray (0);
		glEnableVertexAttribArray (1);

		glDrawArrays (GL_TRIANGLES, 0, 6);

		ppx += 8;
		x++;

		i++;
		if ((i > 0) && ((i % WIDTH_LINE) == 0)) {
			line += WIDTH_LINE;
			x = 0;
			y++;
			ppx = _offset;
			ppy += 8;
			off_addr = 0;
			last = last_off;
			scanline++;
			start_y = y;
		}
	}

	return is_next_screen;
}

static void draw_ppu (struct NESEmu *emu)
{
	struct render_opengl_data *r = emu->_render_data;

	uint32_t is_rec = 0;

	uint16_t ppu_addr[4] = {
		0x2000,
		0x2400,
		0x2800,
		0x2c00

	};

	uint32_t indx_screen0 = emu->ctrl[REAL_PPUCTRL] & 0x3;
	uint32_t indx_screen1 = indx_screen0 + 1;
	uint16_t addr0 = ppu_addr[indx_screen0];
	uint16_t addr1 = ppu_addr[indx_screen1];

	uint32_t is_next_screen = draw_screen (emu, addr0, indx_screen0, 0);

	if (is_next_screen) {
		draw_screen (emu, addr1, indx_screen1, 256);
	}
	emu->max_scroll_indx = 0;
}


static void bind_vertex_group (struct render_opengl_data *r)
{
	glUseProgram (r->program);

	glBindVertexArray (r->vao);
}

static void recreate_palette (struct NESEmu *emu, struct render_opengl_data *r)
{
	static const uint16_t addr_palette = 0x3f00;
	uint16_t ix = 0;
	for (int y = 0; y < 16; y++) {
		if ((r->palette_image[ix] != emu->ppu[addr_palette + ix])) {
			emu->is_new_palette_background = 1;
		} else {
			ix++;
			continue;
		}

		printf ("recreate palette\n");
		for (int i = 0; i < 16; i += 4) {
			r->palette_image[i + 0] = emu->ppu[addr_palette + 0 + i];
			r->palette_image[i + 1] = emu->ppu[addr_palette + 1 + i];
			r->palette_image[i + 2] = emu->ppu[addr_palette + 2 + i];
			r->palette_image[i + 3] = emu->ppu[addr_palette + 3 + i];
		}
		break;
	}
}

static void render_to_framebuffer (struct NESEmu *emu)
{
	glDisable (GL_DEPTH_TEST);
	struct render_opengl_data *r = emu->_render_data;

	glBindFramebuffer (GL_FRAMEBUFFER, r->fbo);

	bind_vertex_group (r);

	recreate_palette (emu, r);

	platform_clear_mask (emu, r->palette_image[0], NULL);

	if (emu->ctrl[REAL_PPUMASK] & MASK_IS_SPRITE_RENDER)
		draw_sprite_if (emu, SPRITE_BEFORE_BACKGROUND);

	draw_ppu (emu);

	if (emu->ctrl[REAL_PPUMASK] & MASK_IS_SPRITE_RENDER)
		draw_sprite_if (emu, SPRITE_AFTER_BACKGROUND);

	if (emu->is_new_palette_background) {
		emu->is_new_palette_background = 0;
	}

	glBindFramebuffer (GL_FRAMEBUFFER, 0);
	glEnable (GL_DEPTH_TEST);
}

static void render_to_surface (struct NESEmu *emu)
{
	struct render_opengl_data *r = emu->_render_data;

	glUseProgram (r->program);

	glBindVertexArray (r->framebuffer_vao);

	math_translate (r->transform, 0.f, 0.f, 0.f);

	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, r->tex_fbo);
	glUniform1i (r->id_sampler, 0);

	glUniformMatrix4fv (r->id_ortho, 1, GL_FALSE, r->ortho);
	glUniformMatrix4fv (r->id_transform, 1, GL_FALSE, r->transform);
	glUniformMatrix4fv (r->id_scale, 1, GL_FALSE, r->scale);
	glUniformMatrix4fv (r->id_model, 1, GL_FALSE, r->model);

	glEnableVertexAttribArray (0);
	glEnableVertexAttribArray (1);

	glDrawArrays (GL_TRIANGLES, 0, 6);
}

void nes_render (struct NESEmu *emu, void *_other_data)
{
	static int in = 0;
	SDL_Window *win = _other_data;

	glViewport (0, 0, emu->width, emu->height);
	render_to_framebuffer (emu);

	//glViewport (0, 0, emu->width * emu->scale, emu->height * emu->scale);
	//render_to_surface (emu);
}


