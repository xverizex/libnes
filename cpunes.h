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
    uint8_t S;
    uint8_t P;
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

    uint8_t mem[0x10000];
};

struct NESCallbacks {
	uint32_t (*init) (struct NESEmu *emu, void *_other_data);
};

void nes_emu_init (struct NESEmu *emu, uint8_t *buffer, uint32_t sz, struct NESCallbacks *clbk);
void nes_emu_execute (struct NESEmu *emu, uint32_t count_instructions);

#endif // CPUNES_H
