#include <stdint.h>
#include <stddef.h>
#include <cpunes.h>
#include <stdio.h>
#include <time.h>

void platform_ppu_mask (struct NESEmu *emu, void *_other_data);

void wait_cycles (struct NESEmu *emu, uint32_t cycles)
{
	//emu->last_cycles_float = (float) cycles * 0.000558730074f;
	//emu->last_cycles_int64 = 16L;
	emu->last_cycles_int64 = cycles * 559L;
	emu->cur_cycles = cycles;
}

void check_flags_ld (struct CPUNes *cpu, uint8_t flags, uint8_t reg)
{
	if (flags & STATUS_FLAG_NF) {
		if (reg & 0x80)
			cpu->P |= STATUS_FLAG_NF;
	}
	if (flags & STATUS_FLAG_ZF) {
		if (reg == 0) {
			cpu->P |= STATUS_FLAG_ZF;
		}
	}
}

static inline void set_ext_cycles (struct CPUNes *cpu, int8_t offset, uint16_t new_offset, uint32_t *ext_cycles)
{
	*ext_cycles = 1;

	if (offset < 0) {
		uint8_t c = cpu->PC;
		uint8_t n = new_offset;
		
		if (c < n) {
			*ext_cycles = 2;
		}
	} else if (offset > 0) {
		uint8_t c = cpu->PC;
		uint8_t n = new_offset;
		
		if (c > n) {
			*ext_cycles = 2;
		}
	}
}

void read_from_address (struct NESEmu *emu, uint16_t addr, uint8_t *r)
{
	//printf ("\tread from addr: %04x = %02x\n", addr, *r);
	if (addr == PPUSTATUS) {
		emu->addr_off = 0;
		emu->ppu_addr = 0;
		*r = 0x80;
		//printf ("%04x <- pc\n", emu->cpu.PC);
		return;
	}
	if (addr < RAM_MAX) {
		*r = emu->ram[addr];
	} else if (addr == 0x2007) {
		// TODO: what is return ppu data?
		if (emu->ppu_addr >= 0x4000) {
			printf ("ppu read exit; %04x\n", emu->ppu_addr);
			emu->is_debug_exit = 1;
		}
		*r = emu->ppu[emu->ppu_addr];
	} else if (addr >= 0x4016 && addr <= 0x4017) {
		*r = 0x40; // JOYS
	} else {
		*r = emu->mem[addr - 0x8000];
	}
}

#include <stdlib.h>
void write_to_address (struct NESEmu *emu, uint16_t addr, uint8_t *r)
{
	//printf ("\twrite to addr: %04x = %02x\n", addr, *r);

	if (addr == OAMADDR) {
		emu->oam_addr = 0;
		emu->oam_addr |= *r;
		return;
	}
	if (addr == OAMDMA) {
		emu->oam_addr |= (*r << 8) & 0xff00;
		return;
	}

	if (addr < RAM_MAX) {
		if (emu->oam_addr >= RAM_MAX) {
			printf ("emu->oam_addr is overflow: %04x\n", emu->oam_addr);
			emu->is_debug_exit = 1;
			return;
		}
		if (((emu->oam_addr >= 0x200) && (emu->oam_addr <= 0x2ff)) && ((addr >= 0x200) && (addr <= 0x2ff))) {
#if 0
			if (((addr == 0x243)&& (*r == 196))) {
				printf (">> PC = %04x; A: %02x X: %02x Y: %02x P: %02x addr: %04x; *r=%02x\n", 
						emu->cpu.PC, emu->cpu.A, emu->cpu.X, emu->cpu.Y, emu->cpu.P, addr, *r);
				emu->is_debug_exit = 1;
				//exit (0);
			} else if (addr == 0x243) {
				static int cnt = 0;
				printf ("cnt: %d; *r %02x %d\n", cnt++, *r, *r);
			}
#endif
			emu->oam[addr - 0x200] = *r;
		} else {
#if 0
			if (addr == 0xb8 && *r == 0xd1) {
			printf (">> PC = %04x; A: %02x X: %02x Y: %02x P: %02x addr: %04x; *r=%02x\n", emu->cpu.PC, emu->cpu.A, emu->cpu.X, emu->cpu.Y, emu->cpu.P, addr, *r);
				exit (0);
			}
#endif
			emu->ram[addr] = *r;
		}
		return;
	}

	if (addr == PPUCTRL) {
		emu->ctrl[REAL_PPUCTRL] = *r;
#if 0
		if (*r & PPUCTRL_VBLANK_NMI) {
			static const uint16_t addr_palette = 0x3f00;
			uint16_t ix = 0;
			uint32_t indx = 0;
			for (int y = 0; y < 16; y++) {
				if ((emu->palette_image[ix] != emu->ppu[addr_palette + ix])) {
					emu->is_new_palette_background = 1;
				}

				for (int i = 0; i < 16; i += 4) {
					emu->palette_image[i + 0] = emu->ppu[addr_palette + 0 + i];
					emu->palette_image[i + 1] = emu->ppu[addr_palette + 1 + i];
					emu->palette_image[i + 2] = emu->ppu[addr_palette + 2 + i];
					emu->palette_image[i + 3] = emu->ppu[addr_palette + 3 + i];
				}
				if (emu->is_new_palette_background)
					break;
			}
		}
#endif
	} else if (addr == PPUMASK) {
		emu->ctrl[REAL_PPUMASK] = *r;
		if ((*r) & MASK_IS_BACKGROUND_RENDER) {
			emu->is_return = 1;
			platform_ppu_mask (emu, NULL);
		}

	} else if (addr == PPUADDR) {
		if (emu->addr_off == 0) {
			emu->ppu_addr = 0;
			emu->ppu_addr |= ((*r << 8) & 0xff00);
			emu->addr_off++;
		} else {
			emu->ppu_addr |= *r & 0xff;
			emu->addr_off = 0;
		}
	} else if (addr == PPUDATA) {
		if (emu->ppu_addr >= 0x4000) {
			printf ("%04x ppu addr\n", emu->ppu_addr);
			emu->is_debug_exit = 1;
			return;
		}
		if (emu->ppu_addr < 0x2000) {
			printf ("write to ppu %04x = %02x\n", emu->ppu_addr, *r);
			emu->is_debug_exit = 1;
		}
		emu->ppu[emu->ppu_addr++] = *r; //screen on the 0x2000 //TODO: fix
	} 
	if (addr >= PPUCTRL && addr <= PPUDATA) {
		emu->ctrl[addr - 0x2000] = *r;
	} 
	if (addr >= 0x4000 && addr < 0x6000) {
	}
}

