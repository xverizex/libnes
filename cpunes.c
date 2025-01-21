#include <cpunes.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static uint32_t is_init_global_func;

typedef void (*handler_opcode) (struct NESEmu *emu);

#define DEFINE_STATIC_FUNC_NES_HANDLER() \
	static handler_opcode nes_handler[256] = {

#define END_DEFINE_STATIC_FUNC() \
	};

#define ADD_HANDLER(func) \
	func,

#define DEFINE_STATIC_PPU_HANDLER() \
	static ppu_manager ppu_handler[N_PPUMANAGE] = {

#define END_DEFINE_STATIC_PPU_HANDLER() \
	};

void invalid_opcode (struct NESEmu *);
void brk_implied (struct NESEmu *);
void ora_indirect_x (struct NESEmu *);
void ora_zeropage (struct NESEmu *);
void asl_zeropage (struct NESEmu *);
void php_implied (struct NESEmu *);
void ora_immediate (struct NESEmu *);
void asl_accumulator (struct NESEmu *);
void ora_absolute (struct NESEmu *);
void asl_absolute (struct NESEmu *);
void bpl_relative (struct NESEmu *);
void ora_indirect_y (struct NESEmu *);
void ora_zeropage_x (struct NESEmu *);
void asl_zeropage_x (struct NESEmu *);
void clc_implied (struct NESEmu *);
void ora_absolute_y (struct NESEmu *);
void ora_absolute_x (struct NESEmu *);
void asl_absolute_x (struct NESEmu *);
void jsr_absolute (struct NESEmu *);
void and_indirect_x (struct NESEmu *);
void bit_zeropage (struct NESEmu *);
void and_zeropage (struct NESEmu *);
void rol_zeropage (struct NESEmu *);
void plp_implied (struct NESEmu *);
void and_immediate (struct NESEmu *);
void rol_accumulator (struct NESEmu *);
void bit_absolute (struct NESEmu *);
void and_absolute (struct NESEmu *);
void rol_absolute (struct NESEmu *);
void bmi_relative (struct NESEmu *);
void and_indirect_y (struct NESEmu *);
void and_zeropage_x (struct NESEmu *);
void rol_zeropage_x (struct NESEmu *);
void sec_implied (struct NESEmu *);
void and_absolute_y (struct NESEmu *);
void and_absolute_x (struct NESEmu *);
void rol_absolute_x (struct NESEmu *);
void rti_implied (struct NESEmu *);
void eor_indirect_x (struct NESEmu *);
void eor_zeropage (struct NESEmu *);
void lsr_zeropage (struct NESEmu *);
void pha_implied (struct NESEmu *);
void eor_immediate (struct NESEmu *);
void lsr_accumulator (struct NESEmu *);
void jmp_absolute (struct NESEmu *);
void eor_absolute (struct NESEmu *);
void lsr_absolute (struct NESEmu *);
void bvc_relative (struct NESEmu *);
void eor_indirect_y (struct NESEmu *);
void eor_zeropage_x (struct NESEmu *);
void lsr_zeropage_x (struct NESEmu *);
void cli_implied (struct NESEmu *);
void eor_absolute_y (struct NESEmu *);
void eor_absolute_x (struct NESEmu *);
void lsr_absolute_x (struct NESEmu *);
void rts_implied (struct NESEmu *);
void adc_indirect_x (struct NESEmu *);
void adc_zeropage (struct NESEmu *);
void ror_zeropage (struct NESEmu *);
void pla_implied (struct NESEmu *);
void adc_immediate (struct NESEmu *);
void ror_accumulator (struct NESEmu *);
void jmp_indirect (struct NESEmu *);
void adc_absolute (struct NESEmu *);
void ror_absolute (struct NESEmu *);
void bvs_relative (struct NESEmu *);
void adc_indirect_y (struct NESEmu *);
void adc_zeropage_x (struct NESEmu *);
void ror_zeropage_x (struct NESEmu *);
void sei_implied (struct NESEmu *);
void adc_absolute_y (struct NESEmu *);
void adc_absolute_x (struct NESEmu *);
void ror_absolute_x (struct NESEmu *);
void sta_indirect_x (struct NESEmu *);
void sty_zeropage (struct NESEmu *);
void sta_zeropage (struct NESEmu *);
void stx_zeropage (struct NESEmu *);
void dey_implied (struct NESEmu *);
void txa_implied (struct NESEmu *);
void sty_absolute (struct NESEmu *);
void sta_absolute (struct NESEmu *);
void stx_absolute (struct NESEmu *);
void bcc_relative (struct NESEmu *);
void sta_indirect_y (struct NESEmu *);
void sty_zeropage_x (struct NESEmu *);
void sta_zeropage_x (struct NESEmu *);
void stx_zeropage_y (struct NESEmu *);
void tya_implied (struct NESEmu *);
void sta_absolute_y (struct NESEmu *);
void txs_implied (struct NESEmu *);
void sta_absolute_x (struct NESEmu *);
void ldy_immediate (struct NESEmu *);
void lda_indirect_x (struct NESEmu *);
void ldx_immediate (struct NESEmu *);
void ldy_zeropage (struct NESEmu *);
void lda_zeropage (struct NESEmu *);
void ldx_zeropage (struct NESEmu *);
void tay_implied (struct NESEmu *);
void lda_immediate (struct NESEmu *);
void tax_implied (struct NESEmu *);
void ldy_absolute (struct NESEmu *);
void lda_absolute (struct NESEmu *);
void ldx_absolute (struct NESEmu *);
void bcs_relative (struct NESEmu *);
void lda_indirect_y (struct NESEmu *);
void ldy_zeropage_x (struct NESEmu *);
void lda_zeropage_x (struct NESEmu *);
void ldx_zeropage_y (struct NESEmu *);
void clv_implied (struct NESEmu *);
void lda_absolute_y (struct NESEmu *);
void tsx_implied (struct NESEmu *);
void ldy_absolute_x (struct NESEmu *);
void lda_absolute_x (struct NESEmu *);
void ldx_absolute_y (struct NESEmu *);
void cpy_immediate (struct NESEmu *);
void cmp_indirect_x (struct NESEmu *);
void cpy_zeropage (struct NESEmu *);
void cmp_zeropage (struct NESEmu *);
void dec_zeropage (struct NESEmu *);
void iny_implied (struct NESEmu *);
void cmp_immediate (struct NESEmu *);
void dex_implied (struct NESEmu *);
void cpy_absolute (struct NESEmu *);
void cmp_absolute (struct NESEmu *);
void dec_absolute (struct NESEmu *);
void bne_relative (struct NESEmu *);
void cmp_indirect_y (struct NESEmu *);
void cmp_zeropage_x (struct NESEmu *);
void dec_zeropage_x (struct NESEmu *);
void cld_implied (struct NESEmu *);
void cmp_absolute_y (struct NESEmu *);
void cmp_absolute_x (struct NESEmu *);
void dec_absolute_x (struct NESEmu *);
void cpx_immediate (struct NESEmu *);
void sbc_indirect_x (struct NESEmu *);
void cpx_zeropage (struct NESEmu *);
void sbc_zeropage (struct NESEmu *);
void inc_zeropage (struct NESEmu *);
void inx_implied (struct NESEmu *);
void sbc_immediate (struct NESEmu *);
void nop_implied (struct NESEmu *);
void cpx_absolute (struct NESEmu *);
void sbc_absolute (struct NESEmu *);
void inc_absolute (struct NESEmu *);
void beq_relative (struct NESEmu *);
void sbc_indirect_y (struct NESEmu *);
void sbc_zeropage_x (struct NESEmu *);
void inc_zeropage_x (struct NESEmu *);
void sed_implied (struct NESEmu *);
void sbc_absolute_y (struct NESEmu *);
void sbc_absolute_x (struct NESEmu *);
void inc_absolute_x (struct NESEmu *);

