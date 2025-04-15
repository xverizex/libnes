#ifndef CPUNES_H
#define CPUNES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define STATUS_FLAG_CF        (1 << 0)
#define STATUS_FLAG_ZF        (1 << 1)  
#define STATUS_FLAG_IF        (1 << 2)
#define STATUS_FLAG_DF        (1 << 3)
#define STATUS_FLAG_BF        (1 << 4)
#define STATUS_FLAG_UNUSED0   (1 << 5)
#define STATUS_FLAG_VF        (1 << 6)
#define STATUS_FLAG_NF        (1 << 7)
/* NVUBDIZC */
/* 76543210 */
#define PPUCTRL_SPRITE_PATTERN			(1 << 3)
#define PPUCTRL_BACKGROUND_PATTERN		(1 << 4)
#define PPUCTRL_VBLANK_NMI    			(1 << 7)

#define FLIP_SPRITE_HORIZONTALLY		(1 << 6)

#define MAX_NMI_CYCLES		1136
/*
 * or 2273 cycles for MAX_NMI_CYCLES
 */

enum {
	PPUCTRL = 0x2000,
	PPUMASK, // 0x2001
	PPUSTATUS, // 0x2002
	OAMADDR, // 0x2003
	OAMDATA, // 0x2004
	PPUSCROLL, // 0x2005
	PPUADDR, // 0x2006
	PPUDATA, // 0x2007
	OAMDMA = 0x4014,
	N_PPUMANAGE
};

enum {
	REAL_PPUCTRL,
	REAL_PPUMASK,
	REAL_PPUSTATUS,
	REAL_OAMADDR,
	REAL_OAMDATA,
	REAL_PPUSCROLL,
	REAL_PPUADDR,
	REAL_PPUDATA,
	REAL_OAMDMA,
	N_REAL_PPUMANAGE
};

enum {
	FORMAT_UNKNOWN,
	FORMAT_INES,
	FORMAT_NES20,
	N_FORMAT_NES
};

enum {
	NAME_TABLE_HORIZONTAL_ARRANGEMENT,
	NAME_TABLE_VERTICAL_ARRANGEMENT,
	N_NAME_TABLE
};

struct CPUNes {
    uint8_t A;
    uint8_t X;
    uint8_t Y;
    uint16_t PC;
    uint8_t S;
    uint8_t P;
};

struct NESEmu;

typedef void (*ppu_manager) (struct NESEmu *, uint8_t *r, uint8_t is_write, uint8_t *returned_reg);

struct NESCallbacks {
	void (*init) (struct NESEmu *emu, void *_other_data);
	float (*calc_time_float) (struct NESEmu *emu, void *_other_data);
	int (*calc_time_uint64) (struct NESEmu *emu, void *_other_data);
	void (*print_debug) (struct NESEmu *emu, void *_other_data);
	void (*ppu_mask) (struct NESEmu *emu, void *_other_data);
	uint32_t (*calc_nmi) (struct NESEmu *emu, void *_other_data);
	void (*render) (struct NESEmu *emu, void *_other_data);
};

#define MASK_IS_NORMAL_GRAYSCALE_RENDER			(1 << 0)
#define MASK_IS_BACKGROUND_SHOW_LEFTMOST		(1 << 1)
#define MASK_IS_SPRITE_SHOW_LEFTMOST			(1 << 2)
#define MASK_IS_BACKGROUND_RENDER			(1 << 3)
#define MASK_IS_SPRITE_RENDER				(1 << 4)
#define MASK_IS_EMPHASIZE_RED				(1 << 5)
#define MASK_IS_EMPHASIZE_GREEN				(1 << 6)
#define MASK_IS_EMPHASIZE_BLUE				(1 << 7)

#define RAM_MAX                 0x800


#define TV_SYSTEM_NTSC             0
#define TV_SYSTEM_PAL              1

#define SCANLINE_SCREEN_HEIGHT            262
#define SCANLINE_CYCLES_TOTAL             113
#define SCANLINE_VBLANK_START             241
#define PPU_CYCLE                          46
#define NS_CYCLE                         559L

struct breakpoint {
	uint16_t addr;
	uint8_t A;
	uint8_t X;
	uint8_t Y;
	uint8_t P;
	uint8_t regs[4];
	uint32_t condition;
	uint32_t is_enabled;
};