// TODO: implement right VF checking
void check_flags (struct CPUNes *cpu, uint8_t flags, uint8_t reg, uint8_t ret)
{
	if (flags & STATUS_FLAG_NF) {
		if (ret & 0x80) {
			cpu->P |= STATUS_FLAG_NF;
		}
	}
	if (flags & STATUS_FLAG_ZF) {
		if (ret == 0) {
			cpu->P |= STATUS_FLAG_ZF;
		}
	}
}

void check_flags_cmp (struct CPUNes *cpu, uint8_t flags, uint8_t reg, uint8_t ret)
{
	if (flags & STATUS_FLAG_NF) {
		if (ret & 0x80) {
			cpu->P |= STATUS_FLAG_NF;
		}
	}
	if (flags & STATUS_FLAG_ZF) {
		if (ret == 0) {
			cpu->P |= STATUS_FLAG_ZF;
		}
	}
}

void eq (uint8_t *r0, uint8_t r1)
{
	*r0 = r1;
}

void void_eq (uint8_t *r0, uint8_t r1)
{
}

void adc_acts (struct NESEmu *emu, uint8_t flags, 
		uint8_t *reg,
		uint8_t result, 
		uint8_t val,
		void (*eq) (uint8_t *r0, uint8_t r1),
		uint16_t cycles_and_bytes)
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t carry = 0;
	if (cpu->P & STATUS_FLAG_CF) carry = 1;
	cpu->P &= ~(flags);
	if (flags & STATUS_FLAG_CF) {
		if ((result < *reg)) {
			cpu->P |= STATUS_FLAG_CF;
		}
	}
	result += carry;
	check_flags (cpu, flags, *reg, result);
	
	if (flags & STATUS_FLAG_CF) {
		if ((result < *reg)) {
			cpu->P |= STATUS_FLAG_CF;
		}
	}

	if (((*reg & 0x80) && (val & 0x80)) && !(result & 0x80))
		cpu->P |= STATUS_FLAG_VF;
	else if ((!(*reg & 0x80) && !(val & 0x80)) && (result & 0x80))
		cpu->P |= STATUS_FLAG_VF;

	eq (reg, result);
	wait_cycles (emu, (cycles_and_bytes >> 8) & 0xff);
	cpu->PC += (cycles_and_bytes & 0xff);
}

void cmp_act (struct NESEmu *emu, 
		uint8_t flags, 
		uint8_t *reg,
		uint8_t result, 
		uint8_t val,
		uint16_t cycles_and_bytes)
{
	struct CPUNes *cpu = &emu->cpu;
	cpu->P &= ~(flags);
	check_flags_cmp (cpu, flags, *reg, result);
	if (*reg >= val)
		cpu->P |= STATUS_FLAG_CF;
	wait_cycles (emu, cycles_and_bytes >> 8);
	emu->cpu.PC += (cycles_and_bytes & 0xff);
}

void repetitive_acts (struct NESEmu *emu, 
		uint8_t flags, 
		uint8_t *reg,
		uint8_t result, 
		void (*eq) (uint8_t *r0, uint8_t r1),
		uint16_t cycles_and_bytes)
{
	struct CPUNes *cpu = &emu->cpu;
	cpu->P &= ~(flags);
	check_flags (cpu, flags, *reg, result);
	eq (reg, result);
	wait_cycles (emu, cycles_and_bytes >> 8);
	emu->cpu.PC += (cycles_and_bytes & 0xff);
}

void asl_acts (struct NESEmu *emu, uint8_t flags, uint8_t *reg, uint16_t cycles_and_bytes)
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t ret = 0;
	uint8_t bt = *reg;
	cpu->P &= ~(flags);
	if (bt & 0x80) {
		cpu->P |= STATUS_FLAG_CF;
	} 
	if (bt & 0x40) {
		cpu->P |= STATUS_FLAG_NF;
	}
	bt <<= 1;
	if (bt == 0x0) {
		cpu->P |= STATUS_FLAG_ZF;
	}
	*reg = bt;
	wait_cycles (emu, cycles_and_bytes >> 8);
	emu->cpu.PC += (cycles_and_bytes & 0xff);
}

void lsr_acts (struct NESEmu *emu, uint8_t flags, uint8_t *mem, uint16_t cycles_and_bytes)
{
	struct CPUNes *cpu = &emu->cpu; 
	uint8_t ret = 0;
	uint8_t bt = *mem;
	cpu->P &= ~(flags);
	if (bt & 0x01) {
		cpu->P |= STATUS_FLAG_CF;
	} 
	bt >>= 1;
	if (bt == 0) {
		cpu->P |= STATUS_FLAG_ZF;
	}
	*mem = bt;
	wait_cycles (emu, cycles_and_bytes >> 8);
	emu->cpu.PC += (cycles_and_bytes & 0xff);
	// TODO: bt == 0?
}

void rol_acts (struct NESEmu *emu, uint8_t flags, uint8_t *mem, uint16_t cycles_and_bytes)
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t bt = *mem;
	uint8_t old_flag = cpu->P;
	cpu->P &= ~(flags);
	if (bt & 0x80) {
		cpu->P |= STATUS_FLAG_CF;
	} 
	if (bt & 0x40) {
		cpu->P |= STATUS_FLAG_NF;
	}
	bt <<= 1;
	if (old_flag & STATUS_FLAG_CF) {
		bt |= 0x01;
	}
	if (bt == 0x0) {
		cpu->P |= STATUS_FLAG_ZF;
	}
	*mem = bt;
	wait_cycles (emu, cycles_and_bytes >> 8);
	emu->cpu.PC += cycles_and_bytes & 0xff;
}

void ror_acts (struct NESEmu *emu, uint8_t flags, uint8_t *mem, uint16_t cycles_and_bytes)
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t bt = *mem;
	uint8_t old_flag = cpu->P;
	cpu->P &= ~(flags);
	if (bt & 0x01) {
		cpu->P |= STATUS_FLAG_CF;
	} 
	bt >>= 1;
	if (old_flag & STATUS_FLAG_CF) {
		bt |= 0x80;
	}
	if (bt & 0x80) {
		cpu->P |= STATUS_FLAG_NF;
	}
	if (bt == 0x00) {
		cpu->P |= STATUS_FLAG_ZF;
	}
	*mem = bt;
	wait_cycles (emu, cycles_and_bytes >> 8);
	emu->cpu.PC += (cycles_and_bytes & 0xff);
}

