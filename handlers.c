#include <stdint.h>
#include <stddef.h>
#include <cpunes.h>

uint16_t accumulator (struct NESEmu *emu);
uint16_t immediate (struct NESEmu *emu);
uint16_t absolute (struct NESEmu *emu);
uint16_t zeropage (struct NESEmu *emu);
uint16_t zeropage_x (struct NESEmu *emu);
uint16_t zeropage_y (struct NESEmu *emu);
uint16_t absolute_x (struct NESEmu *emu);
uint16_t absolute_y (struct NESEmu *emu);
uint16_t indirect (struct NESEmu *emu);
uint16_t indirect_x (struct NESEmu *emu);
uint16_t indirect_y (struct NESEmu *emu);

static void write_to_address (struct NESEmu *emu, uint16_t addr, uint8_t *r)
{
	if (addr >= 0 && addr <= 0x500) {
		emu->ram[addr] = *r;
		return;
	}

	if (addr == PPUMASK) {
		emu->mem[addr] = *r;
		if ((*r) & MASK_IS_BACKGROUND_RENDER) {
			emu->cb->ppu_mask (emu, NULL);
		}

	} if (addr == PPUADDR) {
		if (emu->addr_off == 0) {
			emu->addr = 0;
			emu->addr |= (*r << 8) & 0xff00;
			emu->addr_off++;
		} else {
			emu->addr |= *r & 0xff;
			emu->addr_off = 0;
		}
	} else if (addr == PPUDATA) {
		emu->mem[emu->addr++] = *r;
	} else {
		emu->mem[addr] = *r;
	}
}

static void read_from_address (struct NESEmu *emu, uint16_t addr, uint8_t *r)
{
	if (addr == 0x2002) {
		emu->addr_off = 0;
	}
	if (addr >= 0 && addr <= 0x500) {
		*r = emu->ram[addr];
	} else {
		*r = emu->mem[addr];
	}
}