enum write_cond {
	WRITE_COND_NO,
	WRITE_COND_EQ,
	WRITE_COND_NOT_EQ,
	N_WRITE_COND
};

struct breakwrite {
	uint16_t addr;
	uint8_t val;
	enum write_cond cond;
	uint32_t is_enabled;
};

struct NESEmu {
    struct CPUNes cpu;
    uint8_t is_branch;

    uint8_t *dump;
    uint32_t sz_dump;

    uint32_t fmt_dump;

    uint32_t sz_prg_rom;
    uint32_t sz_chr_rom;
    uint32_t sz_prg_ram;
    uint8_t mapper;

    uint16_t nmi_handler;
    uint16_t reset_handler;
    uint16_t irq_handler;

    uint8_t tv_system;
    uint64_t last_scanline_int64;
    float last_cycles_float;
    uint64_t last_cycles_int64;
    uint64_t tmp_last_cycles_int64;
    float capasity_time;

    uint64_t start_time_nmi;

    uint8_t state;
    uint8_t is_debug_list;
    uint8_t is_show_address;
    uint8_t is_show_bytes;

    char line[32];

    uint8_t addr_off;
    uint16_t oam_addr;
    uint16_t ppu_addr;

    uint16_t latest_exec;
    uint8_t is_nmi_works;

    uint8_t ppu_mask;

    uint16_t width;
    uint16_t height;

    void *_render_data;
    void *_window_data;
    uint32_t is_return;

    uint32_t scale;
    uint32_t is_debug_exit;

    uint8_t ctrl[0x100];
    uint8_t ram[RAM_MAX];
    uint8_t oam[0x100];
    uint8_t *ppu;
    uint8_t *ppu_copy;
    uint8_t *ppu_scroll;
    uint8_t *mem;
    uint8_t *chr;

    uint32_t scanline;
    uint64_t timestamp_scanline;
    uint64_t timestamp_cycles;
    uint32_t is_returned_from_nmi;
    uint32_t counter_for_nmi;
    uint32_t cur_cycles;
    uint32_t cur_scanline_cycles;
    uint32_t work_cycles;
    uint32_t is_new_palette_background;
    uint8_t joy0;
    uint32_t new_state;
    uint8_t state_buttons0;
    uint32_t is_new_state;
    uint8_t ppu_status;
    uint8_t cnt_write_scrollxy;
    uint8_t cnt_read_scrollxy;
    uint8_t offx;
    uint8_t offy;
    uint16_t indx_scroll_linex;
    uint8_t is_ready_to_vertical_blank;
    uint32_t vblank_scanline_cycles;

    uint32_t cycles_to_scanline;
    uint8_t scroll_x[280];
    uint8_t scroll_tile_x[280];
    uint32_t max_scroll_indx;

    uint32_t counter;
    uint32_t cnt_debug;

    struct breakpoint brk[0x100];
    uint32_t debug_brk_cnt;

    struct breakwrite bwr[0x100];
    uint32_t debug_bwr_cnt;
    uint32_t is_debug_bwr;

    uint32_t is_started;
    uint32_t is_debug;

    uint32_t debug_step;

    char buf_regs[256];

    uint32_t latest_step;
    uint32_t only_show;

    uint32_t skeep_cnt;
    uint32_t skeep_trace;
};

enum {
	LATEST_NO,
	LATEST_CNT,
	LATEST_STEP,
	LATEST_TRACE,
	N_LATEST
};

#define CYCLES_TO_SCANLINE        12
#define DELAY_CYCLES             0x1
#define DELAY_SCANLINE           0x2

void nes_get_colors_background_clear (struct NESEmu *emu, float *r, float *g, float *b);

void nes_emu_init (struct NESEmu *emu, uint8_t *buffer, uint32_t sz);
void nes_emu_rescale (struct NESEmu *emu, uint32_t scale);
void nes_emu_execute (struct NESEmu *emu, uint32_t count_instructions, void *_data);
void nes_write_state (struct NESEmu *emu);
void nes_platform_init (struct NESEmu *emu, void *data);
void nes_get_rom (const char *filename, uint8_t **data, uint64_t *filesize);
uint32_t nes_event (struct NESEmu *emu, void *_data);
void nes_init_surface (struct NESEmu *emu);

#ifdef __cplusplus
}
#endif
#endif // CPUNES_H