void ld_acts (struct NESEmu *emu, uint8_t flags, uint8_t *reg, uint16_t addr, uint16_t cycles_and_bytes)
{
	struct CPUNes *cpu = &emu->cpu;
	read_from_address (emu, addr, reg);
	cpu->P &= ~(flags);
	check_flags_ld (cpu, flags, *reg);
	wait_cycles (emu, cycles_and_bytes >> 8);
	emu->cpu.PC += cycles_and_bytes & 0xff;
}

void st_acts (struct NESEmu *emu, uint8_t *reg, uint16_t addr, uint16_t cycles_and_bytes)
{
	struct CPUNes *cpu = &emu->cpu;
	write_to_address (emu, addr, reg);
	wait_cycles (emu, cycles_and_bytes >> 8);
	emu->cpu.PC += cycles_and_bytes & 0xff;
}

void bit_acts (struct NESEmu *emu, uint8_t flags, uint16_t mem, uint16_t cycles_and_bytes)
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t returned_reg = 0;
	read_from_address (emu, mem, &returned_reg);
	cpu->P &= ~(flags);

	if ((cpu->A & returned_reg) == 0)
		cpu->P |= STATUS_FLAG_ZF;

	if ((returned_reg & 0x40))
		cpu->P |= STATUS_FLAG_VF;

	if (returned_reg & 0x80)
		cpu->P |= STATUS_FLAG_NF;

	wait_cycles (emu, cycles_and_bytes >> 8);
	cpu->PC += cycles_and_bytes & 0xff;
}

uint16_t immediate (struct NESEmu *emu);
uint8_t immediate_val (struct NESEmu *emu);
uint16_t absolute (struct NESEmu *emu);
uint8_t zeropage (struct NESEmu *emu);
uint8_t zeropage_x (struct NESEmu *emu);
uint8_t zeropage_y (struct NESEmu *emu);
uint16_t absolute_x (struct NESEmu *emu);
uint16_t absolute_y (struct NESEmu *emu);
uint16_t indirect (struct NESEmu *emu);
uint16_t indirect_x (struct NESEmu *emu);
uint16_t indirect_y (struct NESEmu *emu);

#include <stdlib.h>

uint16_t immediate (struct NESEmu *emu)
{
    return (emu->cpu.PC + 1);
}

uint8_t immediate_val (struct NESEmu *emu)
{
    return emu->mem[(emu->cpu.PC + 1) - 0x8000];
}

uint16_t absolute (struct NESEmu *emu)
{
    uint16_t addr = *((uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
    return addr;
}

uint8_t zeropage (struct NESEmu *emu)
{
    uint8_t addr = emu->mem[(emu->cpu.PC + 1) - 0x8000];
    return addr;
}

uint8_t zeropage_x (struct NESEmu *emu)
{
    uint8_t addr = emu->mem[(emu->cpu.PC + 1) - 0x8000];
    addr += emu->cpu.X;
    return addr;
}

uint8_t zeropage_y (struct NESEmu *emu)
{
    uint8_t addr = emu->mem[(emu->cpu.PC + 1) - 0x8000];
    addr += emu->cpu.Y;
    return addr;
}

uint16_t absolute_x (struct NESEmu *emu)
{
    uint16_t addr = *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000];
    addr += emu->cpu.X;
    return addr;
}

uint16_t absolute_y (struct NESEmu *emu)
{
    uint16_t addr = *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000];
    addr += emu->cpu.Y;
    return addr;
}

uint16_t indirect (struct NESEmu *emu)
{
    	uint16_t addr = *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000];

	uint16_t new_addr = 0;
	if (addr < RAM_MAX) {
    		new_addr = *(uint16_t *) &emu->ram[addr + 0];
	} else {
    		new_addr = *(uint16_t *) &emu->mem[(addr + 0) - 0x8000];
	}

	return new_addr;
}

uint16_t indirect_x (struct NESEmu *emu)
{
	uint8_t addr = emu->mem[(emu->cpu.PC + 1) - 0x8000];
	uint16_t indirect = 0;

	indirect = *((uint16_t *) &emu->ram[addr + emu->cpu.X]);

	return indirect;
}

uint16_t indirect_y (struct NESEmu *emu)
{
	uint8_t zeroaddr = emu->mem[(emu->cpu.PC + 1) - 0x8000];
	uint16_t addr = 0;

	addr = *((uint16_t *) &emu->ram[zeroaddr]);

	return addr + emu->cpu.Y;
}

#include <stdlib.h>
void invalid_opcode (struct NESEmu *emu) 
{
	printf ("\tinvalid opcode: A: %02x X: %02x Y: %02x P: %02x S: %04x PC: %04x OPCODE: %02x\n",
			emu->cpu.A,
			emu->cpu.X,
			emu->cpu.Y,
			emu->cpu.P,
			emu->cpu.S,
			emu->cpu.PC,
			emu->mem[emu->cpu.PC - 0x8000]
	       );
	emu->is_debug_exit = 1;
	//emu->cpu.PC++;
}

#include <stdlib.h>
void brk_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

#if 0
	emu->stack[--cpu->S] = (uint8_t) (cpu->PC & 0xff);
	emu->stack[--cpu->S] = (uint8_t) ((cpu->PC >> 8) & 0xff);
	emu->stack[--cpu->S] = cpu->P;

	cpu->P |= (STATUS_FLAG_IF);

	cpu->PC = emu->irq_handler;
#endif

	wait_cycles (emu, 7);
	printf ("brk\n");
	emu->is_debug_exit = 1;
}

void ora_indirect_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	uint16_t addr = indirect_x (emu);

	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A | val,
			eq,
			(6 << 8) | 2);
}

void ora_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	uint8_t addr = zeropage (emu);

	uint8_t val = emu->ram[addr];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A | val,
			eq,
			(3 << 8) | 2);
}

void asl_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);

	asl_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&emu->ram[addr],
			(5 << 8) | 2);
}

void php_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	uint16_t addr = 0x100 + --cpu->S;
	emu->ram[addr] = cpu->P;

	wait_cycles (emu, 3);

	cpu->PC++;
}

void ora_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t val = immediate_val (emu);

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A | val,
			eq,
			(2 << 8) | 2);
}
void asl_accumulator (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	asl_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->A,
			(2 << 8) | 1);
}

void ora_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A | val,
			eq,
			(4 << 8) | 3);
}

void asl_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	uint8_t *m;
	uint16_t off;
	if (addr < RAM_MAX) {
		m = emu->ram;
		off = 0;
	} else {
		m = emu->mem;
		off = 0x8000;
	}
	asl_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&m[addr - off],
			(6 << 8) | 3);
}

