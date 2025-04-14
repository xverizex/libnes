#include <stdint.h>
#include <stddef.h>
#include <cpunes.h>
#include <stdio.h>
#include <time.h>
#include <debugger.h>

void platform_ppu_mask (struct NESEmu *emu, void *_other_data);

void wait_cycles (struct NESEmu *emu, uint32_t cycles)
{
	//emu->last_cycles_float = (float) cycles * 0.000558730074f;
	//emu->last_cycles_int64 = 16L;
	//TODO: check that this correct +=
	emu->last_cycles_int64 += cycles * NS_CYCLE;
	emu->cur_cycles += cycles;
	emu->work_cycles = cycles;
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

void check_collision (struct NESEmu *emu)
{
	uint8_t sprite_0_y = emu->oam[0];
	uint8_t sprite_0_id_texture = emu->oam[1];
	uint8_t sprite_0_flags = emu->oam[2];
	uint8_t sprite_0_x = emu->oam[3];

	uint32_t sprite_overflow = 0;
	uint32_t idx = 0;
	for (int i = 0; i < 64; i++) {
		uint8_t py = emu->oam[idx + 0];
		if ((py <= emu->scanline) && ((py + 8) >= emu->scanline)) {
			sprite_overflow++;
		}

		idx += 4;
	}
	if (sprite_overflow >= 8) {
		emu->ppu_status |= 0x60;
	}

	if ((sprite_0_y <= emu->scanline) && ((sprite_0_y + 8) >= emu->scanline)) {
		uint32_t idx = 4;
		for (int i = 0; i < 63; i++) {

			uint8_t flags = emu->oam[idx + 2];

			uint8_t py = emu->oam[idx + 0];
			uint8_t id_texture = emu->oam[idx + 1];
			uint8_t px = emu->oam[idx + 3];

			if ((sprite_0_x <= px) && ((sprite_0_x + 8) >= px)) {
				if ((sprite_0_y <= py) && ((sprite_0_y + 8) >= py)) {
					emu->ppu_status |= 0x40;
					break;
				}
			}	
			idx += 4;
		}
	} else {
		emu->ppu_status &= ~(0x40);
	}
}

void read_from_address (struct NESEmu *emu, uint16_t addr, uint8_t *r)
{
	//printf ("\tread from addr: %04x = %02x\n", addr, *r);
	if (addr == PPUSTATUS) {
		emu->addr_off = 0;
		emu->ppu_addr = 0;
		*r = emu->ppu_status;
		if (emu->ppu_status & 0x80) {
			emu->ppu_status &= 0x7f;
		}
		//printf ("%04x <- pc\n", emu->cpu.PC);
		return;
	}
	if (addr == PPUSCROLL) {
		if (emu->cnt_read_scrollxy == 0) {
			*r = emu->offx;
			emu->cnt_read_scrollxy++;
		} else {
			*r = emu->offy;
			emu->cnt_read_scrollxy = 0;
		}
		return;
	}
	if (addr < RAM_MAX) {
#if 0
		if ((emu->cpu.PC == 0xc059)) {
			debug (emu, 0x600, 0x6f0);
		}
#endif
		*r = emu->ram[addr];
		return;
	}
	if (addr == 0x2007) {
		// TODO: what is return ppu data?
		if (emu->ppu_addr >= 0x4000) {
			printf ("ppu read exit; %04x\n", emu->ppu_addr);
			emu->is_debug_exit = 1;
		}
		*r = emu->ppu[emu->ppu_addr];
		return;
	} 

	if (addr >= 0x4016 && addr <= 0x4017) {
		if (addr == 0x4016) {
			if (emu->new_state <= 7) {
				*r = 0x40 | (((emu->joy0 & (1 << emu->new_state)) >> emu->new_state) & 0x01);
				//printf ("emu->pc read joy: %04x\n", emu->cpu.PC);
				emu->new_state++;
#if 0
				if (*r != 0x40)
					printf ("%02x\n", *r);
#endif
				if (emu->new_state == 8) {
					//emu->joy0 = 0;
					//emu->state_buttons0 = 0;
				}
			} else {
				*r = 0x40; // JOYS
			}
		} else {
			*r = 0x40;
		}
		return;
	} 
	if ((addr >= PPUCTRL) && (addr <= PPUDATA)) {
		*r = emu->ctrl[addr - 0x2000];
		return;
	}

	*r = emu->mem[addr - 0x8000];
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
		if ((emu->oam_addr >= 0x100) && ((addr) >= emu->oam_addr) && ((addr) <= (emu->oam_addr + 0xff))) {
			//printf ("writing to %04x value %02x from pc %04x\n", addr, *r, emu->cpu.PC);
			emu->oam[addr - emu->oam_addr] = *r;
		} else {
#if 0
			if (addr >= 0x17 && addr <= (0x17 + 8)) {
				printf (">>> from PC: %04x\n", emu->cpu.PC);
				debug (emu, 0x0, 0x30);
			}
#endif
#if 0
			if (addr == 0x15) {
				printf ("from PC: %04x; A: %02x X: %02X Y: %02X P: %02X\n",
						emu->cpu.PC,
						emu->cpu.A,
						emu->cpu.X,
						emu->cpu.Y,
						emu->cpu.P);
			}
#endif
			emu->ram[addr] = *r;
		}
		return;
	}

	if (addr == PPUSCROLL) {
		if (emu->cnt_write_scrollxy == 0) {
			emu->offx = *r;
			emu->cnt_write_scrollxy++;
			//uint32_t indx = emu->indx_scroll_linex / 8;
			uint32_t indx = emu->scanline / 8 + 1; // crutch (+ 1)
			emu->scroll_x[emu->max_scroll_indx] = *r;
			uint32_t res_indx = indx / 8;
			emu->scroll_tile_x[emu->max_scroll_indx] = res_indx;
			emu->max_scroll_indx++;
			//printf ("scroll x: %02x from %04x; indx: %d; linetile: %d; scanline: %d\n", *r, emu->cpu.PC, indx, indx / 8, emu->scanline);
		} else {
			emu->offy = *r;
			//printf ("scroll y: %02x from %04x\n", *r, emu->cpu.PC);
			emu->cnt_write_scrollxy = 0;
		}
		return;
	} else if (addr == PPUCTRL) {
		emu->ctrl[REAL_PPUCTRL] = *r;
		if (*r & PPUCTRL_VBLANK_NMI) {
			emu->scanline = SCANLINE_VBLANK_START;
		}
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
#if 0
		if ((*r >= 0x68) && (*r <= 0x6f)) {
			printf ("writing collapse from PC: %04x; value: %02x; A: %02x; X: %02x; Y: %02x; P: %02x;\n", emu->cpu.PC, *r,
					emu->cpu.A,
					emu->cpu.X,
					emu->cpu.Y,
					emu->cpu.P);
			debug (emu, 0x600, 0x700);
		}
#endif
		emu->ppu[emu->ppu_addr] = *r;
		emu->ppu_addr++;
					  
	}  else if (addr >= PPUCTRL && addr <= PPUDATA) {
		emu->ctrl[addr - 0x2000] = *r;
	} 
	if ((addr == 0x4017) || (addr == 0x4016)) {
		//printf ("0x4017 %02x\n", *r);
		if (*r & 0x01) {
			emu->new_state = 8;
			emu->state_buttons0 = 0;
		} else if ((*r & 0x01) == 0x0) {
			emu->new_state = 0;
			emu->is_new_state = 1;
			//emu->joy0 = 0;
			//emu->state_buttons0 = 0;
		}
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

void ld_acts_imm (struct NESEmu *emu, uint8_t flags, uint8_t *reg, uint8_t val, uint16_t cycles_and_bytes)
{
	struct CPUNes *cpu = &emu->cpu;
	*reg = val;
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
    		new_addr = *(uint16_t *) &emu->ram[addr];
	} else {
    		new_addr = *(uint16_t *) &emu->mem[addr - 0x8000];
	}

	return new_addr;
}

uint16_t indirect_x (struct NESEmu *emu)
{
	uint8_t addr = emu->mem[(emu->cpu.PC + 1) - 0x8000];
	uint16_t indirect = 0;

	uint8_t res = addr + emu->cpu.X;

	indirect = *((uint16_t *) &emu->ram[res]);

	return indirect;
}

uint16_t indirect_y (struct NESEmu *emu)
{
	uint8_t zeroaddr = emu->mem[(emu->cpu.PC + 1) - 0x8000];
	uint16_t addr = 0;

	addr = *((uint16_t *) &emu->ram[zeroaddr]);

	return addr + (uint16_t) emu->cpu.Y;
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ORA ($%02x, X)", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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

	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ORA $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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

	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ASL $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	asl_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&emu->ram[addr],
			(5 << 8) | 2);
}