static void wait_cycles (struct NESEmu *emu, uint32_t cycles)
{
	emu->last_cycles_float = (float) cycles * 0.000601465f;
	emu->last_cycles_int64 = cycles * 601465L;
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

uint16_t accumulator (struct NESEmu *emu)
{
    return 0;
}

uint16_t immediate (struct NESEmu *emu)
{
    return emu->mem[emu->cpu.PC + 1];
}

uint16_t absolute (struct NESEmu *emu)
{
    uint16_t addr = *(uint16_t *) &emu->mem[emu->cpu.PC + 1];
    return addr;
}

uint16_t zeropage (struct NESEmu *emu)
{
    uint8_t addr = emu->mem[emu->cpu.PC + 1];
    return addr;
}

uint16_t zeropage_x (struct NESEmu *emu)
{
    uint8_t addr = emu->mem[emu->cpu.PC + 1] + emu->cpu.X;
    return addr;
}

uint16_t zeropage_y (struct NESEmu *emu)
{
    uint8_t addr = emu->mem[emu->cpu.PC + 1] + emu->cpu.Y;
    return addr;
}

uint16_t absolute_x (struct NESEmu *emu)
{
    uint16_t addr = *((uint16_t *) &emu->mem[emu->cpu.PC + 1]) + emu->cpu.X;
    return addr;
}

uint16_t absolute_y (struct NESEmu *emu)
{
    uint16_t addr = *((uint16_t *) &emu->mem[emu->cpu.PC + 1]) + emu->cpu.Y;
    return addr;
}

uint16_t indirect (struct NESEmu *emu)
{
	return 0;
}

uint16_t indirect_x (struct NESEmu *emu)
{
    uint8_t addr = emu->mem[emu->cpu.PC + 1];
    uint16_t indirect = *((uint16_t *) &emu->mem[emu->cpu.X]);
    uint16_t fulladdr = addr + indirect;

    return fulladdr;
}

uint16_t indirect_y (struct NESEmu *emu)
{
    uint8_t zeroaddr = emu->mem[emu->cpu.PC + 1];
    uint16_t addr = *((uint16_t *) &emu->mem[zeroaddr]);
    uint16_t fulladdr = addr + emu->cpu.Y;

    return fulladdr;
}

void adc (struct NESEmu *emu, uint16_t addr)
{
    emu->cpu.A += emu->mem[addr];
}

void _and (struct NESEmu *emu, uint16_t addr)
{
    emu->cpu.A &= emu->mem[addr];
}

void asl (struct NESEmu *emu, uint16_t addr)
{
    (void) addr;
    emu->cpu.A <<= 1;
}

void bcc (struct NESEmu *emu, uint16_t addr)
{
    (void) addr;
    if (emu->cpu.P & STATUS_FLAG_CF) {
    } else {
        emu->is_branch = 1;
        emu->cpu.PC += (int8_t) emu->mem[emu->cpu.PC + 1];
    }
}

void bcs (struct NESEmu *emu, uint16_t addr)
{
    (void) addr;
    if (emu->cpu.P & STATUS_FLAG_CF) {
        emu->is_branch = 1;
        emu->cpu.PC += (int8_t) emu->mem[emu->cpu.PC + 1];
    }
}

void beq (struct NESEmu *emu, uint16_t addr)
{
    (void) addr;
    if (emu->cpu.P & STATUS_FLAG_ZF) {
    } else {
        emu->is_branch = 1;
        emu->cpu.PC += (int8_t) emu->mem[emu->cpu.PC + 1];
    }
}

void invalid_opcode (struct NESEmu *) {}
void brk_implied (struct NESEmu *) {}
void ora_indirect_x (struct NESEmu *) {}
void ora_zeropage (struct NESEmu *) {}
void asl_zeropage (struct NESEmu *) {}
void php_implied (struct NESEmu *) {}
void ora_immediate (struct NESEmu *) {}
void asl_accumulator (struct NESEmu *) {}
void ora_absolute (struct NESEmu *) {}
void asl_absolute (struct NESEmu *) {}

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

void bpl_relative (struct NESEmu *emu) 
{
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		int8_t offset = emu->mem[cpu->PC + 1];
		uint16_t new_offset;

		if (offset < 0) {
			uint8_t off = 0xff - offset - 1;
			new_offset = cpu->PC - off;
		} else {
			new_offset = cpu->PC + offset;
		}

		uint8_t *pl = show_debug_info (emu, 2, "BPL");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xff);
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		cpu->PC += 2;
		return;
	}

	struct CPUNes *cpu = &emu->cpu;

	int8_t offset = emu->mem[cpu->PC + 1];

	uint16_t new_offset;

	if (offset < 0) {
		uint8_t off = 0xff - offset - 1;
		new_offset = cpu->PC - off;
	} else {
		new_offset = cpu->PC + offset;
	}

	uint32_t ext_cycles;

	if (emu->cpu.P & STATUS_FLAG_NF) {
		set_ext_cycles (cpu, offset, new_offset, &ext_cycles);

		cpu->PC = new_offset;
	} else {
		cpu->PC += 2;
	}

	wait_cycles (emu, 2 + ext_cycles);
}

void ora_indirect_y (struct NESEmu *) {}
void ora_zeropage_x (struct NESEmu *) {}
void asl_zeropage_x (struct NESEmu *) {}
void clc_implied (struct NESEmu *) {}
void ora_absolute_y (struct NESEmu *) {}
void ora_absolute_x (struct NESEmu *) {}
void asl_absolute_x (struct NESEmu *) {}
void jsr_absolute (struct NESEmu *) {}
void and_indirect_x (struct NESEmu *) {}
void bit_zeropage (struct NESEmu *) {}
void and_zeropage (struct NESEmu *) {}
void rol_zeropage (struct NESEmu *) {}
void plp_implied (struct NESEmu *) {}
void and_immediate (struct NESEmu *) {}
void rol_accumulator (struct NESEmu *) {}

void bit_absolute (struct NESEmu *emu) 
{
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		uint16_t new_offset = *(uint16_t *) &emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 3, "BIT");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xff);
		*pl++ = '$';
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		cpu->PC += 3;
		return;
	}

	struct CPUNes *cpu = &emu->cpu;

	cpu->PC++;

	uint8_t returned_reg = 0;

	uint16_t addr = *(uint16_t *) &emu->mem[cpu->PC];

	returned_reg = emu->mem[addr];

	cpu->PC += 2;

	cpu->P &= ~(STATUS_FLAG_NF|STATUS_FLAG_VF|STATUS_FLAG_ZF);

	cpu->P |= (returned_reg & 0xc0);

	if ((cpu->A & returned_reg) == 0) {
		cpu->P |= (STATUS_FLAG_ZF);
	}

	wait_cycles (emu, 4);
}