static uint8_t hex_number_str (uint8_t n)
{
	if (n >= 0 && n <= 9) {
		return n + '0';
	}

	if (n >= 10 && n <= 15) {
		return (n - 10) + 'A';
	}
}

static uint8_t hex_up_half (uint8_t n)
{
	n = n >> 4;

	return hex_number_str (n);
}

static uint8_t hex_down_half (uint8_t n)
{
	n = n & 0xf;

	return hex_number_str (n);
}

static uint8_t *show_debug_info (struct NESEmu *emu, uint32_t count, char *op)
{
	uint32_t tcount = 0;
	struct CPUNes *cpu = &emu->cpu;

	uint8_t *pl = emu->line;
	if (emu->is_show_address) {
		*pl++ = hex_up_half (cpu->PC >> 8);
		*pl++ = hex_down_half (cpu->PC >> 8);
		*pl++ = hex_up_half (cpu->PC & 0xff);
		*pl++ = hex_down_half (cpu->PC & 0xff);
		*pl++ = ':';
		*pl++ = ' ';
	}
	if (emu->is_show_bytes) {
		for (uint16_t i = 0; i < count; i++) {
			tcount++;
			*pl++ = hex_up_half (emu->mem[emu->cpu.PC + i]);
			*pl++ = hex_down_half (emu->mem[emu->cpu.PC + i]);
			*pl++ = ' ';
		}

	}

	while (tcount < 3) {
		*pl++ = ' ';
		*pl++ = ' ';
		*pl++ = ' ';
		tcount++;
	}
	while (*op != 0) {
		*pl++ = *op++;
	}

	*pl++ = ' ' ;

	return pl;
}
#if 0
	/* lst */
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		int8_t offset = emu->mem[cpu->PC + 1];
		uint16_t new_offset = cpu->PC + offset;

		uint8_t *pl = show_debug_info (emu, 2, "BPL");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xf);
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		return;
	}
#endif

#include <stdlib.h>
void bpl_relative (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	int8_t offset = emu->mem[(cpu->PC + 1) - 0x8000];

	uint16_t new_offset;

	if (offset < 0) {
		uint8_t off = 0xff - offset - 1;
		new_offset = cpu->PC - off;
	} else {
		new_offset = cpu->PC + offset + 2;
	}

	uint32_t ext_cycles = 0;

	if (emu->cpu.P & STATUS_FLAG_NF) {
		cpu->PC += 2;
	} else {
		set_ext_cycles (cpu, offset, new_offset, &ext_cycles);

		cpu->PC = new_offset;
	}

	wait_cycles (emu, 2 + ext_cycles);
}

void ora_indirect_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	/* TODO: cross */
	uint16_t addr = indirect_y (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A | val,
			eq,
			(5 << 8) | 2);
}

void ora_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);

	uint8_t val = emu->ram[addr];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A | val,
			eq,
			(4 << 8) | 2);
}

void asl_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = zeropage_x (emu);

	asl_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&emu->ram[addr],
			(6 << 8) | 2);
}
void clc_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->P &= ~(STATUS_FLAG_CF);

	wait_cycles (emu, 2);

	cpu->PC++;
}

void ora_absolute_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	/* TODO: cross */
	uint16_t addr = absolute_y (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A | val,
			eq,
			(4 << 8) | 3);
}

void ora_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_x (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A | val,
			eq,
			(4 << 8) | 3);
}

void asl_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_x (emu);
	uint8_t *m;
	uint16_t off;
	if (addr < RAM_MAX) {
		m = emu->ram;
		off = 0;
	} else {
		m = emu->mem;
		off = 0x8000;
	}

	asl_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&m[addr - off],
			(7 << 8) | 3);
}

void jsr_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	uint16_t pc = cpu->PC + 3;

	pc--;

	uint16_t addr;

       	addr = 0x100 + --cpu->S;
	emu->ram[addr] = (uint8_t) ((pc >> 8) & 0xff);
	addr = 0x100 + --cpu->S;
	emu->ram[addr] = (uint8_t) (pc & 0xff);

	uint16_t new_pc = 0;
	new_pc = *(uint16_t *) &emu->mem[(cpu->PC + 1) - 0x8000];
	cpu->PC = new_pc;
	//printf ("called: %04x\n", new_pc);

	wait_cycles (emu, 6);
}

void and_indirect_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = indirect_x (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A & val,
			eq,
			(6 << 8) | 2);
}

void bit_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);
	bit_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_VF,
			addr,
			(3 << 8) | 2);
}

void and_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = zeropage (emu);

	uint8_t val = emu->ram[addr];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A & val,
			eq,
			(3 << 8) | 2);
}

void rol_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = zeropage (emu);

	rol_acts (emu,
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&emu->ram[addr],
			(5 << 8) | 2);
}

void plp_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	uint16_t addr = 0x100 + cpu->S++;
	cpu->P = emu->ram[addr];

	wait_cycles (emu, 4);

	cpu->PC++;
}



void and_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t val = immediate_val (emu);

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A & val,
			eq,
			(2 << 8) | 2);
}
void rol_accumulator (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	rol_acts (emu,
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->A,
			(2 << 8) | 1);
}

void bit_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	uint16_t addr = absolute (emu);
	bit_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_VF,
			addr,
			(4 << 8) | 3);
}

void and_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A & val,
			eq,
			(4 << 8) | 3);
}

void rol_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	uint16_t off;
	uint8_t *m;
	if (addr < RAM_MAX) {
		m = emu->ram;
		off = 0;
	} else {
		m = emu->mem;
		off = 0x8000;
	}
	rol_acts (emu,
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&m[addr - off],
			(6 << 8) | 3);
}

void bmi_relative (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	int8_t offset = emu->mem[(cpu->PC + 1) - 0x8000];

	uint16_t new_offset;

	if (offset < 0) {
		uint8_t off = 0xff - offset - 1;
		new_offset = cpu->PC - off;
	} else {
		new_offset = cpu->PC + offset + 2;
	}

	uint32_t ext_cycles = 0;

	if (emu->cpu.P & STATUS_FLAG_NF) {
		set_ext_cycles (cpu, offset, new_offset, &ext_cycles);

		cpu->PC = new_offset;
	} else {
		cpu->PC += 2;
	}

	wait_cycles (emu, 2 + ext_cycles);
}

void and_indirect_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = indirect_y (emu);

	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A & val,
			eq,
			(5 << 8) | 2);
}