void php_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "PHP");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	--cpu->S;
	uint16_t addr = 0x100 + cpu->S;
	emu->ram[addr] = cpu->P;

	wait_cycles (emu, 3);

	cpu->PC++;
}

void ora_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t val = immediate_val (emu);

	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ORA #$%02x", val);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ASL A");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ORA $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ASL $%04x", *(uint16_t *) &emu->mem[emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	asl_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&m[addr - off],
			(6 << 8) | 3);
}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BPL $%04x", new_offset);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, new_offset, debugger_print_regs (emu));
		if (emu->only_show) return;
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
	uint16_t addr = indirect_y (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];
	uint16_t cross_cycles = 0;

	if (emu->cpu.PC >= addr) {
		cross_cycles = (emu->cpu.PC - addr) >> 8;
		if (((emu->cpu.PC - addr) >> 8) > 255) {
			printf ("exit with %d line\n", __LINE__);
			exit (0);
		}
	} else {
		cross_cycles = (addr - emu->cpu.PC) >> 8;
		if (((addr - emu->cpu.PC) >> 8) > 255) {
			printf ("exit with %d line\n", __LINE__);
			exit (0);
		}
	}
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ORA ($%02x), Y", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A | val,
			eq,
			((5 + cross_cycles) << 8) | 2);
}