void and_absolute (struct NESEmu *) {}
void rol_absolute (struct NESEmu *) {}
void bmi_relative (struct NESEmu *) {}
void and_indirect_y (struct NESEmu *) {}
void and_zeropage_x (struct NESEmu *) {}
void rol_zeropage_x (struct NESEmu *) {}
void sec_implied (struct NESEmu *) {}
void and_absolute_y (struct NESEmu *) {}
void and_absolute_x (struct NESEmu *) {}
void rol_absolute_x (struct NESEmu *) {}

void rti_implied (struct NESEmu *emu) 
{
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		int8_t offset = emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 1, "RTI");
		*pl = 0;
		cpu->PC++;
		return;
	}

	emu->cpu.PC = emu->latest_exec;
}
void eor_indirect_x (struct NESEmu *) {}
void eor_zeropage (struct NESEmu *) {}
void lsr_zeropage (struct NESEmu *) {}
void pha_implied (struct NESEmu *) {}
void eor_immediate (struct NESEmu *) {}
void lsr_accumulator (struct NESEmu *) {}

void jmp_absolute (struct NESEmu *emu)
{
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		uint16_t new_offset = *(uint16_t *) &emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 3, "JMP");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xff);
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		cpu->PC += 3;
		return;
	}

	struct CPUNes *cpu = &emu->cpu;

	cpu->PC++;

	uint16_t new_offset = *(uint16_t *) &emu->mem[cpu->PC];

	cpu->PC = new_offset;

	wait_cycles (emu, 3);
}

void eor_absolute (struct NESEmu *) {}
void lsr_absolute (struct NESEmu *) {}
void bvc_relative (struct NESEmu *) {}
void eor_indirect_y (struct NESEmu *) {}
void eor_zeropage_x (struct NESEmu *) {}
void lsr_zeropage_x (struct NESEmu *) {}
void cli_implied (struct NESEmu *) {}
void eor_absolute_y (struct NESEmu *) {}
void eor_absolute_x (struct NESEmu *) {}
void lsr_absolute_x (struct NESEmu *) {}
void rts_implied (struct NESEmu *) {}
void adc_indirect_x (struct NESEmu *) {}
void adc_zeropage (struct NESEmu *) {}
void ror_zeropage (struct NESEmu *) {}
void pla_implied (struct NESEmu *) {}
void adc_immediate (struct NESEmu *) {}
void ror_accumulator (struct NESEmu *) {}
void jmp_indirect (struct NESEmu *) {}
void adc_absolute (struct NESEmu *) {}
void ror_absolute (struct NESEmu *) {}
void bvs_relative (struct NESEmu *) {}
void adc_indirect_y (struct NESEmu *) {}
void adc_zeropage_x (struct NESEmu *) {}
void ror_zeropage_x (struct NESEmu *) {}

void sei_implied (struct NESEmu *emu) 
{
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;

		uint8_t *pl = show_debug_info (emu, 1, "SEI");
		*pl = 0;
		cpu->PC++;
		return;
	}

	emu->cpu.P &= (STATUS_FLAG_IF);
	emu->cpu.PC++;

	wait_cycles (emu, 2);
}

void adc_absolute_y (struct NESEmu *) {}
void adc_absolute_x (struct NESEmu *) {}
void ror_absolute_x (struct NESEmu *) {}
void sta_indirect_x (struct NESEmu *) {}
void sty_zeropage (struct NESEmu *) {}
void sta_zeropage (struct NESEmu *) {}
void stx_zeropage (struct NESEmu *) {}
void dey_implied (struct NESEmu *) {}

void txa_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->A = cpu->X;

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void sty_absolute (struct NESEmu *emu) 
{
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		uint16_t new_offset = *(uint16_t *) &emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 3, "STY");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xff);
		*pl++ = '$';
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		cpu->PC += 2;
		return;
	}

	struct CPUNes *cpu = &emu->cpu;

	cpu->PC++;

	uint8_t *r = &cpu->Y;

	uint16_t addr = *(uint16_t *) &emu->mem[cpu->PC];

	write_to_address (emu, addr, r);

	cpu->PC += 2;

	wait_cycles (emu, 4);
}

void sta_absolute (struct NESEmu *emu) 
{
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		uint16_t new_offset = *(uint16_t *) &emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 3, "STA");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xff);
		*pl++ = '$';
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		cpu->PC += 3;
		return;
	}

	struct CPUNes *cpu = &emu->cpu;

	cpu->PC++;

	uint8_t *r = &cpu->A;

	uint16_t addr = *(uint16_t *) &emu->mem[cpu->PC];

	write_to_address (emu, addr, r);

	wait_cycles (emu, 4);

	cpu->PC += 2;
}