void and_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);

	uint8_t val = emu->ram[addr];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A & val,
			eq,
			(4 << 8) | 2);
}

void rol_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);

	rol_acts (emu,
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&emu->ram[addr],
			(6 << 8) | 2);
}

void sec_implied (struct NESEmu *emu) 
{
	emu->cpu.P |= (STATUS_FLAG_CF);
	emu->cpu.PC++;

	wait_cycles (emu, 2);
}

void and_absolute_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_y (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A & val,
			eq,
			(4 << 8) | 3);
}

void and_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	/* TODO: Cross */
	uint16_t addr = absolute_x (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A & val,
			eq,
			(4 << 8) | 3);
}

void rol_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_x (emu);
	uint16_t off;
	uint8_t *m;
	if (addr < RAM_MAX) {
		m = emu->ram;
		off = 0;
	} else {
		m = emu->mem;
		off = 0x8000;
	}
	rol_acts (emu,
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&m[addr - off],
			(7 << 8) | 3);
}

void rti_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	uint16_t addr = 0x100 + cpu->S++;
	cpu->P = emu->ram[addr];

	addr = 0x100 + cpu->S;

	uint16_t new_pc = 0;
	new_pc = *(uint16_t *) &emu->ram[addr];

	cpu->S += 2;

	cpu->PC = new_pc;

	emu->is_returned_from_nmi = 1;

	wait_cycles (emu, 6);
}

void eor_indirect_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = indirect_x (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A ^ val,
			eq,
			(5 << 8) | 2);
}

void eor_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = zeropage (emu);
	uint8_t val = emu->ram[addr];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A ^ val,
			eq,
			(3 << 8) | 2);
}

void lsr_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = zeropage (emu);

	lsr_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&emu->ram[addr],
			(5 << 8) | 2);
}

void pha_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	uint16_t addr = 0x100 + --cpu->S;

	emu->ram[addr] = cpu->A;

	wait_cycles (emu, 3);

	cpu->PC++;
}

void eor_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t val = immediate_val (emu);

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A ^ val,
			eq,
			(2 << 8) | 2);
}
void lsr_accumulator (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	lsr_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->A,
			(2 << 8) | 1);
}

void jmp_absolute (struct NESEmu *emu)
{
	struct CPUNes *cpu = &emu->cpu;

	uint16_t new_offset = *(uint16_t *) &emu->mem[(cpu->PC + 1) - 0x8000];

	cpu->PC = new_offset;

	wait_cycles (emu, 3);
}

void eor_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A ^ val,
			eq,
			(4 << 8) | 2);
}

void lsr_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	uint8_t *m;
	uint16_t off;
	if (addr < RAM_MAX) {
		m = emu->ram;
		off = 0;
	} else {
		m = emu->mem;
		off = 0x8000;
	}

	lsr_acts (emu,
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
		&m[addr - off],
		(6 << 8) | 3);
}

void bvc_relative (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	int8_t offset = emu->mem[(cpu->PC + 1) - 0x8000];

	uint16_t new_offset;

	if (offset < 0) {
		uint8_t off = 0xff - offset - 1;
		new_offset = cpu->PC - off;
	} else {
		new_offset = cpu->PC + offset + 2;
	}

	uint32_t ext_cycles = 0;

	if (emu->cpu.P & STATUS_FLAG_VF) {
		cpu->PC += 2;
	} else {
		set_ext_cycles (cpu, offset, new_offset, &ext_cycles);

		cpu->PC = new_offset;
	}

	wait_cycles (emu, 2 + ext_cycles);
}

void eor_indirect_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = indirect_y (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A ^ val,
			eq,
			(6 << 8) | 2);
}

void eor_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);

	uint8_t val = emu->ram[addr];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A ^ val,
			eq,
			(4 << 8) | 2);
}

void lsr_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);

	lsr_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&emu->ram[addr],
			(6 << 8) | 2);
}

void cli_implied (struct NESEmu *emu) 
{
	emu->cpu.P &= ~(STATUS_FLAG_IF);
	emu->cpu.PC++;

	wait_cycles (emu, 2);
}

void eor_absolute_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_y (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A ^ val,
			eq,
			(4 << 8) | 3);
}

void eor_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	/* TODO: Cross */
	uint16_t addr = absolute_x (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A ^ val,
			eq,
			(4 << 8) | 3);
}

void lsr_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_x (emu);
	uint8_t *m;
	uint16_t off;
	if (addr < RAM_MAX) {
		m = emu->ram;
		off = 0;
	} else {
		m = emu->mem;
		off = 0x8000;
	}
	lsr_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&m[addr - off],
			(7 << 8) | 3);
}

void rts_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	uint16_t addr = 0x100 + cpu->S;

	cpu->PC = *(uint16_t *) &emu->ram[addr];
	cpu->S += 2;

	cpu->PC++;
	//printf ("rts %04x\n", cpu->PC);

	wait_cycles (emu, 6);
}

void adc_indirect_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = indirect_x (emu);
	uint8_t *m;
	uint16_t off;
	if (addr < RAM_MAX) {
		m = emu->ram;
		off = 0;
	} else {
		m = emu->mem;
		off = 0x8000;
	}

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + m[addr - off], 
		m[addr - off],
		eq,
		(6 << 8) | (2)
		);
}

void adc_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	uint8_t addr = zeropage (emu);

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + emu->ram[addr], 
		emu->ram[addr],
		eq,
		(3 << 8) | (2)
		);
}

void ror_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = zeropage (emu);

	ror_acts (emu,
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&emu->ram[addr],
			(5 << 8) | 2);
}

void pla_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	uint16_t addr = 0x100 + cpu->S++;

	cpu->A = emu->ram[addr];

	uint8_t flags = (STATUS_FLAG_NF|STATUS_FLAG_ZF);
	cpu->P &= ~(flags);

	if (cpu->A == 0x80) {
		cpu->P |= STATUS_FLAG_NF;
	} 
	if (cpu->A == 0x0) {
		cpu->P |= STATUS_FLAG_ZF;
	}

	wait_cycles (emu, 4);

	cpu->PC++;
}



void adc_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t value = immediate_val (emu);

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + value, 
		value,
		eq,
		(2 << 8) | (2)
		);
}

void ror_accumulator (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	ror_acts (emu,
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->A,
			(2 << 8) | 1);
}

void jmp_indirect (struct NESEmu *emu) 
{
	emu->cpu.PC = indirect (emu);

	wait_cycles (emu, 5);
}