void ora_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);

	uint8_t val = emu->ram[addr];

	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ORA $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
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

	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ASL $%02x, X", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	asl_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&emu->ram[addr],
			(6 << 8) | 2);
}
void clc_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "CLC");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	cpu->P &= ~(STATUS_FLAG_CF);

	wait_cycles (emu, 2);

	cpu->PC++;
}

void ora_absolute_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_y (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];
	uint16_t cross_cycles = 0;

	if (emu->cpu.PC >= addr) {
		cross_cycles = (emu->cpu.PC - addr) >> 8;
		if (((emu->cpu.PC - addr) >> 8) > 255) {
			printf ("exit with %d line\n", __LINE__);
			exit (0);
		}
	} else {
		cross_cycles = (addr - emu->cpu.PC) >> 8;
		if (((addr - emu->cpu.PC) >> 8) > 255) {
			printf ("exit with %d line\n", __LINE__);
			exit (0);
		}
	}
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ORA $%04x, Y", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A | val,
			eq,
			((4 + cross_cycles) << 8) | 3);
}

void ora_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_x (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];
	uint8_t cross_cycles = 0;
	if (emu->cpu.PC >= addr) {
		cross_cycles = (emu->cpu.PC - addr) >> 8;
		if (((emu->cpu.PC - addr) >> 8) > 255) {
			printf ("exit with %d line\n", __LINE__);
			exit (0);
		}
	} else {
		cross_cycles = (addr - emu->cpu.PC) >> 8;
		if (((addr - emu->cpu.PC) >> 8) > 255) {
			printf ("exit with %d line\n", __LINE__);
			exit (0);
		}
	}
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ORA $%04x, X", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A | val,
			eq,
			((4 + cross_cycles) << 8) | 3);
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ASL $%04x, X", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
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

	--cpu->S;

       	addr = 0x100 + cpu->S;

	emu->ram[addr] = (uint8_t) ((pc >> 8) & 0xff);
	--cpu->S;

	addr = 0x100 + cpu->S;
	emu->ram[addr] = (uint8_t) (pc & 0xff);

	uint16_t new_pc = 0;
	new_pc = *(uint16_t *) &emu->mem[(cpu->PC + 1) - 0x8000];
	//printf ("called: %04x\n", new_pc);
	
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "JSR $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, new_pc, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	cpu->PC = new_pc;

	wait_cycles (emu, 6);
}