void ppu_ctrl (struct NESEmu *emu, uint8_t *r, uint8_t is_write, uint8_t *returned_reg);
void ppu_mask (struct NESEmu *emu, uint8_t *r, uint8_t is_write, uint8_t *returned_reg);
void ppu_status (struct NESEmu *emu, uint8_t *r, uint8_t is_write, uint8_t *returned_reg);
void oam_addr (struct NESEmu *emu, uint8_t *r, uint8_t is_write, uint8_t *returned_reg);
void oam_data (struct NESEmu *emu, uint8_t *r, uint8_t is_write, uint8_t *returned_reg);
void ppu_scroll (struct NESEmu *emu, uint8_t *r, uint8_t is_write, uint8_t *returned_reg);
void ppu_addr (struct NESEmu *emu, uint8_t *r, uint8_t is_write, uint8_t *returned_reg);
void ppu_data (struct NESEmu *emu, uint8_t *r, uint8_t is_write, uint8_t *returned_reg);

static handler_opcode *pnes_handler = NULL;

static void set_dump_format (struct NESEmu *emu)
{
	uint8_t *d = emu->dump;

	if (d[0] == 'N' && d[1] == 'E' && d[2] == 'S' && d[3] == 0x1a) {
		emu->fmt_dump = FORMAT_INES;

		if ((d[7] & 0x0c) == 0x08) {
			emu->fmt_dump = FORMAT_NES20;
		}
	}
}