void adc_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);

	uint8_t *m;
	uint16_t off;
	if (addr < RAM_MAX) {
		m = emu->ram;
		off = 0;
	} else {
		m = emu->mem;
		off = 0x8000;
	}

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + m[addr - off], 
		m[addr - off],
		eq,
		(4 << 8) | (3)
		);
}

void ror_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	uint16_t off;
	uint8_t *m;
	if (addr < RAM_MAX) {
		m = emu->ram;
		off = 0;
	} else {
		m = emu->mem;
		off = 0x8000;
	}
	ror_acts (emu,
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&m[addr - off],
			(6 << 8) | 3);
}

void bvs_relative (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	int8_t offset = emu->mem[(cpu->PC + 1) - 0x8000];

	uint16_t new_offset;

	if (offset < 0) {
		uint8_t off = 0xff - offset - 1;
		new_offset = cpu->PC - off;
	} else {
		new_offset = cpu->PC + offset + 2;
	}

	uint32_t ext_cycles = 0;

	if (emu->cpu.P & STATUS_FLAG_VF) {
		set_ext_cycles (cpu, offset, new_offset, &ext_cycles);

		cpu->PC = new_offset;
	} else {
		cpu->PC += 2;
	}

	wait_cycles (emu, 2 + ext_cycles);
}

void adc_indirect_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = indirect_y (emu);

	uint8_t *m;
	uint16_t off;
	if (addr < RAM_MAX) {
		m = emu->ram;
		off = 0;
	} else {
		m = emu->mem;
		off = 0x8000;
	}

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + m[addr - off], 
		m[addr - off],
		eq,
		(5 << 8) | (2)
		);

	/*
	 * TODO: cross
	 */
}

void adc_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + emu->ram[addr], 
		emu->ram[addr],
		eq,
		(4 << 8) | (2)
		);
}

void ror_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);

	ror_acts (emu,
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&emu->ram[addr],
			(6 << 8) | 2);
}

void sei_implied (struct NESEmu *emu) 
{
	emu->cpu.P &= (STATUS_FLAG_IF);
	emu->cpu.PC++;

	wait_cycles (emu, 2);
}

void adc_absolute_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_y (emu);

	uint8_t *m;
	uint16_t off;
	if (addr < RAM_MAX) {
		m = emu->ram;
		off = 0;
	} else {
		m = emu->mem;
		off = 0x8000;
	}

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + m[addr - off], 
		m[addr - off],
		eq,
		(4 << 8) | (3)
		);
	/*
	 * TODO: cross page
	 */
}

void adc_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_x (emu);

	uint8_t *m;
	uint16_t off;
	if (addr < RAM_MAX) {
		m = emu->ram;
		off = 0;
	} else {
		m = emu->mem;
		off = 0x8000;
	}

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + m[addr - off], 
		m[addr - off],
		eq,
		(4 << 8) | (3)
		);
	/*
	 * TODO: cross page
	 */
}

void ror_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_x (emu);
	uint16_t off;
	uint8_t *m;
	if (addr < RAM_MAX) {
		m = emu->ram;
		off = 0;
	} else {
		m = emu->mem;
		off = 0x8000;
	}
	ror_acts (emu,
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&m[addr - off],
			(7 << 8) | 3);
}

void sta_indirect_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = indirect_x (emu);
	st_acts (emu, &cpu->A, addr, (6 << 8) | 2);
}

void sty_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);
	st_acts (emu, &cpu->Y, addr, (3 << 8) | 2);
}

void sta_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);
	st_acts (emu, &cpu->A, addr, (3 << 8) | 2);
}
void stx_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);
	st_acts (emu, &cpu->X, addr, (3 << 8) | 2);
}

void dey_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->P &= ~(STATUS_FLAG_NF|STATUS_FLAG_ZF);

	cpu->Y--;

	if (cpu->Y & 0x80)
		cpu->P |= STATUS_FLAG_NF;
	if (cpu->Y == 0)
		cpu->P |= STATUS_FLAG_ZF;

	wait_cycles (emu, 2);

	cpu->PC++;
}

void txa_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->P &= ~(STATUS_FLAG_ZF|STATUS_FLAG_NF);

	cpu->A = cpu->X;

	if (cpu->A == 0) {
		cpu->P |= STATUS_FLAG_ZF;
	}
	if (cpu->A & 0x80) {
		cpu->P |= STATUS_FLAG_NF;
	}

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void sty_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	st_acts (emu, &cpu->Y, addr, (4 << 8) | 3);
}

void sta_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	st_acts (emu, &cpu->A, addr, (4 << 8) | 3);
}



void stx_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	st_acts (emu, &cpu->X, addr, (4 << 8) | 3);
}

void bcc_relative (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	int8_t offset = emu->mem[(cpu->PC + 1) - 0x8000];

	uint16_t new_offset;

	if (offset < 0) {
		uint8_t off = 0xff - offset - 1;
		new_offset = cpu->PC - off;
	} else {
		new_offset = cpu->PC + offset + 2;
	}

	uint32_t ext_cycles = 0;

	if (emu->cpu.P & STATUS_FLAG_CF) {
		cpu->PC += 2;
	} else {
		set_ext_cycles (cpu, offset, new_offset, &ext_cycles);

		cpu->PC = new_offset;
	}

	wait_cycles (emu, 2 + ext_cycles);
}

void sta_indirect_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = indirect_y (emu);
	st_acts (emu, &cpu->A, addr, (6 << 8) | 2);
}

void sty_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);
	st_acts (emu, &cpu->Y, addr, (4 << 8) | 2);
}

void sta_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);
	st_acts (emu, &cpu->A, addr, (4 << 8) | 2);
}
void stx_zeropage_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_y (emu);
	st_acts (emu, &cpu->X, addr, (4 << 8) | 2);
}

void tya_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->P &= ~(STATUS_FLAG_ZF|STATUS_FLAG_NF);

	cpu->A = cpu->Y;

	if (cpu->A == 0) {
		cpu->P |= STATUS_FLAG_ZF;
	}
	if (cpu->A & 0x80) {
		cpu->P |= STATUS_FLAG_NF;
	}

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void sta_absolute_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_y (emu);
	st_acts (emu, &cpu->A, addr, (5 << 8) | 3);
}

void txs_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	// TODO: check this code
//	cpu->S &= 0xff;
	cpu->S = cpu->X;

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void sta_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_x (emu);
	st_acts (emu, &cpu->A, addr, (5 << 8) | 3);
}

void ldy_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = immediate (emu);
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->Y,
			addr,
			(2 << 8) | 2);
}