void and_indirect_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = indirect_x (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "AND ($%02x, X)", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BIT $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "AND $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ROL $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	rol_acts (emu,
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&emu->ram[addr],
			(5 << 8) | 2);
}

void plp_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = 0x100 + cpu->S;

	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "PLP");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	cpu->S++;
	cpu->P = emu->ram[addr];

	wait_cycles (emu, 4);

	cpu->PC++;
}



void and_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t val = immediate_val (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "AND #$%02x", val);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, val, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ROL A");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	rol_acts (emu,
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->A,
			(2 << 8) | 1);
}

void bit_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	uint16_t addr = absolute (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BIT $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "AND $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ROL $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BMI $%04x", new_offset);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, new_offset, debugger_print_regs (emu));
		if (emu->only_show) return;
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "AND ($%02x), Y", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "AND $%02x, X", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ROL $%02x, X", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	rol_acts (emu,
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&emu->ram[addr],
			(6 << 8) | 2);
}

void sec_implied (struct NESEmu *emu) 
{
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "SEC");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	emu->cpu.P |= (STATUS_FLAG_CF);
	emu->cpu.PC++;

	wait_cycles (emu, 2);
}

void and_absolute_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_y (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "AND $%04x, Y", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	uint16_t addr = absolute_x (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];
	uint16_t cross_cycles = 0;

	if (emu->cpu.PC >= addr) {
		cross_cycles = (emu->cpu.PC - addr) >> 8;
		if (((emu->cpu.PC - addr) >> 8) > 255) {
			printf ("exit with %d line\n", __LINE__);
			exit (0);
		}
	} else {
		cross_cycles = (addr - emu->cpu.PC) >> 8;
		if (((addr - emu->cpu.PC) >> 8) > 255) {
			printf ("exit with %d line\n", __LINE__);
			exit (0);
		}
	}
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "AND $%04x, X", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A & val,
			eq,
			((4 + cross_cycles) << 8) | 3);
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ROL $%04x, X", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	rol_acts (emu,
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&m[addr - off],
			(7 << 8) | 3);
}

void rti_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	uint16_t addr = 0x100 + cpu->S;
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "RTI");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	cpu->P = emu->ram[addr];
	cpu->S++;

	addr = 0x100 + cpu->S;

	cpu->PC = 0;
	cpu->PC = (emu->ram[addr] & 0xff);
	cpu->S++;

	addr = 0x100 + cpu->S;

	cpu->S++;

	cpu->PC |= ((emu->ram[addr] << 8) & 0xff00);

	emu->is_returned_from_nmi = 1;

	wait_cycles (emu, 6);
}

void eor_indirect_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = indirect_x (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "EOR ($%02x, X)", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "EOR $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "LSR $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	lsr_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&emu->ram[addr],
			(5 << 8) | 2);
}

void pha_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	--cpu->S;
	uint16_t addr = 0x100 + cpu->S;
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "PHA");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	emu->ram[addr] = cpu->A;

	wait_cycles (emu, 3);

	cpu->PC++;
}

void eor_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t val = immediate_val (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "EOR #$%02x", val);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, val, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "LSR A");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	lsr_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->A,
			(2 << 8) | 1);
}

void jmp_absolute (struct NESEmu *emu)
{
	struct CPUNes *cpu = &emu->cpu;

	uint16_t new_offset = *(uint16_t *) &emu->mem[(cpu->PC + 1) - 0x8000];
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "JMP $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, new_offset, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	cpu->PC = new_offset;

	wait_cycles (emu, 3);
}

void eor_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "EOR $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "LSR $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BVC $%04x", new_offset);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, new_offset, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "EOR ($%02x), Y", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "EOR $%02x, X", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "LSR $%02x, X", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	lsr_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&emu->ram[addr],
			(6 << 8) | 2);
}

void cli_implied (struct NESEmu *emu) 
{
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "CLI");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	emu->cpu.P &= ~(STATUS_FLAG_IF);
	emu->cpu.PC++;

	wait_cycles (emu, 2);
}