static void parse_header (struct NESEmu *emu)
{
	uint8_t *d = emu->dump;

	emu->sz_prg_rom = d[4] * 16384;
	emu->sz_chr_rom = d[5] == 0? 8192: d[5] * 8192;
	emu->mapper = (d[6] >> 4) | (d[7] & 0xf0); 
	emu->sz_prg_ram = d[8] * 8192;
}

void nes_emu_init (struct NESEmu *emu, uint8_t *buffer, uint32_t sz, struct NESCallbacks *clbk)
{
	size_t sz_nes_emu = sizeof (struct NESEmu);

	uint32_t dword_size = sz_nes_emu >> 2; /* div by 4 */
	uint32_t *ptr_dword_emu = (uint32_t *) emu;
	for (uint32_t i = 0; i < dword_size; i++) {
		*ptr_dword_emu = 0;
		ptr_dword_emu++;
	}
	uint32_t last_sz = sz_nes_emu & 0x3;
	if (last_sz) {
		uint8_t *last_byte_emu = (uint8_t *) ptr_dword_emu;
		for (uint32_t i = 0; i < last_sz; i++) {
			last_byte_emu[i] = 0;
		}
	}

	emu->cb = clbk;

	emu->dump = buffer;
	emu->sz_dump = sz;

	set_dump_format (emu);
	parse_header (emu);

	emu->cpu.PC = 0xfffc;
	emu->cpu.S = 0xfd;
	emu->cpu.P |= STATUS_FLAG_IF;

	uint16_t pos_handler = 0xa + emu->sz_prg_rom;

	emu->nmi_handler = *(uint16_t *) &buffer[pos_handler];
	emu->reset_handler = *(uint16_t *) &buffer[pos_handler + 2];
	emu->irq_handler = *(uint16_t *) &buffer[pos_handler + 4];

	memcpy (&emu->mem[0x8000], &buffer[16], emu->sz_prg_rom);

	emu->cpu.PC = emu->reset_handler;

	if (!is_init_global_func) {
		DEFINE_STATIC_PPU_HANDLER ()
			ADD_HANDLER (ppu_ctrl)			/* 0x00 */
			ADD_HANDLER (ppu_mask)			/* 0x01 */
			ADD_HANDLER (ppu_status)		/* 0x02 */
			ADD_HANDLER (oam_addr)			/* 0x03 */
			ADD_HANDLER (oam_data)			/* 0x04 */
			ADD_HANDLER (ppu_scroll)		/* 0x05 */
			ADD_HANDLER (ppu_addr)			/* 0x06 */
			ADD_HANDLER (ppu_data)			/* 0x07 */
		END_DEFINE_STATIC_PPU_HANDLER ()

		emu->ppu_handler = ppu_handler;

		DEFINE_STATIC_FUNC_NES_HANDLER ()
			ADD_HANDLER (brk_implied)		/* 0x00 */
			ADD_HANDLER (ora_indirect_x)		/* 0x01 */
			ADD_HANDLER (invalid_opcode)		/* 0x02 */
			ADD_HANDLER (invalid_opcode)		/* 0x03 */
			ADD_HANDLER (invalid_opcode)		/* 0x04 */
			ADD_HANDLER (ora_zeropage)		/* 0x05 */
			ADD_HANDLER (asl_zeropage)		/* 0x06 */
			ADD_HANDLER (invalid_opcode)		/* 0x07 */
			ADD_HANDLER (php_implied)		/* 0x08 */
			ADD_HANDLER (ora_immediate)		/* 0x09 */
			ADD_HANDLER (asl_accumulator)		/* 0x0a */
			ADD_HANDLER (invalid_opcode)		/* 0x0b */
			ADD_HANDLER (invalid_opcode)		/* 0x0c */
			ADD_HANDLER (ora_absolute)		/* 0x0d */
			ADD_HANDLER (asl_absolute)		/* 0x0e */
			ADD_HANDLER (invalid_opcode)		/* 0x0f */
			ADD_HANDLER (bpl_relative)		/* 0x10 */
			ADD_HANDLER (ora_indirect_y)		/* 0x11 */
			ADD_HANDLER (invalid_opcode)		/* 0x12 */
			ADD_HANDLER (invalid_opcode)		/* 0x13 */
			ADD_HANDLER (invalid_opcode)		/* 0x14 */
			ADD_HANDLER (ora_zeropage_x)		/* 0x15 */
			ADD_HANDLER (asl_zeropage_x)		/* 0x16 */
			ADD_HANDLER (invalid_opcode)		/* 0x17 */
			ADD_HANDLER (clc_implied)		/* 0x18 */
			ADD_HANDLER (ora_absolute_y)		/* 0x19 */
			ADD_HANDLER (invalid_opcode)		/* 0x1a */
			ADD_HANDLER (invalid_opcode)		/* 0x1b */
			ADD_HANDLER (invalid_opcode)		/* 0x1c */
			ADD_HANDLER (ora_absolute_x)		/* 0x1d */
			ADD_HANDLER (asl_absolute_x)		/* 0x1e */
			ADD_HANDLER (invalid_opcode)		/* 0x1f */
			ADD_HANDLER (jsr_absolute)		/* 0x20 */
			ADD_HANDLER (and_indirect_x)		/* 0x21 */
			ADD_HANDLER (invalid_opcode)		/* 0x22 */
			ADD_HANDLER (invalid_opcode)		/* 0x23 */
			ADD_HANDLER (bit_zeropage)		/* 0x24 */
			ADD_HANDLER (and_zeropage)		/* 0x25 */
			ADD_HANDLER (rol_zeropage)		/* 0x26 */
			ADD_HANDLER (invalid_opcode)		/* 0x27 */
			ADD_HANDLER (plp_implied)		/* 0x28 */
			ADD_HANDLER (and_immediate)		/* 0x29 */
			ADD_HANDLER (rol_accumulator)		/* 0x2a */
			ADD_HANDLER (invalid_opcode)		/* 0x2b */
			ADD_HANDLER (bit_absolute)		/* 0x2c */
			ADD_HANDLER (and_absolute)		/* 0x2d */
			ADD_HANDLER (rol_absolute)		/* 0x2e */
			ADD_HANDLER (invalid_opcode)		/* 0x2f */
			ADD_HANDLER (bmi_relative)		/* 0x30 */
			ADD_HANDLER (and_indirect_y)		/* 0x31 */
			ADD_HANDLER (invalid_opcode)		/* 0x32 */
			ADD_HANDLER (invalid_opcode)		/* 0x33 */
			ADD_HANDLER (invalid_opcode)		/* 0x34 */
			ADD_HANDLER (and_zeropage_x)		/* 0x35 */
			ADD_HANDLER (rol_zeropage_x)		/* 0x36 */
			ADD_HANDLER (invalid_opcode)		/* 0x37 */
			ADD_HANDLER (sec_implied)		/* 0x38 */
			ADD_HANDLER (and_absolute_y)		/* 0x39 */
			ADD_HANDLER (invalid_opcode)		/* 0x3a */
			ADD_HANDLER (invalid_opcode)		/* 0x3b */
			ADD_HANDLER (invalid_opcode)		/* 0x3c */
			ADD_HANDLER (and_absolute_x)		/* 0x3d */
			ADD_HANDLER (rol_absolute_x)		/* 0x3e */
			ADD_HANDLER (invalid_opcode)		/* 0x3f */
			ADD_HANDLER (rti_implied)		/* 0x40 */
			ADD_HANDLER (eor_indirect_x)		/* 0x41 */
			ADD_HANDLER (invalid_opcode)		/* 0x42 */
			ADD_HANDLER (invalid_opcode)		/* 0x43 */
			ADD_HANDLER (invalid_opcode)		/* 0x44 */
			ADD_HANDLER (eor_zeropage)		/* 0x45 */
			ADD_HANDLER (lsr_zeropage)		/* 0x46 */
			ADD_HANDLER (invalid_opcode)		/* 0x47 */
			ADD_HANDLER (pha_implied)		/* 0x48 */
			ADD_HANDLER (eor_immediate)		/* 0x49 */
			ADD_HANDLER (lsr_accumulator)		/* 0x4a */
			ADD_HANDLER (invalid_opcode)		/* 0x4b */
			ADD_HANDLER (jmp_absolute)		/* 0x4c */
			ADD_HANDLER (eor_absolute)		/* 0x4d */
			ADD_HANDLER (lsr_absolute)		/* 0x4e */
			ADD_HANDLER (invalid_opcode)		/* 0x4f */
			ADD_HANDLER (bvc_relative)		/* 0x50 */
			ADD_HANDLER (eor_indirect_y)		/* 0x51 */
			ADD_HANDLER (invalid_opcode)		/* 0x52 */
			ADD_HANDLER (invalid_opcode)		/* 0x53 */
			ADD_HANDLER (invalid_opcode)		/* 0x54 */
			ADD_HANDLER (eor_zeropage_x)		/* 0x55 */
			ADD_HANDLER (lsr_zeropage_x)		/* 0x56 */
			ADD_HANDLER (invalid_opcode)		/* 0x57 */
			ADD_HANDLER (cli_implied)		/* 0x58 */
			ADD_HANDLER (eor_absolute_y)		/* 0x59 */
			ADD_HANDLER (invalid_opcode)		/* 0x5a */
			ADD_HANDLER (invalid_opcode)		/* 0x5b */
			ADD_HANDLER (invalid_opcode)		/* 0x5c */
			ADD_HANDLER (eor_absolute_x)		/* 0x5d */
			ADD_HANDLER (lsr_absolute_x)		/* 0x5e */
			ADD_HANDLER (invalid_opcode)		/* 0x5f */
			ADD_HANDLER (rts_implied)		/* 0x60 */
			ADD_HANDLER (adc_indirect_x)		/* 0x61 */
			ADD_HANDLER (invalid_opcode)		/* 0x62 */
			ADD_HANDLER (invalid_opcode)		/* 0x63 */
			ADD_HANDLER (invalid_opcode)		/* 0x64 */
			ADD_HANDLER (adc_zeropage_x)		/* 0x65 */
			ADD_HANDLER (ror_zeropage)		/* 0x66 */
			ADD_HANDLER (invalid_opcode)		/* 0x67 */
			ADD_HANDLER (pla_implied)		/* 0x68 */
			ADD_HANDLER (adc_immediate)		/* 0x69 */
			ADD_HANDLER (ror_accumulator)		/* 0x6a */
			ADD_HANDLER (invalid_opcode)		/* 0x6b */
			ADD_HANDLER (jmp_indirect)		/* 0x6c */
			ADD_HANDLER (adc_absolute)		/* 0x6d */
			ADD_HANDLER (ror_absolute)		/* 0x6e */
			ADD_HANDLER (invalid_opcode)		/* 0x6f */
			ADD_HANDLER (bvs_relative)		/* 0x70 */
			ADD_HANDLER (adc_indirect_y)		/* 0x71 */
			ADD_HANDLER (invalid_opcode)		/* 0x72 */
			ADD_HANDLER (invalid_opcode)		/* 0x73 */
			ADD_HANDLER (invalid_opcode)		/* 0x74 */
			ADD_HANDLER (adc_zeropage_x)		/* 0x75 */
			ADD_HANDLER (ror_zeropage_x)		/* 0x76 */
			ADD_HANDLER (invalid_opcode)		/* 0x77 */
			ADD_HANDLER (sei_implied)		/* 0x78 */
			ADD_HANDLER (adc_absolute_y)		/* 0x79 */
			ADD_HANDLER (invalid_opcode)		/* 0x7a */
			ADD_HANDLER (invalid_opcode)		/* 0x7b */
			ADD_HANDLER (invalid_opcode)		/* 0x7c */
			ADD_HANDLER (adc_absolute_x)		/* 0x7d */
			ADD_HANDLER (ror_absolute_x)		/* 0x7e */
			ADD_HANDLER (invalid_opcode)		/* 0x7f */
			ADD_HANDLER (invalid_opcode)		/* 0x80 */
			ADD_HANDLER (sta_indirect_x)		/* 0x81 */
			ADD_HANDLER (invalid_opcode)		/* 0x82 */
			ADD_HANDLER (invalid_opcode)		/* 0x83 */
			ADD_HANDLER (sty_zeropage)		/* 0x84 */
			ADD_HANDLER (sta_zeropage)		/* 0x85 */
			ADD_HANDLER (stx_zeropage)		/* 0x86 */
			ADD_HANDLER (invalid_opcode)		/* 0x87 */
			ADD_HANDLER (dey_implied)		/* 0x88 */
			ADD_HANDLER (invalid_opcode)		/* 0x89 */
			ADD_HANDLER (txa_implied)		/* 0x8a */
			ADD_HANDLER (invalid_opcode)		/* 0x8b */
			ADD_HANDLER (sty_absolute)		/* 0x8c */
			ADD_HANDLER (sta_absolute)		/* 0x8d */
			ADD_HANDLER (stx_absolute)		/* 0x8e */
			ADD_HANDLER (invalid_opcode)		/* 0x8f */
			ADD_HANDLER (bcc_relative)		/* 0x90 */
			ADD_HANDLER (sta_indirect_y)		/* 0x91 */
			ADD_HANDLER (invalid_opcode)		/* 0x92 */
			ADD_HANDLER (invalid_opcode)		/* 0x93 */
			ADD_HANDLER (sty_zeropage_x)		/* 0x94 */
			ADD_HANDLER (sta_zeropage_x)		/* 0x95 */
			ADD_HANDLER (stx_zeropage_y)		/* 0x96 */
			ADD_HANDLER (invalid_opcode)		/* 0x97 */
			ADD_HANDLER (tya_implied)		/* 0x98 */
			ADD_HANDLER (sta_absolute_y)		/* 0x99 */
			ADD_HANDLER (txs_implied)		/* 0x9a */
			ADD_HANDLER (invalid_opcode)		/* 0x9b */
			ADD_HANDLER (invalid_opcode)		/* 0x9c */
			ADD_HANDLER (sta_absolute_x)		/* 0x9d */
			ADD_HANDLER (invalid_opcode)		/* 0x9e */
			ADD_HANDLER (invalid_opcode)		/* 0x9f */
			ADD_HANDLER (ldy_immediate)		/* 0xa0 */
			ADD_HANDLER (lda_indirect_x)		/* 0xa1 */
			ADD_HANDLER (ldx_immediate)		/* 0xa2 */
			ADD_HANDLER (invalid_opcode)		/* 0xa3 */
			ADD_HANDLER (ldy_zeropage)		/* 0xa4 */
			ADD_HANDLER (lda_zeropage)		/* 0xa5 */
			ADD_HANDLER (ldx_zeropage)		/* 0xa6 */
			ADD_HANDLER (invalid_opcode)		/* 0xa7 */
			ADD_HANDLER (tay_implied)		/* 0xa8 */
			ADD_HANDLER (lda_immediate)		/* 0xa9 */
			ADD_HANDLER (tax_implied)		/* 0xaa */
			ADD_HANDLER (invalid_opcode)		/* 0xab */
			ADD_HANDLER (ldy_absolute)		/* 0xac */
			ADD_HANDLER (lda_absolute)		/* 0xad */
			ADD_HANDLER (ldx_absolute)		/* 0xae */
			ADD_HANDLER (invalid_opcode)		/* 0xaf */
			ADD_HANDLER (bcs_relative)		/* 0xb0 */
			ADD_HANDLER (lda_indirect_y)		/* 0xb1 */
			ADD_HANDLER (invalid_opcode)		/* 0xb2 */
			ADD_HANDLER (invalid_opcode)		/* 0xb3 */
			ADD_HANDLER (ldy_zeropage_x)		/* 0xb4 */
			ADD_HANDLER (lda_zeropage_x)		/* 0xb5 */
			ADD_HANDLER (ldx_zeropage_y)		/* 0xb6 */
			ADD_HANDLER (invalid_opcode)		/* 0xb7 */
			ADD_HANDLER (clv_implied)		/* 0xb8 */
			ADD_HANDLER (lda_absolute_y)		/* 0xb9 */
			ADD_HANDLER (tsx_implied)		/* 0xba */
			ADD_HANDLER (invalid_opcode)		/* 0xbb */
			ADD_HANDLER (ldy_absolute_x)		/* 0xbc */
			ADD_HANDLER (lda_absolute_x)		/* 0xbd */
			ADD_HANDLER (ldx_absolute_y)		/* 0xbe */
			ADD_HANDLER (invalid_opcode)		/* 0xbf */
			ADD_HANDLER (cpy_immediate)		/* 0xc0 */
			ADD_HANDLER (cmp_indirect_x)		/* 0xc1 */
			ADD_HANDLER (invalid_opcode)		/* 0xc2 */
			ADD_HANDLER (invalid_opcode)		/* 0xc3 */
			ADD_HANDLER (cpy_zeropage)		/* 0xc4 */
			ADD_HANDLER (cmp_zeropage)		/* 0xc5 */
			ADD_HANDLER (dec_zeropage)		/* 0xc6 */
			ADD_HANDLER (invalid_opcode)		/* 0xc7 */
			ADD_HANDLER (iny_implied)		/* 0xc8 */
			ADD_HANDLER (cmp_immediate)		/* 0xc9 */
			ADD_HANDLER (dex_implied)		/* 0xca */
			ADD_HANDLER (invalid_opcode)		/* 0xcb */
			ADD_HANDLER (cpy_absolute)		/* 0xcc */
			ADD_HANDLER (cmp_absolute)		/* 0xcd */
			ADD_HANDLER (dec_absolute)		/* 0xce */
			ADD_HANDLER (invalid_opcode)		/* 0xcf */
			ADD_HANDLER (bne_relative)		/* 0xd0 */
			ADD_HANDLER (cmp_indirect_y)		/* 0xd1 */
			ADD_HANDLER (invalid_opcode)		/* 0xd2 */
			ADD_HANDLER (invalid_opcode)		/* 0xd3 */
			ADD_HANDLER (invalid_opcode)		/* 0xd4 */
			ADD_HANDLER (cmp_zeropage_x)		/* 0xd5 */
			ADD_HANDLER (dec_zeropage_x)		/* 0xd6 */
			ADD_HANDLER (invalid_opcode)		/* 0xd7 */
			ADD_HANDLER (cld_implied)		/* 0xd8 */
			ADD_HANDLER (cmp_absolute_y)		/* 0xd9 */
			ADD_HANDLER (invalid_opcode)		/* 0xda */
			ADD_HANDLER (invalid_opcode)		/* 0xdb */
			ADD_HANDLER (invalid_opcode)		/* 0xdc */
			ADD_HANDLER (cmp_absolute_x)		/* 0xdd */
			ADD_HANDLER (dec_absolute_x)		/* 0xde */
			ADD_HANDLER (invalid_opcode)		/* 0xdf */
			ADD_HANDLER (cpx_immediate)		/* 0xe0 */
			ADD_HANDLER (sbc_indirect_x)		/* 0xe1 */
			ADD_HANDLER (invalid_opcode)		/* 0xe2 */
			ADD_HANDLER (invalid_opcode)		/* 0xe3 */
			ADD_HANDLER (cpx_zeropage)		/* 0xe4 */
			ADD_HANDLER (sbc_zeropage)		/* 0xe5 */
			ADD_HANDLER (inc_zeropage)		/* 0xe6 */
			ADD_HANDLER (invalid_opcode)		/* 0xe7 */
			ADD_HANDLER (inx_implied)		/* 0xe8 */
			ADD_HANDLER (sbc_immediate)		/* 0xe9 */
			ADD_HANDLER (nop_implied)		/* 0xea */
			ADD_HANDLER (invalid_opcode)		/* 0xeb */
			ADD_HANDLER (cpx_absolute)		/* 0xec */
			ADD_HANDLER (sbc_absolute)		/* 0xed */
			ADD_HANDLER (inc_absolute)		/* 0xee */
			ADD_HANDLER (invalid_opcode)		/* 0xef */
			ADD_HANDLER (beq_relative)		/* 0xf0 */
			ADD_HANDLER (sbc_indirect_y)		/* 0xf1 */
			ADD_HANDLER (invalid_opcode)		/* 0xf2 */
			ADD_HANDLER (invalid_opcode)		/* 0xf3 */
			ADD_HANDLER (invalid_opcode)		/* 0xf4 */
			ADD_HANDLER (sbc_zeropage_x)		/* 0xf5 */
			ADD_HANDLER (inc_zeropage_x)		/* 0xf6 */
			ADD_HANDLER (invalid_opcode)		/* 0xf7 */
			ADD_HANDLER (sed_implied)		/* 0xf8 */
			ADD_HANDLER (sbc_absolute_y)		/* 0xf9 */
			ADD_HANDLER (invalid_opcode)		/* 0xfa */
			ADD_HANDLER (invalid_opcode)		/* 0xfb */
			ADD_HANDLER (invalid_opcode)		/* 0xfc */
			ADD_HANDLER (sbc_absolute_x)		/* 0xfd */
			ADD_HANDLER (inc_absolute_x)		/* 0xfe */
			ADD_HANDLER (invalid_opcode)		/* 0xff */
		END_DEFINE_STATIC_FUNC ()

		pnes_handler = nes_handler;

		is_init_global_func = 1;
	}
}

void nes_emu_execute (struct NESEmu *emu, uint32_t count_instructions)
{
	if (emu->is_debug_list) {
		uint16_t tmp_pc = emu->cpu.PC;
		emu->cpu.PC = 0x8000;

    		for (uint16_t off = 0x8000; off < 0xffff;) {

			pnes_handler [emu->mem[emu->cpu.PC]] (emu);

			if (off == emu->cpu.PC)
				break;

			if (emu->cb && emu->cb->print_debug) {
				emu->cb->print_debug (emu, NULL);
			}

			off = emu->cpu.PC;
    		}

		emu->cpu.PC = tmp_pc;
		return;
	}

	if (emu->cb->calc_time_uint64) {
		emu->cb->calc_time_uint64 (emu, NULL);
		if (emu->last_cycles_int64 > 0) {
			return;
		}
		emu->last_cycles_int64 = 0;
	}

	pnes_handler [emu->mem[emu->cpu.PC]] (emu);
}