void lda_indirect_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = indirect_x (emu);
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->A,
			addr,
			(6 << 8) | 2);
}

void ldx_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = immediate (emu);
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->X,
			addr,
			(2 << 8) | 2);
}

void ldy_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->Y,
			addr,
			(3 << 8) | 2);
}

void lda_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->A,
			addr,
			(3 << 8) | 2);
}

void ldx_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->X,
			addr,
			(3 << 8) | 2);
}

void tay_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->P &= ~(STATUS_FLAG_ZF|STATUS_FLAG_NF);

	cpu->Y = cpu->A;
	
	if (cpu->Y == 0) {
		cpu->P |= STATUS_FLAG_ZF;
	}
	if (cpu->Y & 0x80) {
		cpu->P |= STATUS_FLAG_NF;
	}

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void lda_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = immediate (emu);
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->A,
			addr,
			(2 << 8) | 2);
}

void tax_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->P &= ~(STATUS_FLAG_ZF|STATUS_FLAG_NF);

	cpu->X = cpu->A;

	if (cpu->X == 0) {
		cpu->P |= STATUS_FLAG_ZF;
	}
	if (cpu->X & 0x80) {
		cpu->P |= STATUS_FLAG_NF;
	}

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void ldy_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->Y,
			addr,
			(4 << 8) | 3);
}

void lda_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->A,
			addr,
			(4 << 8) | 3);
}

void ldx_absolute (struct NESEmu *emu)
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->X,
			addr,
			(4 << 8) | 3);
}

void bcs_relative (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	int8_t offset = emu->mem[(cpu->PC + 1) - 0x8000];

	uint16_t new_offset;

	if (offset < 0) {
		uint8_t off = 0xff - offset - 1;
		new_offset = cpu->PC - off;
	} else {
		new_offset = cpu->PC + offset + 2;
	}

	uint32_t ext_cycles = 0;

	if (emu->cpu.P & STATUS_FLAG_CF) {
		set_ext_cycles (cpu, offset, new_offset, &ext_cycles);

		cpu->PC = new_offset;
	} else {
		cpu->PC += 2;
	}

	wait_cycles (emu, 2 + ext_cycles);
}

void lda_indirect_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = indirect_y (emu);
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->A,
			addr,
			(5 << 8) | 2);
}
void ldy_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->Y,
			addr,
			(4 << 8) | 2);
}

void lda_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->A,
			addr,
			(4 << 8) | 2);
}

void ldx_zeropage_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_y (emu);
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->X,
			addr,
			(4 << 8) | 2);
}

void clv_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	emu->cpu.P &= ~(STATUS_FLAG_VF);

	wait_cycles (emu, 2);

	emu->cpu.PC++;
}

void lda_absolute_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_y (emu);
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->A,
			addr,
			(4 << 8) | 3);
}

void tsx_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->P &= ~(STATUS_FLAG_ZF|STATUS_FLAG_NF);

	cpu->X = cpu->S;

	if (cpu->X == 0) {
		cpu->P |= STATUS_FLAG_ZF;
	}
	if (cpu->X & 0x80) {
		cpu->P |= STATUS_FLAG_NF;
	}

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void ldy_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_x (emu);
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->Y,
			addr,
			(4 << 8) | 3);
}

void lda_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_x (emu);
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->A,
			addr,
			(4 << 8) | 3);
}

void ldx_absolute_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_y (emu);
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->X,
			addr,
			(4 << 8) | 3);
}

void cpy_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t val = immediate_val (emu);

	cmp_act (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->Y,
			cpu->Y - val,
			val,
			(2 << 8) | 2);
}

void cmp_indirect_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = indirect_x (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	cmp_act (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->A,
			cpu->A - val,
			val,
			(6 << 8) | 2);
}

void cpy_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);

	uint8_t val = emu->ram[addr];

	cmp_act (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->Y,
			cpu->Y - val,
			val,
			(3 << 8) | 2);
}

void cmp_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);

	uint8_t val = emu->ram[addr];

	cmp_act (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->A,
			cpu->A - val,
			val,
			(3 << 8) | 2);
}

void dec_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = zeropage (emu);

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&emu->ram[addr],
			--emu->ram[addr],
			void_eq,
			(5 << 8) | 2);
}

void iny_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->P &= ~(STATUS_FLAG_NF|STATUS_FLAG_ZF);

	emu->cpu.Y++;

	if (cpu->Y & 0x80)
		cpu->P |= STATUS_FLAG_NF;
	if (cpu->Y == 0)
		cpu->P |= STATUS_FLAG_ZF;

	wait_cycles (emu, 2);

	emu->cpu.PC++;
}

void cmp_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t val = immediate_val (emu);

	cmp_act (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->A,
			cpu->A - val,
			val,
			(2 << 8) | 2);
}

void dex_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->P &= ~(STATUS_FLAG_NF|STATUS_FLAG_ZF);

	cpu->X--;

	if (cpu->X & 0x80)
		cpu->P |= STATUS_FLAG_NF;
	if (cpu->X == 0)
		cpu->P |= STATUS_FLAG_ZF;

	wait_cycles (emu, 2);

	cpu->PC++;
}

void cpy_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);

	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	cmp_act (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->Y,
			cpu->Y - val,
			val,
			(4 << 8) | 3);
}

void cmp_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	cmp_act (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->A,
			cpu->A - val,
			val,
			(4 << 8) | 3);
}

void dec_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	uint8_t *m;
	uint16_t off;
	if (addr < RAM_MAX) {
		off = 0;
		m = emu->ram;
	} else {
		off = 0x8000;
		m = emu->mem;
	}

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&m[addr - off],
			--m[addr - off],
			void_eq,
			(6 << 8) | 3);
}

void bne_relative (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	int8_t offset = emu->mem[(cpu->PC + 1) - 0x8000];

	uint16_t new_offset;

	if (offset < 0) {
		uint8_t off = 0xff - offset - 1;
		new_offset = cpu->PC - off;
	} else {
		new_offset = cpu->PC + offset + 2;
	}

	uint32_t ext_cycles = 0;

	if (emu->cpu.P & STATUS_FLAG_ZF) {
		cpu->PC += 2;
	} else {
		set_ext_cycles (cpu, offset, new_offset, &ext_cycles);

		cpu->PC = new_offset;
	}

	wait_cycles (emu, 2 + ext_cycles);
}

void cmp_indirect_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	uint16_t addr = indirect_y (emu);

	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	cmp_act (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->A,
			cpu->A - val,
			val,
			(5 << 8) | 2);
}