void eor_absolute_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_y (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "EOR $%04x, Y", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	uint16_t addr = absolute_x (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];
	uint16_t cross_cycles = 0;

	if (emu->cpu.PC >= addr) {
		cross_cycles = (emu->cpu.PC - addr) >> 8;
		if (((emu->cpu.PC - addr) >> 8) > 255) {
			printf ("exit with %d line\n", __LINE__);
			exit (0);
		}
	} else {
		cross_cycles = (addr - emu->cpu.PC) >> 8;
		if (((addr - emu->cpu.PC) >> 8) > 255) {
			printf ("exit with %d line\n", __LINE__);
			exit (0);
		}
	}
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "EOR $%04x, X", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&cpu->A,
			cpu->A ^ val,
			eq,
			((4 + cross_cycles) << 8) | 3);
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "LSR $%04x, X", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	lsr_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&m[addr - off],
			(7 << 8) | 3);
}

void rts_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "RTS");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	uint16_t addr = 0x100 + cpu->S;

	cpu->PC = 0;
	cpu->PC |= (emu->ram[addr] & 0xff);
	cpu->S++;

	addr = 0x100 + cpu->S;

	cpu->S++;

	cpu->PC |= ((emu->ram[addr] << 8) & 0xff00);

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ADC ($%02x, X)", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ADC $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ROR $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000]);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	ror_acts (emu,
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&emu->ram[addr],
			(5 << 8) | 2);
}

void pla_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "PLA");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	uint16_t addr = 0x100 + cpu->S;
	cpu->S++;

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "ADC #$%02x", value);
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, buf, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "ROR A", 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	ror_acts (emu,
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&cpu->A,
			(2 << 8) | 1);
}

void jmp_indirect (struct NESEmu *emu) 
{
	uint16_t off = indirect (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "JMP ($%04x)", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], off, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	emu->cpu.PC = off;

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "ADC $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "ROR $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "BVS $%04x", new_offset, new_offset, debugger_print_regs (emu));
		if (emu->only_show) return;
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

	uint8_t cross_cycles = 0;
	if (emu->cpu.PC >= addr) {
		cross_cycles = (emu->cpu.PC - addr) >> 8;
		if (((emu->cpu.PC - addr) >> 8) > 255) {
			printf ("exit with %d line\n", __LINE__);
			exit (0);
		}
	} else {
		cross_cycles = (addr - emu->cpu.PC) >> 8;
		if (((addr - emu->cpu.PC) >> 8) > 255) {
			printf ("exit with %d line\n", __LINE__);
			exit (0);
		}
	}
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "ADC ($%02x), Y", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + m[addr - off], 
		m[addr - off],
		eq,
		((5 + cross_cycles) << 8) | (2)
		);
}

void adc_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "ADC $%02x, X", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "ROR $%02x, X", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	ror_acts (emu,
			STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF,
			&emu->ram[addr],
			(6 << 8) | 2);
}

void sei_implied (struct NESEmu *emu) 
{
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "SEI", 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
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

	uint8_t cross_cycles = 0;
	if (emu->cpu.PC >= addr) {
		cross_cycles = (emu->cpu.PC - addr) >> 8;
		if (((emu->cpu.PC - addr) >> 8) > 255) {
			printf ("exit with %d line\n", __LINE__);
			exit (0);
		}
	} else {
		cross_cycles = (addr - emu->cpu.PC) >> 8;
		if (((addr - emu->cpu.PC) >> 8) > 255) {
			printf ("exit with %d line\n", __LINE__);
			exit (0);
		}
	}
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "ADC $%04x, Y", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + m[addr - off], 
		m[addr - off],
		eq,
		((4 + cross_cycles) << 8) | (3)
		);
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

	uint8_t cross_cycles = 0;
	if (emu->cpu.PC >= addr) {
		cross_cycles = (emu->cpu.PC - addr) >> 8;
		if (((emu->cpu.PC - addr) >> 8) > 255) {
			printf ("exit with %d line\n", __LINE__);
			exit (0);
		}
	} else {
		cross_cycles = (addr - emu->cpu.PC) >> 8;
		if (((addr - emu->cpu.PC) >> 8) > 255) {
			printf ("exit with %d line\n", __LINE__);
			exit (0);
		}
	}
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "ADC $%04x, X", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + m[addr - off], 
		m[addr - off],
		eq,
		((4 + cross_cycles) << 8) | (3)
		);
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "ROR $%04x, X", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "STA ($%02x, X)", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	st_acts (emu, &cpu->A, addr, (6 << 8) | 2);
}

