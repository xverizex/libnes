#include <cpunes.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <handlers.h>

static uint32_t is_init_global_func;

typedef void (*handler_opcode) (struct NESEmu *emu);
static handler_opcode *pnes_handler;

#define DEFINE_STATIC_STRUCT_NES_HANDLER() \
	static handler_opcode nes_handler[256] = {

#define END_DEFINE_STATIC_STRUCT() \
	};

#define ADD_HANDLER(func) \
	func,

static handler_opcode *pnes_handler = NULL;

static void set_dump_format (struct NESEmu *emu)
{
	uint8_t *d = emu->dump;
	if (d[0] == 'N' && d[1] == 'E' && d[2] == 'S' && d[3] == 0x1a) {
		emu->fmt_dump = FORMAT_INES;
		if ((d[7] & 0x0c) == 0x08) {
			emu->fmt_dump = FORMAT_NES20;
			printf ("FORMAT NES20\n");
		} else {
			printf ("FORMAT INES\n");
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

	printf ("size prg rom: %d bytes\n", emu->sz_prg_rom);
	printf ("size chr rom: %d bytes\n", emu->sz_chr_rom);
	printf ("size prg ram: %d\n", emu->sz_prg_ram);
	printf ("mapper: %d\n", emu->mapper);
}

#include <stdio.h>
#include <stdlib.h>

void platform_init (struct NESEmu *emu, void *data);
void platform_alloc_memory_map (struct NESEmu *emu);

void nes_emu_init (struct NESEmu *emu, uint8_t *data, uint32_t sz_file)
{
	size_t sz_nes_emu = sizeof (struct NESEmu);
	memset (emu, 0, sz_nes_emu);

	emu->dump = data;
	emu->sz_dump = sz_file;
	set_dump_format (emu);
	parse_header (emu);
	emu->cpu.PC = 0x8000;
	emu->cpu.S = 0xff;
	emu->cpu.P |= STATUS_FLAG_IF;
	emu->counter_for_nmi = 0;
	emu->cur_cycles = 0;

	emu->nmi_handler = *(uint16_t *) &data[0x10 + emu->sz_prg_rom - 6];
	emu->reset_handler = *(uint16_t *) &data[0x10 + emu->sz_prg_rom - 4];
	emu->irq_handler = *(uint16_t *) &data[0x10 + emu->sz_prg_rom - 2];

	printf ("nmi: %04x\n", emu->nmi_handler);
	printf ("reset: %04x\n", emu->reset_handler);
	printf ("irq: %04x\n", emu->irq_handler);

	emu->cpu.PC = emu->reset_handler;

	emu->timestamp_cycles = 0;
	emu->last_cycles_int64 = 0;

	emu->scale = 4;
	emu->width = 256;
	emu->height = 224;

	emu->new_state = 0;
	emu->is_new_palette_background = 1;

	platform_alloc_memory_map (emu);

	memcpy (emu->mem, &data[0x10], emu->sz_prg_rom);
	memcpy (emu->chr, &data[0x10 + emu->sz_prg_rom], 0x2000);

	if (!is_init_global_func) {
		DEFINE_STATIC_STRUCT_NES_HANDLER ()
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
			ADD_HANDLER (adc_zeropage)		/* 0x65 */
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
		END_DEFINE_STATIC_STRUCT ()

		pnes_handler = nes_handler;
	}

	platform_init (emu, NULL);
}

int platform_delay (struct NESEmu *emu, void *_other_data);
uint32_t platform_delay_nmi (struct NESEmu *emu, void *_data);
void platform_render (struct NESEmu *emu, void *data);

static void debug (struct NESEmu *emu, uint32_t cnt)
{
	uint16_t addr = 0;

	printf ("\n%04x:", addr);
	for (int i = 0; i < cnt; i++) {
		if (i > 0 && i % 16 == 0) {
			addr += 16;
			printf ("\n%04x:", addr);
		}
		printf ("%02x ", emu->ram[i]);
	}
	printf ("\n");
}

void nes_emu_execute (struct NESEmu *emu, uint32_t count_instructions, void *_data)
{

	for (int i = 0; i < count_instructions; i++) {

		if (emu->is_nmi_works) {
			while (platform_delay (emu, NULL));
		} else {
			while (platform_delay (emu, NULL));
		}

		uint16_t pc = emu->cpu.PC;

#if 0
		if (!(emu->ctrl[REAL_PPUCTRL] & PPUCTRL_VBLANK_NMI)) {
			emu->counter_for_nmi = 0;
			emu->cur_cycles = 0;
			//emu->last_cycles_int64 = 0;
		}
#endif

		if (emu->is_nmi_works) {
		} else if (emu->ctrl[REAL_PPUCTRL] & PPUCTRL_VBLANK_NMI) {
			if (platform_delay_nmi (emu, NULL)) {
				uint16_t addr;
				addr = 0x100 + --emu->cpu.S;
				emu->ram[addr] = ( emu->cpu.PC >> 8 ) & 0xff;
				addr = 0x100 + --emu->cpu.S;
				emu->ram[addr] = ((emu->cpu.PC) & 0xff);
				addr = 0x100 + --emu->cpu.S;
				emu->ram[addr] = emu->cpu.P;

				emu->cpu.PC = emu->nmi_handler;
				emu->is_nmi_works = 1;
			}
		}


		//printf ("pc: %04x\n", pc);

		static uint32_t runs = 0;
		static uint32_t tick = 0;
		if (!emu->is_nmi_works) {
			static uint32_t cnt = 0;
#if 0
			if (emu->cpu.PC == 0xc783) {
				cnt++;
				//printf ("cnt: %d\n", cnt);
				//debug (emu, 0xf0);
			}
#endif
			//debug (emu, 0xe0);
#if 0
			if (pc == 0xc7b0 && emu->cpu.X == 0x38) {
			//if (emu->is_debug_exit) {
				runs = 1;
				debug (emu, 0xf0);
				printf ("A: %02x X: %02x Y: %02x P: %02x PC: %04x; tick: %d\n",
						emu->cpu.A,
						emu->cpu.X,
						emu->cpu.Y,
						emu->cpu.P,
						pc,
						tick
						);
				getc (stdin);
				emu->is_debug_exit = 0;
			} else if (runs) {
				debug (emu, 0xf0);
				printf ("A: %02x X: %02x Y: %02x P: %02x PC: %04x; tick: %d\n",
						emu->cpu.A,
						emu->cpu.X,
						emu->cpu.Y,
						emu->cpu.P,
						pc,
						tick
						);
				getc (stdin);
			}
#endif
		}

		pnes_handler [emu->mem[emu->cpu.PC - 0x8000]] (emu);
		if (emu->is_debug_exit) {
			printf ("###### debug exit error #######\n");
			printf ("A: %02x X: %02x Y: %02x P: %02x PC: %04x; bytes: %02x %02x %02x\n",
					emu->cpu.A,
					emu->cpu.X,
					emu->cpu.Y,
					emu->cpu.P,
					pc,
					emu->mem[pc - 0x8000 + 0],
					emu->mem[pc - 0x8000 + 1],
					emu->mem[pc - 0x8000 + 2]
					);
			exit (0);
		}

		if (emu->is_returned_from_nmi) {
			platform_render (emu, _data);
			emu->is_returned_from_nmi = 0;
			emu->is_nmi_works = 0;
			emu->last_cycles_int64 = 0;
			emu->start_time_nmi = 0;
		}
	}
}

void nes_write_state (struct NESEmu *emu)
{
	emu->joy0 = emu->state_buttons0;
	emu->new_state = 0;
}
