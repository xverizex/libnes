#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <cpunes.h>
#include <string.h>
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

static const char *vert_shader_str = 
"#version 300 es\n"
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
"#version 300 es\n"
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

struct render_linux_data {
	uint32_t program;
	float ortho[16];
	float transform[16];
	float scale[16];
	float model[16];
	uint32_t sampler;
};

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
	fn = -1.f / (far - near);

	e[0] = 2.f * rl;
	e[5] = 2.f * tb;
	e[10] = 2.f * fn;
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
	uint32_t fragment_shader = create_shader (frag_str, GL_VERTEX_SHADER);
	uint32_t shader = glCreateProgram ();
	glAttachShader (shader, vertex_shader);
	glAttachShader (shader, fragment_shader);
	glLinkProgram (shader);

	glDeleteShader (vertex_shader);
	glDeleteShader (fragment_shader);

	return shader;
}

static void init_space (struct NESEmu *emu, struct render_linux_data *r)
{
	math_ortho_rh (r->ortho, 0, emu->width, emu->height, 0, -1, 10);
	math_model (r->model, 1.f);
	math_translate (r->transform, 0.f, 0.f, 0.f);
	math_scale (r->scale, 1.f);
}

void linux_opengl_init (struct NESEmu *emu, void *_other_data)
{
	struct render_linux_data *r = malloc (sizeof (struct render_linux_data));
	memset (r, 0, sizeof (struct render_linux_data));
	r->program = compile_shader (vert_shader_str, frag_shader_str);
	init_space (r);

	emu->_render_data = r;
}

void linux_init_callbacks (struct NESCallbacks *cb)
{
	cb->init = linux_opengl_init;
	cb->print_debug = linux_print_debug;
	cb->calc_time_uint64 = linux_calc_time_uint64;
	cb->ppu_mask = linux_clear_screen;
	cb->calc_nmi = linux_calc_time_nmi;
}