void sty_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "STY $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	st_acts (emu, &cpu->Y, addr, (3 << 8) | 2);
}

void sta_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "STA $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	st_acts (emu, &cpu->A, addr, (3 << 8) | 2);
}
void stx_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "STX $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	st_acts (emu, &cpu->X, addr, (3 << 8) | 2);
}

void dey_implied (struct NESEmu *emu) 
{
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "DEY", 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "TXA", 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "STY $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	st_acts (emu, &cpu->Y, addr, (4 << 8) | 3);
}

void sta_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "STA $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	st_acts (emu, &cpu->A, addr, (4 << 8) | 3);
}



void stx_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "STX $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
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

	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "BCC $%04x", new_offset, new_offset, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "STA ($%02x), Y", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	st_acts (emu, &cpu->A, addr, (6 << 8) | 2);
}

void sty_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "STY $%02x, X", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	st_acts (emu, &cpu->Y, addr, (4 << 8) | 2);
}

void sta_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "STA $%02x, X", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	st_acts (emu, &cpu->A, addr, (4 << 8) | 2);
}
void stx_zeropage_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_y (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "STX $%02x, Y", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	st_acts (emu, &cpu->X, addr, (4 << 8) | 2);
}

void tya_implied (struct NESEmu *emu) 
{
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "TYA", 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "STA $%04x, Y", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	st_acts (emu, &cpu->A, addr, (5 << 8) | 3);
}

void txs_implied (struct NESEmu *emu) 
{
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "TXS", 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	struct CPUNes *cpu = &emu->cpu;

	cpu->S = cpu->X;

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void sta_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_x (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "STA $%04x, X", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	st_acts (emu, &cpu->A, addr, (5 << 8) | 3);
}

void ldy_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t val = immediate_val (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "LDY #$%02x", val, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	ld_acts_imm (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->Y,
			val,
			(2 << 8) | 2);
}

void lda_indirect_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = indirect_x (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "LDA ($%02x, X)", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->A,
			addr,
			(6 << 8) | 2);
}

void ldx_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t val = immediate_val (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "LDX #$%02x", val, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	ld_acts_imm (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->X,
			val,
			(2 << 8) | 2);
}

void ldy_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "LDY $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->Y,
			addr,
			(3 << 8) | 2);
}

void lda_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "LDA $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->A,
			addr,
			(3 << 8) | 2);
}

void ldx_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "LDX $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->X,
			addr,
			(3 << 8) | 2);
}

void tay_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "TAY", 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	uint8_t val = immediate_val (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "LDA #$%02x", val, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	ld_acts_imm (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->A,
			val,
			(2 << 8) | 2);
}

void tax_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "TAX", 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "LDY $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->Y,
			addr,
			(4 << 8) | 3);
}

void lda_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "LDA $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->A,
			addr,
			(4 << 8) | 3);
}

void ldx_absolute (struct NESEmu *emu)
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "LDX $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "BCS $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], new_offset, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "LDA ($%02x), Y", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->A,
			addr,
			(5 << 8) | 2);
}
void ldy_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "LDY $%02x, X", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->Y,
			addr,
			(4 << 8) | 2);
}

void lda_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "LDA $%02x, X", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->A,
			addr,
			(4 << 8) | 2);
}

void ldx_zeropage_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_y (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "LDX $%02x, Y", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->X,
			addr,
			(4 << 8) | 2);
}

void clv_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "CLV", 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	emu->cpu.P &= ~(STATUS_FLAG_VF);

	wait_cycles (emu, 2);

	emu->cpu.PC++;
}

void lda_absolute_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_y (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "LDA $%04x, Y", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->A,
			addr,
			(4 << 8) | 3);
}

