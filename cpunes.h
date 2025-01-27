#ifndef CPUNES_H
#define CPUNES_H
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

enum {
	PPUCTRL = 0x2000,
	PPUMASK,
	PPUSTATUS,
	OAMADDR,
	OAMDATA,
	PPUSCROLL,
	PPUADDR,
	PPUDATA,
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
	N_FORMAT
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
    uint16_t S;
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

    float last_cycles_float;
    uint64_t last_cycles_int64;
    uint64_t tmp_last_cycles_int64;
    float capasity_time;

    uint64_t start_time_nmi;

    uint8_t is_debug_list;
    uint8_t is_show_address;
    uint8_t is_show_bytes;
    uint8_t is_debug;

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
    uint32_t is_return;

    struct NESCallbacks *cb;

    uint32_t scale;
    uint32_t is_debug_exit;

    uint8_t ctrl[0x100];
    uint8_t stack[0x300];
    uint8_t ram[RAM_MAX];
    uint8_t oam[0x800];
    uint8_t ppu[0x10000];
    uint8_t mem[0x8000];
    uint8_t chr[0x2000];
};

void nes_get_colors_background_clear (struct NESEmu *emu, float *r, float *g, float *b);

void nes_emu_init (struct NESEmu *emu, uint8_t *buffer, uint32_t sz, struct NESCallbacks *clbk);
void nes_emu_rescale (struct NESEmu *emu, uint32_t scale);
void nes_emu_execute (struct NESEmu *emu, uint32_t count_instructions, void *_data);

#endif // CPUNES_H
