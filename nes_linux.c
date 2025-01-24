#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <cpunes.h>
#include <string.h>
#include <GLES3/gl3.h>
#include <stdlib.h>

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

    if (diff_time >= 44) {
        emu->start_time_nmi = 0;
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
	uint32_t fragment_shader = create_shader (frag_str, GL_FRAGMENT_SHADER);
	uint32_t shader = glCreateProgram ();
	glAttachShader (shader, vertex_shader);
	glAttachShader (shader, fragment_shader);
	glLinkProgram (shader);

	glDeleteShader (vertex_shader);
	glDeleteShader (fragment_shader);

	return shader;
}

#define SPRITE_COUNT             (16 * 16)

struct render_linux_data {
	uint32_t program;
	float ortho[16];
	float transform[16];
	float scale[16];
	float model[16];
	uint32_t id_ortho;
	uint32_t id_transform;
	uint32_t id_scale;
	uint32_t id_model;
	uint32_t id_sampler;
	uint32_t texture;
	uint32_t *sprites[SPRITE_COUNT];
	uint8_t sprite_bits[SPRITE_COUNT][16];
	uint32_t sprite_texture[SPRITE_COUNT];
	uint8_t sprite_bits_one[16];
	uint32_t vao;
	uint32_t vbo;
};

static void init_space (struct NESEmu *emu, struct render_linux_data *r)
{
	math_ortho_rh (r->ortho, 0, emu->width, emu->height, 0, -1, 10);
	math_model (r->model, 1.f);
	math_translate (r->transform, 0.f, 0.f, 0.f);
	math_scale (r->scale, 1.f);
}

static void init_sprite_array (struct NESEmu *emu, struct render_linux_data *r)
{
	uint32_t count = SPRITE_COUNT;
	
	uint32_t count_bytes = sizeof (uint32_t) * 8 * 8;
	for (uint32_t i = 0; i < count; i++) {
		r->sprites[i] = malloc (count_bytes);
		memset (r->sprites[i], 0, count_bytes);
	}	
}

static void init_textures (uint32_t *tex, uint32_t tex_width, uint32_t tex_height)
{
	uint32_t count = SPRITE_COUNT;

	glGenTextures (count, tex);

	for (uint32_t i = 0; i < count; i++) {
		glBindTexture (GL_TEXTURE_2D, tex[i]);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture (GL_TEXTURE_2D, 0);
	}
}

static void init_vao (struct NESEmu *emu, struct render_linux_data *r)
{
	float w = 8;
	float h = 8;

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
         0.f,    0.f,  0.f, 0.f, 1.f,
         0.0f,     h,  0.f, 0.f, 0.f,
            w,   0.f,  0.f, 1.f, 1.f,
            w,   0.f,  0.f, 1.f, 1.f,
            w,     h,  0.f, 1.f, 0.f,
          0.f,     h,  0.f, 0.f, 0.f
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

static void init_id (struct NESEmu *emu, struct render_linux_data *r)
{
	glUseProgram (r->program);

	r->id_ortho = glGetUniformLocation (r->program, "Ortho");
	r->id_transform = glGetUniformLocation (r->program, "Transform");
	r->id_scale = glGetUniformLocation (r->program, "Scale");
	r->id_model = glGetUniformLocation (r->program, "Model");
	r->id_sampler = glGetUniformLocation (r->program, "Texture0");

	glUseProgram (0);
}

void linux_opengl_init (struct NESEmu *emu, void *_other_data)
{
	struct render_linux_data *r = malloc (sizeof (struct render_linux_data));
	memset (r, 0, sizeof (struct render_linux_data));
	r->program = compile_shader (vert_shader_str, frag_shader_str);
	init_space (emu, r);
	init_sprite_array (emu, r);
	init_textures (r->sprite_texture, 8, 8);
	init_vao (emu, r);
	init_id (emu, r);

	emu->_render_data = r;
}

/*
 * 1. get palette +
 * 2. get char data texture +
 * 3. fill texture buffer +
 */

uint32_t palette_get_color (struct NESEmu *emu, uint8_t idx);

void debug (uint8_t *mem)
{
	for (int i = 0; i < 512; i++) {
		if (i > 0 && i % 16 == 0) {
			printf ("\n");
		}
		printf ("%02x ", *mem++);
	}
	printf ("\n");
	printf ("\n");
}

static void build_texture (struct NESEmu *emu, struct render_linux_data *r, uint8_t id_texture)
{
	uint16_t addr_palette = 0x3f00;
	uint8_t p[4][4];
	for (int i = 0; i < 4; i++) {
		p[i][0] = emu->mem[addr_palette + 0];
		p[i][1] = emu->mem[addr_palette + 1];
		p[i][2] = emu->mem[addr_palette + 2];
		p[i][3] = emu->mem[addr_palette + 3];

		addr_palette += 4;
	}

	uint16_t addr = ((emu->mem[PPUCTRL] & PPUCTRL_SPRITE_PATTERN) == 0x0? 0x0: 0x1000);

	uint8_t *ptr = &emu->mem[addr];
	ptr += id_texture * 16; 

	memcpy (r->sprite_bits_one, ptr, 16);


	uint8_t *sp = (uint8_t *) r->sprites[id_texture];

	for (int i = 0; i < 8; i++) {
		uint8_t s = 0x80;
		uint8_t low = r->sprite_bits_one[i + 0];
		uint8_t high = r->sprite_bits_one[i + 8];

		for (int ii = 0; ii < 8; ii++) {
			uint8_t n = 0;
			if (low & s) n = 1;
			if (high & s) n |= 2;
			uint32_t plt = palette_get_color (emu, p[0][n]);
			*sp++ = (plt >>  0) & 0xff;
			*sp++ = (plt >>  8) & 0xff;
			*sp++ = (plt >> 16) & 0xff;
			*sp++ = 0xff;
			s >>= 1;
		}

	}

	glBindTexture (GL_TEXTURE_2D, r->sprite_texture[id_texture]);

	glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, 8, 8, GL_RGBA, GL_UNSIGNED_BYTE, r->sprites[id_texture]);

	glBindTexture (GL_TEXTURE_2D, 0);
}

void linux_opengl_render (struct NESEmu *emu, void *_other_data)
{
	struct render_linux_data *r = emu->_render_data;
	uint16_t idx = 0;

	glUseProgram (r->program);

	glBindVertexArray (r->vao);

	for (int i = 0; i < 256; i++) {

		uint8_t px = emu->ram[0x200 + idx + 3];
		uint8_t py = emu->ram[0x200 + idx + 0];
		uint8_t flags = emu->ram[0x200 + idx + 2];
		uint8_t id_texture = emu->ram[0x200 + idx + 1];

		math_translate (r->transform, px, py, 0.f);

		build_texture (emu, r, id_texture);

		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, r->sprite_texture[id_texture]);
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

void linux_init_callbacks (struct NESCallbacks *cb)
{
	cb->init = linux_opengl_init;
	cb->print_debug = linux_print_debug;
	cb->calc_time_uint64 = linux_calc_time_uint64;
	cb->ppu_mask = linux_clear_screen;
	cb->calc_nmi = linux_calc_time_nmi;
	cb->render = linux_opengl_render;
}