void tsx_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "TSX", 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "LDY $%04x, X", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->Y,
			addr,
			(4 << 8) | 3);
}

void lda_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_x (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "LDA $%04x, X", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->A,
			addr,
			(4 << 8) | 3);
}

void ldx_absolute_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_y (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "LDX $%04x, Y", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	ld_acts (emu, STATUS_FLAG_ZF|STATUS_FLAG_NF,
			&cpu->X,
			addr,
			(4 << 8) | 3);
}

void cpy_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t val = immediate_val (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "CPY #$%02x", val, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "CMP ($%02x, X)", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "CPY $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "CMP $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "DEC $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "INY", 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "CMP #$%02x", val, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "DEX", 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "CPY $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "CMP $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "DEC $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "BNE $%04x", new_offset, new_offset, debugger_print_regs (emu));
		if (emu->only_show) return;
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "CMP ($%02x), Y", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "CMP $%02x, X", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "DEC $%02x, X", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "CLD", 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	emu->cpu.P &= ~(STATUS_FLAG_DF);

	wait_cycles (emu, 2);

	emu->cpu.PC++;
}

void cmp_absolute_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_y (emu);

	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "CMP $%04x, Y", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "CMP $%04x, X", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "DEC $%04x, X", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "CPX #$%02x", val, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "SBC ($%02x, X)", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + ~val, 
		~val,
		eq,
		(6 << 8) | (2)
		);
}

void cpx_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	uint8_t addr = zeropage (emu);

	uint8_t val = emu->ram[addr];
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "CPX $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "SBC $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + ~val, 
		~val,
		eq,
		(3 << 8) | (2)
		);
}

void inc_zeropage (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "INC $%02x", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "INX", 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "SBC #$%02x", val, 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + ~val, 
		~val,
		eq,
		(2 << 8) | (2)
		);
}

void nop_implied (struct NESEmu *emu) 
{
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "NOP", 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
	struct CPUNes *cpu = &emu->cpu;
	emu->cpu.PC++;

	wait_cycles (emu, 2);
}

void cpx_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "CPX $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "SBC $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + ~val, 
		~val,
		eq,
		(4 << 8) | (3)
		);
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "INC $%04x", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "BEQ $%04x", new_offset, new_offset, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "SBC ($%02x), Y", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + ~val, 
		~val,
		eq,
		(5 << 8) | (2)
		);
}

void sbc_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);
	uint8_t val = emu->ram[addr];
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "SBC $%02x, X", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + ~val, 
		~val,
		eq,
		(4 << 8) | (2)
		);
}

void inc_zeropage_x (struct NESEmu *emu) {
	struct CPUNes *cpu = &emu->cpu;
	uint8_t addr = zeropage_x (emu);
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "INC $%02x, X", *(uint8_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&emu->ram[addr],
			++emu->ram[addr],
			void_eq,
			(6 << 8) | 2);
}

void sed_implied (struct NESEmu *emu) 
{
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "SED", 0x0, debugger_print_regs (emu));
		if (emu->only_show) return;
	}
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "SBC $%04x, Y", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + ~val, 
		~val,
		eq,
		(4 << 8) | (3)
		);
}
void sbc_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint16_t addr = absolute_x (emu);
	uint8_t val = addr < RAM_MAX? emu->ram[addr]: emu->mem[addr - 0x8000];
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "SBC $%04x, X", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	adc_acts (emu, 
		STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, 
		&cpu->A, 
		cpu->A + ~val, 
		~val,
		eq,
		(4 << 8) | (3)
		);
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
	if (emu->debug_step) {
		char buf[256];
		snprintf (buf, 256, "BRK");
		printf ("%04x: %-20s [%04x] [%s]\n", emu->cpu.PC, "INC $%04x, X", *(uint16_t *) &emu->mem[(emu->cpu.PC + 1) - 0x8000], addr, debugger_print_regs (emu));
		if (emu->only_show) return;
	}

	repetitive_acts (emu, 
			STATUS_FLAG_NF|STATUS_FLAG_ZF,
			&m[addr - off],
			++m[addr - off],
			void_eq,
			(7 << 8) | 3);
}