void cmp_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);

	cmp_act (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->A,
			cpu->A - emu->ram[addr],
			emu->ram[addr],
			(4 << 8) | 2);
}

void dec_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&emu->ram[addr],
			--emu->ram[addr],
			void_eq,
			(6 << 8) | 2);
}

void cld_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	emu->cpu.P &= ~(STATUS_FLAG_DF);

	wait_cycles (emu, 2);

	emu->cpu.PC++;
}

void cmp_absolute_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_y (emu);

	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	cmp_act (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->A,
			cpu->A - val,
			val,
			(4 << 8) | 3);
}

void cmp_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_x (emu);

	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	cmp_act (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->A,
			cpu->A - val,
			val,
			(4 << 8) | 3);
}

void dec_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_x (emu);
	uint16_t off;
	uint8_t *m;
	if (addr < RAM_MAX) {
		m = emu->ram;
		off = 0;
	} else {
		m = emu->mem;
		off = 0x8000;
	}

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&m[addr - off],
			--m[addr - off],
			void_eq,
			(7 << 8) | 3);
}

void cpx_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	uint8_t val = immediate_val (emu);

	cmp_act (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->X,
			cpu->X - val,
			val,
			(2 << 8) | 2);
}

void sbc_indirect_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = indirect_x (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + ~val, 
		~val,
		eq,
		(6 << 8) | (2)
		);

#if 0
	/* TODO: need or CF flag */
	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_VF,
			&cpu->A,
			cpu->A - val - ((emu->cpu.P & STATUS_FLAG_CF)? 0: 1),
			eq,
			(6 << 8) | 2);
#endif
}

void cpx_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	uint8_t addr = zeropage (emu);

	uint8_t val = emu->ram[addr];

	cmp_act (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->X,
			cpu->X - val,
			val,
			(3 << 8) | 2);
}

void sbc_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);

	uint8_t val = emu->ram[addr];

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + ~val, 
		~val,
		eq,
		(3 << 8) | (2)
		);
#if 0
	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF,
			&cpu->A,
			cpu->A - val - ((emu->cpu.P & STATUS_FLAG_CF)? 0: 1),
			eq,
			(3 << 8) | 2);
#endif
}

void inc_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&emu->ram[addr],
			++emu->ram[addr],
			void_eq,
			(5 << 8) | 2);

}

void inx_implied (struct NESEmu *emu)
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->P &= ~(STATUS_FLAG_NF|STATUS_FLAG_ZF);

	emu->cpu.X++;

	if (cpu->X & 0x80)
		cpu->P |= STATUS_FLAG_NF;

	if (cpu->X == 0)
		cpu->P |= STATUS_FLAG_ZF;

	wait_cycles (emu, 2);

	cpu->PC++;
}

void sbc_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t val = immediate_val (emu);

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + ~val, 
		~val,
		eq,
		(2 << 8) | (2)
		);

#if 0
	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF,
			&cpu->A,
			cpu->A - val - ((emu->cpu.P & STATUS_FLAG_CF)? 0: 1),
			eq,
			(2 << 8) | 2);
#endif
}

void nop_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	emu->cpu.PC++;

	wait_cycles (emu, 2);
}

void cpx_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	cmp_act (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->X,
			cpu->X - val,
			val,
			(4 << 8) | 3);
}
void sbc_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + ~val, 
		~val,
		eq,
		(4 << 8) | (3)
		);

#if 0
	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF,
			&cpu->A,
			cpu->A - val - ((emu->cpu.P & STATUS_FLAG_CF)? 0: 1),
			eq,
			(4 << 8) | 3);
#endif
}

void inc_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	uint8_t *m;
	uint16_t off;
	if (addr < RAM_MAX) {
		m = emu->ram;
		off = 0;
	} else {
		m = emu->mem;
		off = 0x8000;
	}

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&m[addr - off],
			++m[addr - off],
			void_eq,
			(6 << 8) | 3);

}

void beq_relative (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	int8_t offset = emu->mem[(cpu->PC + 1) - 0x8000];

	uint16_t new_offset;

	if (offset < 0) {
		uint8_t off = 0xff - offset - 1;
		new_offset = cpu->PC - off;
	} else {
		new_offset = cpu->PC + offset + 2;
	}

	uint32_t ext_cycles = 0;

	if (emu->cpu.P & STATUS_FLAG_ZF) {
		set_ext_cycles (cpu, offset, new_offset, &ext_cycles);

		cpu->PC = new_offset;
	} else {
		cpu->PC += 2;
	}

	wait_cycles (emu, 2 + ext_cycles);
}

void sbc_indirect_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = indirect_y (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + ~val, 
		~val,
		eq,
		(5 << 8) | (2)
		);

#if 0
	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF,
			&cpu->A,
			cpu->A - val - ((emu->cpu.P & STATUS_FLAG_CF)? 0: 1),
			eq,
			(5 << 8) | 2);
#endif
}

void sbc_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);
	uint8_t val = emu->ram[addr];

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + ~val, 
		~val,
		eq,
		(4 << 8) | (2)
		);
#if 0
	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF,
			&cpu->A,
			cpu->A - val - ((emu->cpu.P & STATUS_FLAG_CF)? 0: 1),
			eq,
			(4 << 8) | 2);
#endif
}

void inc_zeropage_x (struct NESEmu *emu) {
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&emu->ram[addr],
			++emu->ram[addr],
			void_eq,
			(6 << 8) | 2);
}

void sed_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	emu->cpu.P |= (STATUS_FLAG_DF);

	wait_cycles (emu, 2);

	emu->cpu.PC++;
}

void sbc_absolute_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_y (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + ~val, 
		~val,
		eq,
		(4 << 8) | (3)
		);

#if 0
	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF,
			&cpu->A,
			cpu->A - val - ((emu->cpu.P & STATUS_FLAG_CF)? 0: 1),
			eq,
			(4 << 8) | 3);
#endif
}
void sbc_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_x (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + ~val, 
		~val,
		eq,
		(4 << 8) | (3)
		);
#if 0
	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF,
			&cpu->A,
			cpu->A - val - ((emu->cpu.P & STATUS_FLAG_CF)? 0: 1),
			eq,
			(4 << 8) | 3);
#endif
}

void inc_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_x (emu);
	uint8_t *m;
	uint16_t off;
	if (addr < RAM_MAX) {
		m = emu->ram;
		off = 0;
	} else {
		m = emu->mem;
		off = 0x8000;
	}

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&m[addr - off],
			++m[addr - off],
			void_eq,
			(7 << 8) | 3);
}