void stx_absolute (struct NESEmu *emu) 
{
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		uint16_t new_offset = *(uint16_t *) &emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 3, "STX");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xff);
		*pl++ = '$';
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		cpu->PC += 3;
		return;
	}

	struct CPUNes *cpu = &emu->cpu;

	cpu->PC++;

	uint8_t *r = &cpu->X;

	uint16_t addr = *(uint16_t *) &emu->mem[cpu->PC];

	write_to_address (emu, addr, r);

	cpu->PC += 2;

	wait_cycles (emu, 4);
}

void bcc_relative (struct NESEmu *) {}
void sta_indirect_y (struct NESEmu *) {}
void sty_zeropage_x (struct NESEmu *) {}
void sta_zeropage_x (struct NESEmu *) {}
void stx_zeropage_y (struct NESEmu *) {}

void tya_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->A = cpu->Y;

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void sta_absolute_y (struct NESEmu *) {}

void txs_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	emu->mem[cpu->S] = cpu->X;

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void sta_absolute_x (struct NESEmu *) {}
void ldy_immediate (struct NESEmu *) {}
void lda_indirect_x (struct NESEmu *) {}

void ldx_immediate (struct NESEmu *emu) 
{
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		int8_t offset = emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 2, "LDX");
		uint8_t number = (offset & 0xff);
		*pl++ = '#';
		*pl++ = '$';
		*pl++ = hex_up_half (number);
		*pl++ = hex_down_half (number);
		*pl = 0;
		cpu->PC += 2;
		return;
	}

	struct CPUNes *cpu = &emu->cpu;

	cpu->PC++;
	uint8_t imm_byte = emu->mem[emu->cpu.PC];
	if (imm_byte == 0) {
		cpu->P |= (STATUS_FLAG_ZF);
	}
	if (imm_byte >= 0x80) {
		cpu->P |= (STATUS_FLAG_NF);
	}
	cpu->X = imm_byte;

	wait_cycles (emu, 2);

	cpu->PC++;
}

void ldy_zeropage (struct NESEmu *) {}
void lda_zeropage (struct NESEmu *) {}
void ldx_zeropage (struct NESEmu *) {}

void tay_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->Y = cpu->A;

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void lda_immediate (struct NESEmu *emu) 
{
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		int8_t offset = emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 2, "LDA");
		uint8_t number = (offset & 0xff);
		*pl++ = '#';
		*pl++ = '$';
		*pl++ = hex_up_half (number);
		*pl++ = hex_down_half (number);
		*pl = 0;
		cpu->PC += 2;
		return;
	}

	struct CPUNes *cpu = &emu->cpu;

	cpu->PC++;
	uint8_t imm_byte = emu->mem[emu->cpu.PC];
	if (imm_byte == 0) {
		cpu->P |= (STATUS_FLAG_ZF);
	}
	if (imm_byte >= 0x80) {
		cpu->P |= (STATUS_FLAG_NF);
	}
	cpu->A = imm_byte;

	wait_cycles (emu, 2);

	cpu->PC++;
}

void tax_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->X = cpu->A;

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void ldy_absolute (struct NESEmu *emu) 
{
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		uint16_t new_offset = *(uint16_t *) &emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 3, "LDY");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xff);
		*pl++ = '$';
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		cpu->PC += 3;
		return;
	}

	struct CPUNes *cpu = &emu->cpu;

	uint16_t addr = *(uint16_t *) &emu->mem[cpu->PC + 1];

	read_from_address (emu, addr, &cpu->Y);

	emu->cpu.PC += 3;

	wait_cycles (emu, 4);
}

void lda_absolute (struct NESEmu *emu) 
{
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		uint16_t new_offset = *(uint16_t *) &emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 3, "LDA");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xff);
		*pl++ = '$';
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		cpu->PC += 3;
		return;
	}

	struct CPUNes *cpu = &emu->cpu;

	uint16_t addr = *(uint16_t *) &emu->mem[cpu->PC + 1];

	read_from_address (emu, addr, &cpu->A);

	emu->cpu.PC += 3;

	wait_cycles (emu, 4);
}

