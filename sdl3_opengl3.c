#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <cpunes.h>
#include <string.h>
#include <GLES3/gl3.h>
#include <stdlib.h>
#include <SDL3/SDL.h>

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

uint32_t platform_delay_nmi (struct NESEmu *emu, void *_other_data)
{
#if 1
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
	uint32_t background_texture[960];
	uint32_t sprite_texture[64];
	uint32_t vao;
	uint32_t vbo;
	uint8_t palette_image[16];
	uint32_t is_need_reorder_palette_background;
};

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
	glGenTextures (960, r->background_texture);
	for (int i = 0; i < 960; i++) {
		glBindTexture (GL_TEXTURE_2D, r->background_texture[i]);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glBindTexture (GL_TEXTURE_2D, 0);
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

	memset (emu->mem, 0, emu->sz_prg_rom);
	memset (emu->chr, 0, emu->sz_chr_rom);
	memset (emu->ppu, 0, 0x4000);
	memset (emu->ppu_copy, 0, 0x4000);
}

void platform_init (struct NESEmu *emu, void *_other_data)
{
	struct render_opengl_data *r = malloc (sizeof (struct render_opengl_data));
	memset (r, 0, sizeof (struct render_opengl_data));
	r->program = compile_shader (vert_shader_str, frag_shader_str);
	init_space (emu, r);
	init_sprite_array (emu, r);
	init_textures (r, 8, 8);
	init_vao (emu, r);
	init_id (emu, r);

	emu->_render_data = r;
}


static void build_background (struct NESEmu *emu, struct render_opengl_data *r, uint8_t id_texture, uint8_t x, uint8_t y, uint16_t naddr, uint32_t indx_screen)
{
	static const uint16_t palette[2] = {
		0x23c0,
		0x27c0
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

		math_translate (r->transform, px, py, 0.f);

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

static void draw_ppu (struct NESEmu *emu)
{
	struct render_opengl_data *r = emu->_render_data;

	int32_t ppx = 0;
	int32_t ppy = 0;
	uint8_t x, y;
	x = y = 0;
	int ind = 0;
	int ddt = 0;
	int m = 0;
	x = y = ppx = ppy = 0;

	uint16_t ppu_addr[4] = {
		0x2000,
		0x2400,
		0x2800,
		0x2c00

	};


	uint16_t addr = ppu_addr[emu->ctrl[REAL_PPUCTRL] & 0x3];

	uint32_t indx_screen = 0;

	for (uint16_t i = 0; i < 960; i++) {

		if ((i > 0) && ((i % 32) == 0)) {
			x = 0;
			y++;
			ppx = 0;
			ppy += 8;
		}

		uint16_t naddr = i + addr;

		uint8_t id_texture = emu->ppu[naddr];

		math_translate (r->transform, ppx, ppy, 0.f);

		if ((emu->ppu_copy[naddr] != emu->ppu[naddr]) || emu->is_new_palette_background) {
			build_background (emu, r, id_texture, x, y, i, indx_screen);
			emu->ppu_copy[naddr] = emu->ppu[naddr];
		}

		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, r->background_texture[i]);
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
	}
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

void platform_render (struct NESEmu *emu, void *_other_data)
{
	SDL_Window *win = _other_data;

	struct render_opengl_data *r = emu->_render_data;

	bind_vertex_group (r);

	recreate_palette (emu, r);

	platform_clear_mask (emu, r->palette_image[0], NULL);

	draw_sprite_if (emu, SPRITE_BEFORE_BACKGROUND);

	draw_ppu (emu);

	draw_sprite_if (emu, SPRITE_AFTER_BACKGROUND);

	if (emu->is_new_palette_background) {
		emu->is_new_palette_background = 0;
	}

	SDL_GL_SwapWindow (win);

	emu->ppu_status |= 0x80;
}