void ldx_absolute (struct NESEmu *emu)
{
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		uint16_t new_offset = *(uint16_t *) &emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 3, "LDX");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xff);
		*pl++ = '$';
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		cpu->PC += 3;
		return;
	}

	struct CPUNes *cpu = &emu->cpu;

	uint16_t addr = *(uint16_t *) &emu->mem[cpu->PC + 1];

	read_from_address (emu, addr, &cpu->X);

	emu->cpu.PC += 3;

	wait_cycles (emu, 4);
}

void bcs_relative (struct NESEmu *) {}
void lda_indirect_y (struct NESEmu *) {}
void ldy_zeropage_x (struct NESEmu *) {}
void lda_zeropage_x (struct NESEmu *) {}
void ldx_zeropage_y (struct NESEmu *) {}
void clv_implied (struct NESEmu *) {}
void lda_absolute_y (struct NESEmu *) {}

void tsx_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->X = emu->mem[cpu->S];

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void ldy_absolute_x (struct NESEmu *) {}
void lda_absolute_x (struct NESEmu *) {}
void ldx_absolute_y (struct NESEmu *) {}
void cpy_immediate (struct NESEmu *) {}
void cmp_indirect_x (struct NESEmu *) {}
void cpy_zeropage (struct NESEmu *) {}
void cmp_zeropage (struct NESEmu *) {}
void dec_zeropage (struct NESEmu *) {}

void iny_implied (struct NESEmu *emu) 
{
	emu->cpu.Y++;

	emu->cpu.PC++;

	wait_cycles (emu, 2);
}

void cmp_immediate (struct NESEmu *) {}
void dex_implied (struct NESEmu *) {}
void cpy_absolute (struct NESEmu *) {}
void cmp_absolute (struct NESEmu *) {}
void dec_absolute (struct NESEmu *) {}
void bne_relative (struct NESEmu *) {}
void cmp_indirect_y (struct NESEmu *) {}
void cmp_zeropage_x (struct NESEmu *) {}
void dec_zeropage_x (struct NESEmu *) {}

void cld_implied (struct NESEmu *emu) 
{
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		uint8_t *pl = show_debug_info (emu, 1, "CLD");
		*pl = 0;
		cpu->PC++;
		return;
	}

	emu->cpu.P &= ~(STATUS_FLAG_DF);

	wait_cycles (emu, 2);

	emu->cpu.PC++;
}

void cmp_absolute_y (struct NESEmu *) {}
void cmp_absolute_x (struct NESEmu *) {}
void dec_absolute_x (struct NESEmu *) {}
void cpx_immediate (struct NESEmu *) {}
void sbc_indirect_x (struct NESEmu *) {}
void cpx_zeropage (struct NESEmu *) {}
void sbc_zeropage (struct NESEmu *) {}
void inc_zeropage (struct NESEmu *) {}

void inx_implied (struct NESEmu *emu)
{
	emu->cpu.X++;

	emu->cpu.PC++;

	wait_cycles (emu, 2);
}

void sbc_immediate (struct NESEmu *) {}
void nop_implied (struct NESEmu *) {}
void cpx_absolute (struct NESEmu *) {}
void sbc_absolute (struct NESEmu *) {}
void inc_absolute (struct NESEmu *) {}
void beq_relative (struct NESEmu *) {}
void sbc_indirect_y (struct NESEmu *) {}
void sbc_zeropage_x (struct NESEmu *) {}
void inc_zeropage_x (struct NESEmu *) {}
void sed_implied (struct NESEmu *) {}
void sbc_absolute_y (struct NESEmu *) {}
void sbc_absolute_x (struct NESEmu *) {}
void inc_absolute_x (struct NESEmu *) {}

#if 0
void calc_addr (struct NESEmu *emu,
                uint16_t (*get_addr) (struct NESEmu *emu),
                void (*flags) (struct NESEmu *emu, uint16_t addr),
                void (*opcode_exec) (struct NESEmu *emu, uint16_t addr),
                uint16_t pc_offset,
                uint8_t cycles,
                uint8_t cross_page
                )
{
    emu->is_branch = 0;
    uint16_t addr = 0;
    if (get_addr) addr = get_addr (emu);
    if (flags) flags (emu, addr);
    if (opcode_exec) opcode_exec (emu, addr);
    wait_cycles(emu, addr, cycles);
    if (emu->is_branch == 0)
        emu->cpu.PC += pc_offset;
}
#endif
